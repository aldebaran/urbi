/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/system.cc
 ** \brief Creation of the Urbi object system.
 */

#include <libport/config.h>
#include <libport/asio.hh>
#include <libport/cstdlib>
#include <libport/format.hh>
#include <libport/program-name.hh>
#include <libport/xltdl.hh>

#include <libport/cerrno>
#include <memory>
#include <sstream>

#include <kernel/uobject.hh>
#include <kernel/userver.hh>

#include <urbi/object/code.hh>
#include <urbi/object/cxx-primitive.hh>
#include <urbi/object/dictionary.hh>
#include <object/urbi-exception.hh>
#include <urbi/object/float.hh>
#include <urbi/object/global.hh>
#include <urbi/object/list.hh>
#include <urbi/object/object.hh>
#include <urbi/object/path.hh>
#include <object/symbols.hh>
#include <object/system.hh>
#include <urbi/object/tag.hh>
#include <urbi/object/job.hh>
#include <parser/transform.hh>
#include <runner/exception.hh>
#include <runner/interpreter.hh>
#include <urbi/runner/raise.hh>
#include <runner/runner.hh>

GD_CATEGORY(Urbi);

namespace urbi
{
  static std::vector<Initialization>&
  initializations_()
  {
    static std::vector<Initialization> initializations;
    return initializations;
  }

  int initialization_register(const Initialization& action)
  {
    initializations_() << action;
    return 42;
  }

  namespace object
  {

    using ::kernel::interpreter;
    using ::kernel::runner;

    /// \param context  if there is an error message, the string to display.
    rObject
    execute_parsed(parser::parse_result_type p,
                   libport::Symbol, const std::string& context,
                   rObject self)
    {
      runner::Interpreter& run = interpreter();

      // Report errors.
      if (ast::rError errs = p->errors_get())
      {
        // Steal the errors from the ParseResult.  It is tailored to
        // abort if there are some errors that were not reported to
        // the user, which is what we are doing now.
        ast::Error::messages_type ms;
        std::swap(ms, errs->messages_get());
        // FIXME: Yes, we actually raise only the first error.
        foreach (ast::rMessage m, ms)
          if (m->tag_get() == "error")
            runner::raise_syntax_error(m->location_get(), m->text_get(),
                                       context);
          else
          {
            // Raising and catching is not elegant, and probably
            // costly, as compared to making the exceptions and
            // displaying with regular functions.  But it's short to
            // implement, and it is definitely not a hot spot.
            try
            {
              runner::raise_syntax_error(m->location_get(), m->text_get(),
                                         context, false);
            }
            catch (const object::UrbiException& ue)
            {
              run.show_exception(ue, "warning");
            }
          }
      }

      ast::rConstAst ast;
      try
      {
        ast = parser::transform(ast::rConstExp(p->ast_get()));
      }
      catch (const runner::Exception& e)
      {
        e.raise(context);
      }

      return run.eval(ast.get(), self ? self : rObject(run.lobby_get()));
    }

    rObject system_class;

    /*--------------------.
    | System primitives.  |
    `--------------------*/


#define SERVER_FUNCTION(Function)               \
    static void                                 \
    system_ ## Function()                       \
    {                                           \
      ::kernel::urbiserver->Function();         \
    }

    SERVER_FUNCTION(reboot)
    SERVER_FUNCTION(shutdown)

#undef SERVER_FUNCTION


    static void
    system_sleep(const rObject&, libport::ufloat seconds)
    {
      runner::Runner& r = runner();
      if (seconds == std::numeric_limits<ufloat>::infinity())
        r.yield_until_terminated(r);
      else
        r.yield_for(libport::utime_t(seconds * 1000000.0));
    }

    static float
    system_time()
    {
      return ::kernel::scheduler().get_time() / 1000000.0;
    }

    static float
    system_shiftedTime()
    {
      runner::Runner& r = runner();
      return (r.scheduler_get().get_time() - r.time_shift_get()) / 1000000.0;
    }

    rObject
    eval(const std::string& code, rObject self)
    {
      return execute_parsed(parser::parse(code, ast::loc()),
                            SYMBOL(eval), code, self);
    }

    static rObject
    eval(rObject, const std::string& code)
    {
      return eval(code);
    }

