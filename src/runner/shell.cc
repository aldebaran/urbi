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

#include <libport/debug.hh>

#include <urbi/object/lobby.hh>
#include <urbi/object/object.hh>
#include <urbi/object/tag.hh>

#include <ast/print.hh>
#include <kernel/uconnection.hh>
#include <kernel/userver.hh>
#include <parser/parse-result.hh>
#include <parser/transform.hh>
#include <parser/uparser.hh>
#include <runner/exception.hh>
#include <runner/shell.hh>

#include <sched/job.hh>
#include <sched/scheduler.hh>

namespace runner
{
  GD_CATEGORY(Urbi.Shell);

  Shell::Shell(const rLobby& lobby,
	       sched::Scheduler& scheduler,
	       libport::Symbol name,
               std::istream& input)
    : Interpreter(lobby, scheduler, ast::rConstAst(), name)
    , executing_(false)
    , input_(input)
    , stop_(false)
  {
    GD_FINFO_TRACE("new shell: %s.", name_get());
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
      GD_FINFO_DUMP("StopException ignored: %s", e.what());
    }
    catch (const sched::TerminateException&)
    {
      GD_INFO_DUMP("TerminateException");
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
  Shell::handle_command_(ast::rConstExp exp)
  {
    LIBPORT_SCOPE_SET(executing_, true);
    const ast::Stmt* stmt = dynamic_cast<const ast::Stmt*>(exp.get());

    if (stmt && stmt->flavor_get() == ast::flavor_comma)
    {
      GD_FPUSH_TRACE("%s: executing command in background.", name_get());
      GD_FINFO_DUMP("%s: command: %s", name_get(), *exp);
      sched::rJob subrunner =
        new Interpreter(*this, stmt->expression_get().get(),
                        libport::Symbol::fresh(name_get()));
      jobs_ <<  subrunner;
      subrunner->start_job();
      if (!input_.eof())
        yield();
    }
    else
    {
      GD_FPUSH_TRACE("%s: executing command.", name_get());
      GD_FINFO_DUMP("%s: command: %s", name_get(), *exp);
      eval_print_(exp.get());
      if (!input_.eof())
        yield();
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
    while (!stop_)
    {
      try
      {
        work_();
      }
      // Don't kill the shell when we receive the exception
      // fired by "connectionTag.stop".
      catch (const sched::StopException& e)
      {
        GD_FINFO_DUMP("StopException ignored: %s", e.what());
      }
    }

    kernel::server().connection_remove(lobby_get()->connection_get());
    foreach (const sched::rJob& job, jobs_)
      job->terminate_now();
    yield_until_terminated(jobs_);
  }

  void
  Shell::work_()
  {
    parser::UParser parser(input_);

    while (!stop_)
    {
      try
      {
        parser::parse_result_type res;
        {
          GD_FPUSH_TRACE("%s: reading command.", name_get());
          res = parser.parse();
        }
        ast::rExp ast = parser::transform(ast::rConstExp(res->ast_xget()));
        handle_command_(ast);
        if (input_.eof())
        {
          GD_FPUSH_TRACE("%s: end reached, disconnecting.", name_get());
          lobby_get()->disconnect();
          stop_ = true;
        }
      }
      catch (const Exception& e)
      {
        GD_FINFO_TRACE("%s: command is invalid, printing errors.", name_get());
        e.print(*this);
      }
      for (jobs_type::iterator it = jobs_.begin();
           it != jobs_.end();
           /* nothing */)
        if ((*it)->terminated())
          it = jobs_.erase(it);
        else
          ++it;
    }
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
