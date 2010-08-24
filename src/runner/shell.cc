/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/finally.hh>

#include <ast/nary.hh>
#include <ast/stmt.hh>

#include <urbi/object/lobby.hh>
#include <urbi/object/object.hh>
#include <urbi/object/tag.hh>

#include <kernel/uconnection.hh>
#include <kernel/userver.hh>
#include <runner/shell.hh>

#include <sched/job.hh>
#include <sched/scheduler.hh>

namespace runner
{
  GD_CATEGORY(Urbi);

  Shell::Shell(const rLobby& lobby,
	       sched::Scheduler& scheduler,
	       libport::Symbol name)
    : Interpreter(lobby, scheduler, ast::rConstAst(), name)
    , executing_(false)
  {
  }

  inline
  void
  Shell::handle_oob_()
  {
    while (!oob_calls_.empty())
    {
      oob_calls_.front()();
      oob_calls_.pop_front();
    }
  }


  void
  Shell::eval_print_(const ast::Exp* exp)
  {
    // Visual Studio on Windows does not rewind the stack before the
    // end of a "try/catch" block, "catch" included. It means that we
    // cannot display the exception in the "catch" block in case this
    // is a scheduling error due to stack exhaustion, as we may need
    // the stack. The following objects will be set if we have an
    // exception to show, and it will be printed after the "catch"
    // block, or if we have an exception to rethrow.
    boost::scoped_ptr<object::UrbiException> exception_to_show;

    try
    {
      object::rObject res = operator()(exp);

      // We need to keep checking for void here because it
      // cannot be passed to the << function.
      if (res && res != object::void_class)
      {
        // Display the value using the topLevel channel.  If it is not
        // (yet) defined, do nothing, unless this environment variable
        // is set.
        static bool toplevel_debug = getenv("URBI_TOPLEVEL");

        rSlot topLevel =
          lobby_get()->slot_locate(SYMBOL(topLevel), false).second;
       if (topLevel)
         topLevel->value()->call(SYMBOL(LT_LT), res);
        else if (toplevel_debug)
        {
          try
          {
            rObject result = res->call(SYMBOL(asToplevelPrintable));
            std::ostringstream os;
            result->print(os);
            lobby_->connection_get().send(os.str());
            lobby_->connection_get().endline();
          }
          catch (object::UrbiException&)
          {
            // nothing
          }
        }
      }
    }
    // Catch and print unhandled exceptions
    catch (object::UrbiException& e)
    {
      exception_to_show.reset(new object::UrbiException(e.value_get(),
                                                        e.backtrace_get()));
    }
    // When we receive the exception fired by "connectionTag.stop",
    // kill the jobs, not the shell.
    catch (const sched::StopException& e)
    {
      GD_FINFO_DUMP("shell: StopException ignored: %s", e.what());
    }
    catch (const sched::TerminateException&)
    {
      GD_INFO_DUMP("shell: TerminateException");
      throw;
    }
    // Stop invalid exceptions thrown by primitives
    catch (const std::exception& e)
    {
      send_message("error",
                   libport::format("Invalid exception `%s' caught", e.what()));
    }
    catch (...)
    {
      send_message("error", "Invalid unknown exception caught");
    }

    if (exception_to_show.get())
      show_exception(*exception_to_show);
  }

  void
  Shell::handle_command_()
  {
    ast::rConstExp exp = commands_.front();
    commands_.pop_front();

    const ast::Stmt* stmt = dynamic_cast<const ast::Stmt*>(exp.get());

    if (stmt && stmt->flavor_get() == ast::flavor_comma)
    {
      sched::rJob subrunner =
        new Interpreter(*this, operator()(stmt->expression_get().get()),
                        libport::Symbol::fresh(name_get()));
      subrunner->start_job();
    }
    else
    {
      eval_print_(exp.get());
    }

    if (non_interruptible_get())
    {
      send_message("error", "the toplevel cannot be non-interruptible");
      non_interruptible_set(false);
    }
  }

  void
  Shell::work()
  {
    while (true)
    {
      try
      {
        work_();
      }
      // Don't kill the shell when we receive the exception
      // fired by "connectionTag.stop".
      catch (const sched::StopException& e)
      {
        GD_FINFO_DUMP("shell: StopException ignored: %s", e.what());
      }
    }
  }

  void
  Shell::work_()
  {
    handle_oob_();
    // Wait until we have some work to do
    if (commands_.empty())
    {
      executing_ = false;

      // The first yield will remember our possible side effect. Subsequent
      // ones will not and will pretend that we didn't have any side effect,
      // as we couldn't have influenced the system any further.
      yield_until_things_changed();

      libport::Finally finally(boost::bind(&Job::side_effect_free_set,
                                           this,
                                           side_effect_free_get()));
      side_effect_free_set(true);
      while (commands_.empty())
      {
        libport::Finally finally(boost::bind(&Job::side_effect_free_set,
                                             this,
                                             side_effect_free_get()));
        handle_oob_();
        yield_until_things_changed();
      }
      executing_ = true;
    }

    handle_command_();
    yield();
  }

  void
  Shell::append_command(const ast::rConstNary& command)
  {
    foreach (const ast::rExp& e, command->children_get())
      commands_ << e;
    scheduler_get().signal_world_change();
  }

  void
  Shell::insert_oob_call(boost::function0<void> func)
  {
    oob_calls_ << func;
    scheduler_get().signal_world_change();
  }

  bool
  Shell::pending_command_get() const
  {
    return executing_ || !commands_.empty();
  }

  void
  Shell::pending_commands_clear()
  {
    commands_.clear();
  }
} // namespace runner