    static rObject
    eval(rObject, const std::string& code, rObject self)
    {
      return eval(code, self);
    }

    static rObject
    system_searchFile(const rObject&, const std::string& filename)
    {
      try
      {
        return new Path(::kernel::urbiserver->find_file(filename));
      }
      catch (libport::file_library::Not_found&)
      {
        runner::raise_urbi(SYMBOL(FileNotFound), to_urbi(filename));
        // Never reached
        aver(false);
        return 0;
      }
    }

    static List::value_type
    system_searchPath()
    {
      List::value_type res;
      foreach (const libport::path& p,
               ::kernel::urbiserver->search_path.search_path_get())
        res << new Path(p);
      return res;
    }

    static void
    system_searchPathSet(const rObject&, List::value_type list)
    {
      ::kernel::urbiserver->search_path.search_path().clear();
      BOOST_FOREACH (rObject p, list)
      {
        rPath path = p->as<Path>();
        ::kernel::urbiserver->search_path.search_path()
          .push_back(path->value_get());
      }
    }

    static rObject
    loadFile(rObject, const std::string& filename, rObject self)
    {
      GD_FPUSH_TRACE("Load file: %s", filename);
#if defined ENABLE_SERIALIZATION
      if (!libport::path(filename).exists())
        runner::raise_urbi(SYMBOL(FileNotFound), to_urbi(filename));
#endif
      rObject res = execute_parsed(parser::parse_file(filename),
                                   SYMBOL(loadFile), filename, self);
      return res;
    }

    static rObject
    loadFile(rObject self, const std::string& filename)
    {
      return loadFile(self, filename, 0);
    }

    static rObject
    system_currentRunner()
    {
      return runner().as_job();
    }

    static float
    system_cycle()
    {
      return ::kernel::scheduler().cycle_get();
    }

    static void
    system_nonInterruptible()
    {
      runner().non_interruptible_set(true);
    }

    static rObject
    system_spawn(const rObject&,
                 const rCode& code,
                 const rObject& clear_tags)
    {
      runner::Runner& r = runner();
      runner::Interpreter* new_runner =
        new runner::Interpreter(interpreter(),
                                rObject(code),
                                libport::Symbol::fresh(r.name_get()));

      if (clear_tags->as_bool())
        new_runner->tag_stack_clear();

      new_runner->time_shift_set(r.time_shift_get());
      new_runner->start_job();
      return new_runner->as_job();
    }

