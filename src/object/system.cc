/**
 ** \file object/system-class.cc
 ** \brief Creation of the URBI object system.
 */

#include <ltdl.h>

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>
#include <libport/cstdlib>
#include <libport/format.hh>
#include <libport/program-name.hh>
#include <libport/xltdl.hh>

#include <cerrno>
#include <memory>
#include <sstream>

#include <kernel/uconnection.hh>
#include <kernel/uobject.hh>
#include <kernel/userver.hh>

#include <object/code.hh>
#include <object/cxx-primitive.hh>
#include <object/dictionary.hh>
#include <object/float.hh>
#include <object/global.hh>
#include <object/list.hh>
#include <object/lobby.hh>
#include <object/path.hh>
#include <object/symbols.hh>
#include <object/system.hh>
#include <object/tag.hh>
#include <object/task.hh>
#include <parser/transform.hh>
#include <runner/at-handler.hh>
#include <runner/interpreter.hh>
#include <runner/raise.hh>
#include <runner/runner.hh>

#include <ast/nary.hh>
#include <ast/routine.hh>

namespace object
{
  using kernel::urbiserver;


  static inline
  runner::Runner&
  runner()
  {
    return ::kernel::urbiserver->getCurrentRunner();
  }

  static inline
  runner::Interpreter&
  interpreter()
  {
    return dynamic_cast<runner::Interpreter&>(runner());
  }

  static inline
  sched::Scheduler&
  scheduler()
  {
    return runner().scheduler_get();
  }

  // Extract a filename from a String or a Path object
  static std::string
  filename_get(const rObject& o)
  {
    if (o.is_a<Path>())
      return o->as<Path>()->as_string();
    type_check<String>(o);
    return o->as<String>()->value_get();
  }


  rObject
  execute_parsed(parser::parse_result_type p,
                 libport::Symbol fun, std::string e)
  {
    runner::Interpreter& run = interpreter();

    // Report potential errors
    {
      ast::rNary errs = new ast::Nary();
      p->process_errors(*errs);
      run(errs.get());
    }

    ast::rConstAst ast = parser::transform(p->ast_get());
    if (!ast)
      RAISE(e);

    runner::Interpreter* sub = new runner::Interpreter(run, ast, fun);
    // So that it will resist to the call to yield_until_terminated,
    // and will be reclaimed at the end of the scope.
    sched::rJob job = sub;
    sched::Job::ChildrenCollecter children(&run, 1);
    run.register_child(sub, children);
    sub->start_job();
    try
    {
      run.yield_until_terminated(*job);
    }
    catch (const sched::ChildException& ce)
    {
      // Kill the sub-job and propagate.
      ce.rethrow_child_exception();
    }
    return sub->result_get();
  }


  rObject system_class;

  /*--------------------.
  | System primitives.  |
  `--------------------*/


#define SERVER_FUNCTION(Function)               \
  static void                                   \
  system_ ## Function()                         \
  {                                             \
    urbiserver->Function();                     \
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
      r.yield_until(r.scheduler_get().get_time()
                    + static_cast<libport::utime_t>(seconds * 1000000.0));
  }

  static float
  system_time()
  {
    return scheduler().get_time() / 1000000.0;
  }

  static float
  system_shiftedTime()
  {
    runner::Runner& r = runner();
    return (r.scheduler_get().get_time() - r.time_shift_get()) / 1000000.0;
  }

  static rObject
  system_eval(const rObject&, const std::string& code)
  {
    return execute_parsed(parser::parse(code, ast::loc()),
                          SYMBOL(eval),
                          "error executing command: " + code);
  }

  static void
  system_registerAtJob(const rObject&,
                       const rObject& condition,
                       const rObject& clause,
                       const rObject& on_leave)
  {
    runner::register_at_job(interpreter(),
			    condition, clause, on_leave);
  }

  static rObject
  system_scopeTag()
  {
    return new Tag(interpreter().scope_tag());
  }

  static rObject
  system_searchFile(const rObject&, const rObject& f)
  {
    const std::string& filename = filename_get(f);
    try
    {
      return new Path(urbiserver->find_file(filename));
    }
    catch (libport::file_library::Not_found&)
    {
      runner::raise_urbi(SYMBOL(FileNotFound), to_urbi(filename));
      // Never reached
      assertion(false);
      return 0;
    }
  }

  static List::value_type
  system_searchPath()
  {
    List::value_type res;
    foreach (const libport::path& p, urbiserver->search_path.search_path_get())
      res.push_back(new Path(p));
    return res;
  }

  static rObject
  system_loadFile(const rObject&, const rObject& f)
  {
    const std::string& filename = filename_get(f);
#if defined ENABLE_SERIALIZATION
    if (!libport::path(filename).exists())
      runner::raise_urbi(SYMBOL(FileNotFound), to_urbi(filename));
#endif
    return execute_parsed(parser::parse_file(filename),
                          SYMBOL(loadFile),
                          "error loading file: " + filename);
  }

