/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
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

#include <memory>
#include <sstream>
#include <map>

#include <libport/asio.hh>
#include <libport/cerrno>
#include <libport/cstdlib>
#include <libport/format.hh>
#include <libport/locale.hh>
#include <libport/program-name.hh>
#include <libport/unistd.h>
#include <libport/xltdl.hh>

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
#include <object/profile.hh>
#include <object/symbols.hh>
#include <object/system.hh>
#include <urbi/object/tag.hh>
#include <urbi/object/job.hh>
#include <parser/transform.hh>
#include <runner/exception.hh>
#include <runner/job.hh>
#include <runner/state.hh>
#include <urbi/runner/raise.hh>

#include <eval/ast.hh>
#include <eval/call.hh>
#include <eval/send-message.hh>

GD_CATEGORY(Urbi.Object);

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

    using ::kernel::runner;

    namespace
    {

      static rObject
      execute_parsed(parser::parse_result_type p, rObject self)
      {
        ast::rConstAst ast = parser::transform(ast::rConstExp(p));
        runner::Job& run = runner();
        return
          eval::ast_context(run, ast.get(),
                            self ? self : rObject(run.state.lobby_get()));
      }

    }

    rObject system_class;

    /*--------------------.
    | System primitives.  |
    `--------------------*/

    static void
    system_breakpoint()
    {
      return;
    }

    static void
    system_reboot()
    {
      ::kernel::urbiserver->reboot();
    }

    static void
    system_shutdown(Object* /*self*/)
    {
      ::kernel::urbiserver->shutdown();
    }

    static void
    system_shutdown(Object* /*self*/, int rv)
    {
      ::kernel::urbiserver->shutdown(rv);
    }

