/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <serialize/binary-i-serializer.hh>

#include <libport/finally.hh>

#include <ast/nary.hh>
#include <ast/stmt.hh>

#include <libport/debug.hh>

#include <urbi/object/lobby.hh>
#include <urbi/object/object.hh>
#include <urbi/object/tag.hh>

#include <urbi/uvalue-serialize.hh>

#include <object/system.hh>

#include <ast/print.hh>
#include <urbi/kernel/uconnection.hh>
#include <kernel/uobject.hh>
#include <urbi/kernel/userver.hh>
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
	       const std::string& name,
               std::istream& input)
    : Interpreter(lobby, scheduler, ast::rConstAst(), name)
    , executing_(false)
    , input_(input)
    , stop_(false)
    , binary_mode_(false)
    , parser_(new parser::UParser(input_))
  {
    GD_FINFO_TRACE("new shell: %s %p", name_get(), this);
  }

  Shell::~Shell()
  {
    GD_FINFO_TRACE("dying shell: %s %p", name_get(), this);
    if (serializationJob_)
      serializationJob_->terminate_asap();
    delete parser_;
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
      Exception::warn(exp->location_get(),
                      libport::format("invalid exception caught: %s",
                                      e.what()));
    }
    catch (...)
    {
      Exception::warn(exp->location_get(), "unknown exception caught");
    }

    if (exception_to_show.get())
      show_exception(*exception_to_show);
  }

  void
  Shell::handle_command_(ast::rConstExp exp, bool canYield)
  {
    LIBPORT_SCOPE_SET(executing_, true);
    const ast::Stmt* stmt = dynamic_cast<const ast::Stmt*>(exp.get());

    if (stmt && stmt->flavor_get() == ast::flavor_comma)
    {
      GD_FPUSH_TRACE("%s: executing command in background.", name_get());
      GD_FINFO_DUMP("%s: command: %s", name_get(), *exp);
      sched::rJob subrunner =
        new Interpreter(*this, stmt->expression_get().get(),
                        libport::Symbol::fresh_string(name_get()));
      jobs_ <<  subrunner;
      subrunner->start_job();
      if (canYield && !input_.eof())
        yield();
    }
    else
    {
      GD_FPUSH_TRACE("%s: executing command (%s).", name_get(), stmt);
      GD_FINFO_DUMP("%s: command: %s", name_get(), *exp);
      eval_print_(exp.get());
      if (canYield && !input_.eof())
        yield();
    }

    if (non_interruptible_get())
    {
      send_message("error", "the toplevel cannot be non-interruptible");
      non_interruptible_set(false);
    }
  }

  static
  void collect_connection(kernel::UConnection* c)
  {
    GD_FPUSH_DUMP("closing connection %p", c);
    aver(c);
    kernel::server().connection_remove(*c);
  }

  void
  Shell::handle_command_end_()
  {
    if (input_.eof())
    {
      stop();
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

    foreach (const sched::rJob& job, jobs_)
      job->terminate_now();
    yield_until_terminated(jobs_);
  }

  void
  Shell::work_()
  {
    while (!stop_)
    {
      try
      {
        parser::parse_result_type res;
        {
          GD_FPUSH_TRACE("%s: reading command.", name_get());
          // It once happened that parser_ was null here.  The
          // scenario is that an remote UObject in binary mode sends a
          // message that is too big to fit into the stream buffer,
          // and as a result the message was incomplete.  This caused
          // processSerializedMessages to end in an unexpected way,
          // and control returned here, with !stop_.  So we returned
          // in the regular processing flow, although there is no
          // parser around.
          //
          // To work around this, the buffer size was increased (from
          // 1k to 8k on OS X).
          assert(parser_);
          res = parser_->parse();
        }
        ast::rExp ast = parser::transform(ast::rConstExp(res));
        handle_command_(ast);
        handle_command_end_();
      }
      catch (const Exception& e)
      {
        GD_FINFO_TRACE("%s: command is invalid, printing errors.",
                       name_get());
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

  void
  Shell::processSerializedMessages()
  {
    GD_CATEGORY(Urbi.Shell.Serialize);
    GD_INFO_TRACE("Entering processSerializedMessages");
    libport::serialize::BinaryISerializer deserializer(input_);
    while (binary_mode_ && !stop_)
    {
      try
      {
        GD_INFO_DEBUG("Reading new serialized message type");
        char msgType;
        deserializer >> msgType;
        GD_FINFO_DEBUG("Processing serialized message of type %s",
                       (int)msgType);
        std::string code
          = urbi::uobjects::processSerializedMessage(msgType, deserializer);
        GD_FINFO_DEBUG("Got value '%s'", code);
        if (!code.empty())
        {
          // We must parse and execute synchronously, but honor ',': this is
          // toplevel code.
          yy::location loc;
          parser::UParser p(code, &loc);
          p.oneshot_set(false);
          ast::rConstExp e = p.parse();
          ast::rConstExp ast = parser::transform(e);
          handle_command_(ast::rConstExp(ast), false);
        }
        handle_command_end_();
      }
      // Catch and print unhandled exceptions
      catch (object::UrbiException& e)
      {
        show_exception(object::UrbiException(e.value_get(),
                                             e.backtrace_get()));
      }
      catch (const Exception& e)
      {
        GD_FINFO_TRACE("%s: command is invalid, printing errors.",
                       name_get());
        e.print(*this);
      }
      catch (const sched::StopException& e)
      {
        GD_FINFO_DUMP("StopException ignored: %s", e.what());
      }
      catch (const sched::TerminateException&)
      {
        GD_INFO_DUMP("TerminateException");
        throw;
      }
      catch (const std::runtime_error& re)
      {
        GD_FINFO_TRACE("Serialized message error: %s", re.what());
        Exception e;
        e.err(ast::loc(), re.what());
        e.print(*this);
      }
      catch (const std::exception& e)
      {
        GD_FWARN("Unexpected error '%s' caught in processSerializedMessages",
                 e.what());
        throw;
      }
      catch(...)
      {
        GD_WARN("Unknown error caught in processSerializedMessages");
        throw;
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

  void
  Shell::setSerializationMode(bool m, const std::string& tag)
  {
    GD_CATEGORY(Urbi.Shell.Serialize);
    GD_FINFO_TRACE("setSerializationMode %s", m);
    lobby_get()->send(m ? "true" : "false", tag);
    if (m == binary_mode_)
      return;
    if (!m && binary_mode_)
      throw std::runtime_error("There is no turning back from serialized mode");
    delete parser_;
    parser_ = 0;
    binary_mode_ = m;
    if (binary_mode_)
      processSerializedMessages();
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

  void
  Shell::stop()
  {
    if (stop_)
      return;
    GD_FPUSH_TRACE("%s: end reached.", name_get());
    if (&kernel::urbiserver->ghost_connection_get() !=
        &lobby_get()->connection_get())
    {
      // Asynchronous destruction of the connection is necessary
      // otherwise the connection destructor (which is stopping the
      // connection tag) will raise a Stop exception.  In addition, the
      // UConnection is supposed to hold the last reference on this
      // Shell once the work function has ended, thus scheduling the
      // ansynchronous destruction will destroy the lobby and the shell.
      // The UConnection should be fetch before disconnecting the lobby,
      // because the lobby will lose it's reference on the connection.
      GD_FPUSH_DUMP("%s: schedule connection destruction.", name_get());
      aver(&lobby_get()->connection_get());
      kernel::server()
        .schedule(SYMBOL(collect_connection),
                  boost::bind(collect_connection,
                              &lobby_get()->connection_get()));
      GD_FPUSH_DUMP("%s: disconnecting.", name_get());
      lobby_get()->disconnect();
      stop_ = true;
    }
    // Force interruption of work_
    lobby_get()->tag_get()->stop();
  }
} // namespace runner