  static rObject
  system_currentRunner()
  {
    return runner().as_task();
  }

  static float
  system_cycle()
  {
    return scheduler().cycle_get();
  }

  static libport::Symbol
  system_fresh()
  {
    return libport::Symbol::fresh();
  }

  static rObject
  system_lobby()
  {
    return runner().lobby_get();
  }

  static void
  system_nonInterruptible()
  {
    runner().non_interruptible_set(true);
  }

  static void
  system_quit()
  {
    runner().lobby_get()->connection_get().close();
  }

  static void
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
  }

  static rObject
  system_stats()
  {
    const sched::scheduler_stats_type& stats = scheduler().stats_get();

    // If statistics have just been reset, return "nil" since we cannot
    // return anything significant.
    if (stats.empty())
      return nil_class;

    // The space after "Symbol(" is mandatory to avoid triggering an error in
    // symbol generation code
    Dictionary::value_type res;
#define ADDSTAT(Suffix, Function, Divisor)      \
    res[libport::Symbol("cycles" # Suffix)] =   \
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
    scheduler().stats_reset();
  }

  // This should give a backtrace as an urbi object.
  static void
  system_backtrace()
  {
    // FIXME: This method sucks a bit, because show_backtrace sucks a
    // bit, because our channeling/message-sending system sucks a lot.
    runner::Runner::backtrace_type bt = runner().backtrace_get();
    bt.pop_back();
    foreach (const runner::Runner::frame_type& elt,
	     boost::make_iterator_range(boost::rbegin(bt),
					boost::rend(bt)))
      runner().send_message("backtrace", elt.first + " (" + elt.second + ")");
  }

  static List::value_type
  system_jobs()
  {
    List::value_type res;
    foreach(sched::rJob job, scheduler().jobs_get())
      res.push_back(dynamic_cast<runner::Runner*>(job.get())->as_task());
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
  static void                                           \
  system_ ## Function ()                                \
  {                                                     \
    urbiserver->Variable = Value;                       \
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
    rString v = value->call(SYMBOL(asString))->as<String>();
    setenv(name.c_str(), v->value_get().c_str(), 1);
    return v;
  }

  static rObject
  system_unsetenv(const rObject&, const std::string& name)
  {
    rObject res = system_getenv(0, name);
    unsetenv(name.c_str());
    return res;
  }

  static libport::InstanceTracker<Lobby>::set_type
  system_lobbies()
  {
    return Lobby::instances_get();
  }

  static void
  load(const rObject& name, bool global)
  {
    const std::string& filename = filename_get(name);

    libport::xlt_advise dl;
    dl.ext().global(global).path()
      .push_back(libport::xgetenv("URBI_UOBJECT_PATH", ".:"), ":");

    libport::xlt_handle handle = dl.open(filename);
    if (!handle.handle)
      RAISE("Failed to open `" + filename + "': " + lt_dlerror());
    handle.detach();

    // Reload uobjects
    uobjects_reload();

    // Reload CxxObjects
    CxxObject::create();
    CxxObject::initialize(global_class);
    CxxObject::cleanup();
  }

  static void
  system_loadLibrary(const rObject&, const rObject& name)
  {
    load(name, true);
  }

  static void
  system_loadModule(const rObject&, const rObject& name)
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
    int res = system(s.c_str());
    switch (res)
    {
    case -1:
      // FIXME: This is potentially widly used, see also path.cc.
      RAISE(libport::format("%1%: %2%", strerror(errno), s));
      break;
    case 127:
      RAISE(libport::format("shell failed: %1%", s));
      break;
    }
    return res;
  }

  void
  system_class_initialize()
  {
#define DECLARE(Name)                                                   \
    system_class->slot_set(SYMBOL(Name),                                \
                           make_primitive(&system_##Name))              \

    DECLARE(_exit);
    DECLARE(aliveJobs);
    DECLARE(arguments);
    DECLARE(backtrace);
    DECLARE(breakpoint);
    DECLARE(currentRunner);
    DECLARE(cycle);
    DECLARE(eval);
    DECLARE(fresh);
    DECLARE(getenv);
    DECLARE(jobs);
    DECLARE(loadFile);
    DECLARE(loadModule);
    DECLARE(loadLibrary);
    DECLARE(lobbies);
    DECLARE(lobby);
    DECLARE(nonInterruptible);
    DECLARE(programName);
    DECLARE(quit);
    DECLARE(reboot);
    DECLARE(registerAtJob);
    DECLARE(resetStats);
    DECLARE(scopeTag);
    DECLARE(searchFile);
    DECLARE(searchPath);
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

#undef DECLARE
  }

} // namespace object