#undef SERVER_FUNCTION

    static void
    system_sleep_inf(const rObject&)
    {
      runner::Job& r = runner();
      r.yield_until_terminated(r);
    }

    static void
    system_sleep(const rObject& o, libport::ufloat seconds)
    {
      runner::Job& r = runner();
      if (seconds == std::numeric_limits<ufloat>::infinity())
        system_sleep_inf(o);
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
      runner::Job& r = runner();
      return (r.scheduler_get().get_time() - r.time_shift_get()) / 1000000.0;
    }

    rObject
    eval(const std::string& code, rObject self)
      try
      {
        return execute_parsed(parser::parse(code, ast::loc()), self);
      }
      catch (const runner::Exception& e)
      {
        e.raise(code);
      }

    // This function is only used for debug purpose, by providing a mean to
    // create an std::string out of a C string which can be produced by gdb.
    rObject gdb_eval(const char* content);

    rObject
    gdb_eval(const char* content)
    {
      std::string code(content);
      return eval(code);
    }

    template <typename T>
    struct HexTo {
      T value;
      operator T() const {return value;}
      friend std::istream& operator>>(std::istream& in, HexTo& out) {
        in >> std::hex >> out.value;
        return in;
      }
    };

    // Call "0xffffffff".'$objAddr'()
    static rObject
    system_DOLLAR_objAddr(const std::string& addr)
    {
      size_t ptr = lexical_cast<HexTo<size_t> >(addr);
      return reinterpret_cast<Object*>(ptr);
    }

    static rObject
    system_eval(rObject, const std::string& code)
    {
      return eval(code);
    }

    static rObject
    system_eval(rObject, const std::string& code, rObject self)
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
    system_searchPath(const rObject&)
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
      foreach (rObject p, list)
      {
        rPath path = p->as<Path>();
        ::kernel::urbiserver->search_path.search_path()
          .push_back(path->value_get());
      }
    }

    static rObject
    system_loadFile(rObject, const std::string& filename, rObject self)
    {
      GD_FPUSH_TRACE("Load file: %s", filename);
#if defined ENABLE_SERIALIZATION
      if (!libport::path(filename).exists())
        runner::raise_urbi(SYMBOL(FileNotFound), to_urbi(filename));
#endif
      try
      {
        return execute_parsed(parser::parse_file(filename), self);
      }
      catch (const runner::Exception& e)
      {
        e.raise(filename);
      }
    }

    static rObject
    system_loadFile(rObject self, const std::string& filename)
    {
      return system_loadFile(self, filename, 0);
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

    static boost::optional<Dictionary::value_type>
    system_stats()
    {
      const sched::scheduler_stats_type& stats =
        ::kernel::scheduler().stats_get();

      // If statistics have just been reset, return "nil" since we cannot
      // return anything significant.
      if (stats.empty())
        return 0;

      // The space after "Symbol(" is mandatory to avoid triggering an error in
      // symbol generation code
      Dictionary::value_type res;
#define ADDSTAT(Suffix, Function, Divisor)      \
      res[new String("cycles" # Suffix)] =      \
        new Float(stats.Function() / Divisor)
      ADDSTAT(, size, 1);
      ADDSTAT(Max, max, 1e6);
      ADDSTAT(Mean, mean, 1e6);
      ADDSTAT(Min, min, 1e6);
      ADDSTAT(StdDev, standard_deviation, 1e6);
      ADDSTAT(Variance, variance, 1e3);
#undef ADDSTAT
      return res;
    }

    static void
    system_resetStats()
    {
      ::kernel::scheduler().stats_reset();
    }

    static void
    system_stopall ()
    {
      ::kernel::urbiserver->stopall = true;
    }

    static boost::optional<std::string>
    system_getenv(const rObject&, const std::string& name)
    {
      if (const char* res = getenv(name.c_str()))
        return std::string(res);
      return 0;
    }

    static std::string
    system_setenv(const rObject&, const std::string& name, rObject value)
    {
      std::string res = value->as_string();
      setenv(name.c_str(), res.c_str(), 1);
      return res;
    }

    static boost::optional<std::string>
    system_unsetenv(const rObject&, const std::string& name)
    {
      boost::optional<std::string> res = system_getenv(0, name);
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
      GD_FINFO_TRACE("load library %s from %s", filename,
                     uobject_uobjects_path());
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
    system_loadLibrary(const rObject&, const libport::path& name)
    {
      load(name, true);
    }

    static void
    system_loadModule(const rObject&, const libport::path& name)
    {
      load(name, false);
    }

    static libport::cli_args_type urbi_arguments_;
    static boost::optional<std::string> urbi_program_name_;
    // We will keep our environment in a dictionary.
    static rDictionary env_;

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

    static rDictionary
    system_env_init()
    {
      std::map<std::string, std::string> env = libport::getenviron();
      if (!env_)
        env_ = new Dictionary;
      else
        env_->clear();
      for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); ++it)
        env_->set (to_urbi(it->first), to_urbi(it->second));
      return env_;
    }

    static rDictionary
    system_env()
    {
      if (!env_)
        system_env_init();
      return env_;
    }

    /*---------------.
    | System files.  |
    `---------------*/

    static void
    system_addSystemFile(const rObject&, const std::string& name)
    {
      system_files_get().insert(libport::Symbol(name));
    }

    static void
    system_setSystemFiles(const rObject&,
                          const std::vector<libport::Symbol>& names)
    {
      system_files_get().clear();
      system_files_get().insert(names.begin(), names.end());
    }

    static std::vector<libport::Symbol>
    system_systemFiles(const rObject&)
    {
      return std::vector<libport::Symbol>(system_files_get().begin(),
                                          system_files_get().end());
    }

    system_files_type&
    system_files_get()
    {
      static system_files_type system_files_;
      return system_files_;
    }

    bool
    is_system_location(const ast::loc& l)
    {
      return (l.begin.filename
              && libport::has(system_files_get(), *l.begin.filename));
    }


    /*---------.
    | Locale.  |
    `---------*/

#define FORWARD_EXCEPTION(Command)              \
    try                                         \
    {                                           \
      Command;                                  \
    }                                           \
    catch (const std::runtime_error& e)         \
    {                                           \
      FRAISE(e.what());                         \
    }

    static void
    system_setLocale(rObject,
                     const std::string& cat, const std::string& loc)
      FORWARD_EXCEPTION(libport::setlocale(cat, loc));

    static void
    system_setLocale(rObject,
                     const std::string& cat)
      FORWARD_EXCEPTION(libport::setlocale(cat));

    static std::string
    system_getLocale(rObject,
                     const std::string& cat)
      FORWARD_EXCEPTION(return libport::setlocale(cat, 0));


    void
    system_redefinitionMode()
    {
      runner().state.redefinition_mode_set(true);
    }

    rPath
    system_urbiRoot()
    {
      return new Path(::kernel::urbiserver->urbi_root_get().root());
    }

    void
    system_noVoidError()
    {
      runner().state.void_error_set(false);
    }

    static bool
    system_interactive()
    {
      return kernel::urbiserver->interactive_get();
    }

    static rObject
    system_profile(Object* self, Executable* action)
    {
      if (runner().is_profiling())
      {
        runner::Exception::warn(
          runner().state.innermost_node_get()->location_get(),
          "already profiling");
        objects_type args;
        args << self;
        (*action)(args);
        return nil_class;
      }

      object::rProfile profile = new object::Profile;
      {
        FINALLY(((Object*, self)), ::kernel:: runner().profile_stop());
        runner().profile_start(profile, SYMBOL(LT_profiled_GT), action);
        objects_type args;
        args << self;
        (*action)(args);
      }

      return profile;
    }

    static sched::jobs_type dead_jobs_;

    static void
    system_poll()
    {
      dead_jobs_.clear();
      dead_jobs_ = kernel::scheduler().terminated_jobs_get();
      // let refcounting do the job.
      kernel::scheduler().terminated_jobs_clear();
      libport::utime_t select_time = 0;
      // 0-delay in fast mode
      static bool fast = system_class->call(SYMBOL(fast))->as_bool();
      if (!fast)
      {
        libport::utime_t deadline = ::kernel::scheduler().deadline_get();
        if (deadline != sched::SCHED_IMMEDIATE)
          select_time = std::max(deadline - libport::utime(),
                               (libport::utime_t)0);
      }
      static bool interactive =
        system_class->call(SYMBOL(interactive))->as_bool();
#ifdef WIN32
      // Linux and MacOS are using a POSIX file descriptor integrated in
      // the io_service for stdin, but not Windows.
      if (interactive)
        select_time = std::min((libport::utime_t)100000, select_time);
#else
      int flags = 0;
      if (interactive)
      {
        // Set stdin/out/err to non-blocking io.
        flags = fcntl(STDOUT_FILENO, F_GETFL);
        ERRNO_RUN(fcntl, STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);
      }
#endif
      boost::asio::io_service& ios = ::kernel::urbiserver->get_io_service();
      GD_FINFO_TRACE("Poll task will poll for %s", select_time);
      // Return without delay after the first operation, but perform all
      // pending operations.
      // pollFor has a minimum duration time (which depends on the OS), so
      // if we call it when our select_time is constantly below this value,
      // this will limit the CPU time urbi will use. So we set a threshold.
      if (select_time > 1000)
        libport::pollFor(select_time, true, ios);
      ios.reset();
      ios.poll();
#ifndef WIN32
      if (interactive)
        ERRNO_RUN(fcntl, STDOUT_FILENO, F_SETFL, flags & ~O_NONBLOCK);
#endif
    }

    static void
    system_pollLoop()
    {
      while (true)
      {
        system_poll();
        // We just waited the appropriate time, simply yield
        ::kernel::runner().yield();
      }
    }

    static std::string
    system_hostName()
    {
      static const size_t hostname_length_max = 1024;
      char result[hostname_length_max];
      if (gethostname(result, hostname_length_max))
      {
        if (errno == ENAMETOOLONG)
          FRAISE("Hostname is too long (maximum is %s characters).",
                 hostname_length_max);
        else
          RAISE("Unable to get hostname");
      }
      return result;
    }

    void
    system_class_initialize()
    {
#define DECLARE(Name)                                           \
      system_class->bind(SYMBOL_(Name), &system_##Name)

      DECLARE(_exit);
      DECLARE(addSystemFile);
      DECLARE(arguments);
      DECLARE(breakpoint);
      DECLARE(cycle);
      DECLARE(getLocale);
      DECLARE(getenv);
      DECLARE(hostName);
      DECLARE(interactive);
      DECLARE(loadLibrary);
      DECLARE(loadModule);
      DECLARE(noVoidError);
      DECLARE(nonInterruptible);
      DECLARE(poll);
      DECLARE(pollLoop);
      DECLARE(profile);
      DECLARE(programName);
      DECLARE(reboot);
      DECLARE(redefinitionMode);
      DECLARE(resetStats);
      DECLARE(searchFile);
      DECLARE(setSystemFiles);
      DECLARE(setenv);
      DECLARE(shiftedTime);
      system_class->bind(SYMBOL(shutdown),
                         static_cast<void (*)(Object*)>(&system_shutdown));
      system_class->bind(SYMBOL(shutdown),
                         static_cast<void (*)(Object*, int)>(&system_shutdown));
      DECLARE(stats);
      DECLARE(stopall);
      DECLARE(system);
      DECLARE(systemFiles);
      DECLARE(time);
      DECLARE(unsetenv);
      DECLARE(urbiRoot);
      DECLARE(DOLLAR_objAddr);

#undef DECLARE

      system_class->bind(SYMBOL(env), &system_env);
      system_class->bind(SYMBOL(initenv), &system_env_init);
      system_class->bind(SYMBOL(sleep), &system_sleep);
      system_class->bind(SYMBOL(sleep), &system_sleep_inf);
      system_class->bind(SYMBOL(searchPath), &system_searchPath,
                         &system_searchPathSet);

#define DECLARE(Name, Ret, ...)                                         \
      system_class->bind                                                \
        (SYMBOL_(Name),                                                 \
         static_cast<Ret (*) (rObject, __VA_ARGS__)>(&system_ ## Name))

      DECLARE(eval, rObject, const std::string&);
      DECLARE(eval, rObject, const std::string&, rObject);
      DECLARE(loadFile, rObject, const std::string&);
      DECLARE(loadFile, rObject, const std::string&, rObject);
      DECLARE(setLocale, void, const std::string& cat);
      DECLARE(setLocale, void, const std::string& cat, const std::string& loc);
#undef DECLARE
    }

  } // namespace object
}