    static rObject
    system_stats()
    {
      const sched::scheduler_stats_type& stats =
        ::kernel::scheduler().stats_get();

      // If statistics have just been reset, return "nil" since we cannot
      // return anything significant.
      if (stats.empty())
        return nil_class;

      // The space after "Symbol(" is mandatory to avoid triggering an error in
      // symbol generation code
      Dictionary::value_type res;
#define ADDSTAT(Suffix, Function, Divisor)      \
      res[new String("cycles" # Suffix)] = \
        new Float(stats.Function() / Divisor)
      ADDSTAT(, size, 1);
      ADDSTAT(Max, max, 1e6);
      ADDSTAT(Mean, mean, 1e6);
      ADDSTAT(Min, min, 1e6);
      ADDSTAT(StdDev, standard_deviation, 1e6);
      ADDSTAT(Variance, variance, 1e3);
#undef ADDSTAT
      return new Dictionary(res);
    }

    static void
    system_resetStats()
    {
      ::kernel::scheduler().stats_reset();
    }

    // This should give a backtrace as an urbi object.
    static void
    system_backtrace()
    {
      // FIXME: This method sucks a bit, because show_backtrace sucks a
      // bit, because our channeling/message-sending system sucks a lot.
      runner::Runner::backtrace_type bt = runner().backtrace_get();
      bt.pop_back();
      rforeach (runner::Runner::frame_type& i, bt)
        runner().send_message
        ("backtrace",
         libport::format("%s (%s)",
                         *i->getSlot(SYMBOL(name)),
                         *i->getSlot(SYMBOL(location))));
    }

    static List::value_type
    system_jobs()
    {
      List::value_type res;
      foreach (sched::rJob job, ::kernel::scheduler().jobs_get())
      {
        rObject o = dynamic_cast<runner::Runner*>(job.get())->as_job();
        if (o != nil_class)
          res << o;
      }
      return res;
    }

    static int
    system_aliveJobs()
    {
      return sched::Job::alive_jobs();
    }

    static void
    system_breakpoint()
    {
      return;
    }

#define SERVER_SET_VAR(Function, Variable, Value)       \
    static void                                         \
    system_ ## Function ()                              \
    {                                                   \
      ::kernel::urbiserver->Variable = Value;           \
    }

    SERVER_SET_VAR(stopall, stopall, true)

#undef SERVER_SET_VAR

    static rObject
    system_getenv(const rObject&, const std::string& name)
    {
      char* res = getenv(name.c_str());
      return res ? new String(res) : nil_class;
    }

    static rObject
    system_setenv(const rObject&, const std::string& name, rObject value)
    {
      std::string v = value->as_string();
      setenv(name.c_str(), v.c_str(), 1);
      return new String(v);
    }

    static rObject
    system_unsetenv(const rObject&, const std::string& name)
    {
      rObject res = system_getenv(0, name);
      unsetenv(name.c_str());
      return res;
    }

#define MESSAGE "aborting module loading because of "
    void
    initializations_run()
    {
      try
      {
        foreach (const Initialization& action, initializations_())
          action();
      }
      catch (const urbi::object::UrbiException& e)
      {
        initializations_().clear();
        throw;
      }
      catch (const std::exception& e)
      {
        initializations_().clear();
        runner::raise_primitive_error
          (libport::format(MESSAGE "fatal error: %s", e.what()));
      }
      catch (...)
      {
        initializations_().clear();
        runner::raise_primitive_error(MESSAGE "unkown exception");
      }
      initializations_().clear();
    }
#undef MESSAGE

    static void
    load(const std::string& filename, bool global)
    {
      libport::xlt_advise dl;
      dl.ext   ()
        .global(global)
        .path  (uobject_uobjects_path());

      libport::xlt_handle handle;
      try
      {
        handle = dl.open(filename);
      }
      catch(const libport::xlt_advise::exception& e)
      {
        RAISE(e.what());
      }

      handle.detach();

      // Reload uobjects
      uobjects_reload();

      initializations_run();
    }

    static void
    system_loadLibrary(const rObject&, const std::string& name)
    {
      load(name, true);
    }

    static void
    system_loadModule(const rObject&, const std::string& name)
    {
      load(name, false);
    }

    static libport::cli_args_type urbi_arguments_;
    static boost::optional<std::string> urbi_program_name_;

    void
    system_push_argument(const std::string& arg)
    {
      urbi_arguments_.push_back(arg);
    }

    void
    system_set_program_name(const std::string& name)
    {
      urbi_program_name_ = name;
    }

    static const libport::cli_args_type&
    system_arguments()
    {
      return urbi_arguments_;
    }

    static boost::optional<std::string>
    system_programName()
    {
      return urbi_program_name_;
    }

    static void
    system__exit(const rObject&, int status)
    {
      exit(status);
    }

    static int
    system_system(const rObject&, const std::string& s)
    {
      switch (int res = system(s.c_str()))
      {
        case -1:
          // FIXME: This is potentially widly used, see also path.cc.
          FRAISE("%1%: %2%", strerror(errno), s);
        case 127:
          FRAISE("shell failed: %1%", s);
        default:
          return res;
      }
    }

    static system_files_type system_files_;

    static void
    system_addSystemFile(const rObject&, rObject name)
    {
      system_files_.insert(libport::Symbol(from_urbi<std::string>(name)));
    }

    static void
    system_setSystemFiles(const rObject&,
                          const std::vector<libport::Symbol>& names)
    {
      system_files_.clear();
      system_files_.insert(names.begin(), names.end());
    }

    static std::vector<libport::Symbol>
    system_systemFiles(const rObject&)
    {
      return std::vector<libport::Symbol>(system_files_.begin(),
                                          system_files_.end());
    }

    system_files_type&
    system_files_get()
    {
      return system_files_;
    }

    bool
    is_system_location(const ast::loc& l)
    {
      return (l.begin.filename
              && libport::mhas(system_files_, *l.begin.filename));
    }

    void
    system_redefinitionMode()
    {
      runner().redefinition_mode_set(true);
    }

    rPath
    system_urbiRoot()
    {
      return new Path(::kernel::urbiserver->urbi_root_get().root());
    }

    void
    system_noVoidError()
    {
      runner().void_error_set(false);
    }

    static bool
    system_interactive()
    {
      return kernel::urbiserver->interactive_get();
    }

    static void
    system_poll()
    {
      libport::utime_t select_time = 0;
      // 0-delay in fast mode
      if (!system_class->slot_get(SYMBOL(fast))->as_bool())
      {
        libport::utime_t deadline = ::kernel::scheduler().deadline_get();
        if (deadline != sched::SCHED_IMMEDIATE)
          select_time = deadline - libport::utime();
      }
#ifdef WIN32
      // Linux and MacOS are using a POSIX file descriptor integrated in
      // the io_service for stdin, but not Windows.
      if (kernel::urbiserver->interactive_get())
        select_time = std::min((libport::utime_t)100000, select_time);
#else
      bool interactive =
        system_class->slot_get(SYMBOL(interactive))->as_bool();
      int flags = 0;
      if (interactive)
      {
        // Set stdin/out/err to non-blocking io.
        flags = fcntl(STDOUT_FILENO, F_GETFL);
        ERRNO_RUN(fcntl, STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);
      }
#endif
      boost::asio::io_service& ios = ::kernel::urbiserver->get_io_service();
      // Return without delay after the first operation, but perform all
      // pending operations
      if (select_time > 0)
        libport::pollFor(select_time, true, ios);
      ios.reset();
      ios.poll();
#ifndef WIN32
      if (interactive)
      {
        // Reset stdin/out/err flags.
        fcntl(STDOUT_FILENO, F_GETFL);
        ERRNO_RUN(fcntl, STDOUT_FILENO, F_SETFL, flags & ~O_NONBLOCK);
      }
#endif
    }

    static void
    system_pollLoop()
    {
      while(true)
      {
        system_poll();
        // We just waited the appropriate time, simply yield
        ::kernel::urbiserver->getCurrentRunner().yield();
      }
    }

    void
    system_class_initialize()
    {
#define DECLARE(Name)                                           \
      system_class->slot_set(SYMBOL(Name),                      \
                             primitive(&system_##Name))         \

      DECLARE(_exit);
      DECLARE(aliveJobs);
      DECLARE(arguments);
      DECLARE(backtrace);
      DECLARE(breakpoint);
      DECLARE(currentRunner);
      DECLARE(cycle);
      DECLARE(getenv);
      DECLARE(interactive);
      DECLARE(jobs);
      DECLARE(loadLibrary);
      DECLARE(loadModule);
      DECLARE(noVoidError);
      DECLARE(nonInterruptible);
      DECLARE(poll);
      DECLARE(pollLoop);
      DECLARE(programName);
      DECLARE(reboot);
      DECLARE(redefinitionMode);
      DECLARE(resetStats);
      DECLARE(searchFile);
      DECLARE(setenv);
      DECLARE(shiftedTime);
      DECLARE(shutdown);
      DECLARE(sleep);
      DECLARE(spawn);
      DECLARE(stats);
      DECLARE(stopall);
      DECLARE(system);
      DECLARE(time);
      DECLARE(unsetenv);
      DECLARE(urbiRoot);

      DECLARE(addSystemFile);
      DECLARE(setSystemFiles);
      DECLARE(systemFiles);

#undef DECLARE

      system_class->bind(SYMBOL(searchPath), &system_searchPath,
                         SYMBOL(searchPathSet), &system_searchPathSet);
      system_class->bind(SYMBOL(eval),
                         static_cast<rObject (*)(rObject, const std::string&)>(&eval));
      system_class->bind(SYMBOL(eval),
                         static_cast<rObject (*)(rObject, const std::string&, rObject)>(&eval));
      system_class->bind(SYMBOL(loadFile),
                         static_cast<rObject (*)(rObject, const std::string&)>(&loadFile));
      system_class->bind(SYMBOL(loadFile),
                         static_cast<rObject (*)(rObject, const std::string&, rObject)>(&loadFile));
    }

  } // namespace object
}
