/**
 ** \file object/system-class.cc
 ** \brief Creation of the URBI object system.
 */

#include <ltdl.h>

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>
#include <libport/cstdlib>
#include <libport/program-name.hh>

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
#include <object/lobby.hh>
#include <object/list.hh>
#include <object/path.hh>
#include <object/primitives.hh>
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
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    runner::Interpreter& run = dynamic_cast<runner::Interpreter&>(r);

    // Report potential errors
    {
      ast::rNary errs = new ast::Nary();
      p->process_errors(*errs);
      run(errs.get());
    }

    ast::rConstAst ast = parser::transform(p->ast_get());
    if (!ast)
      runner::raise_primitive_error(e);

    runner::Interpreter* sub = new runner::Interpreter(run, ast, fun);
    // So that it will resist to the call to yield_until_terminated,
    // and will be reclaimed at the end of the scope.
    sched::rJob job = sub;
    libport::Finally finally;
    r.register_child(sub, finally);
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
  system_ ## Function ()                        \
  {                                             \
    urbiserver->Function();                     \
  }

  SERVER_FUNCTION(reboot)
  SERVER_FUNCTION(shutdown)

#undef SERVER_FUNCTION


  static rObject
  system_class_sleep(objects_type args)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    check_arg_count(args.size() - 1, 1);

    type_check<Float>(args[1]);

    rFloat arg1 = args[1]->as<Float>();
    libport::utime_t deadline;
    if (arg1->value_get() == std::numeric_limits<ufloat>::infinity())
      r.yield_until_terminated(r);
    else
    {
      deadline = r.scheduler_get().get_time() +
	static_cast<libport::utime_t>(arg1->value_get() * 1000000.0);
      r.yield_until (deadline);
    }
    return void_class;
  }

  static float
  system_time()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    return r.scheduler_get().get_time() / 1000000.0;
  }

  static float
  system_shiftedTime()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    return (r.scheduler_get().get_time() - r.time_shift_get()) / 1000000.0;
  }

  static rObject
  system_class_assert_(objects_type args)
  {
    check_arg_count(args.size() - 1, 2);
    type_check<String>(args[2]);
    rString arg2 = args[2]->as<String>();
    if (!is_true(args[1]))
      runner::raise_primitive_error("assertion `" + arg2->value_get() +
				    "' failed");
    return void_class;
  }

  static rObject
  system_class_eval(objects_type args)
  {
    check_arg_count(args.size() - 1, 1);
    type_check<String>(args[1]);
    rString arg1 = args[1]->as<String>();
    return
      execute_parsed(parser::parse(arg1->value_get(), ast::loc()),
                     SYMBOL(eval),
                     "error executing command: " + arg1->value_get());
  }

  static rObject
  system_class_registerAtJob (objects_type args)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    check_arg_count(args.size() - 1, 3);
    runner::register_at_job(dynamic_cast<runner::Interpreter&>(r),
			    args[1], args[2], args[3]);
    return object::void_class;
  }

  static rObject
  system_class_scopeTag(objects_type args)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    check_arg_count(args.size() - 1, 0);
    const sched::rTag& scope_tag =
      dynamic_cast<runner::Interpreter&>(r).scope_tag();
    return new Tag(scope_tag);
  }

  static rObject
  system_class_searchFile(objects_type args)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

    check_arg_count(args.size() - 1, 1);
    const std::string filename = filename_get(args[1]);

    kernel::UServer& s = r.lobby_get()->connection_get().server_get();
    try
    {
      return new Path(s.find_file(filename));
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
  system_class_loadFile(objects_type args)
  {
    check_arg_count(args.size() - 1, 1);
    const std::string filename = filename_get(args[1]);

    if (!libport::path(filename).exists())
      runner::raise_urbi(SYMBOL(FileNotFound), to_urbi(filename));
    return
      execute_parsed(parser::parse_file(filename),
                     SYMBOL(loadFile),
		     "error loading file: " + filename);
  }

  static rObject
  system_class_currentRunner (objects_type args)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    check_arg_count(args.size() - 1, 0);
    return r.as_task();
  }

  static float
  system_cycle()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    return r.scheduler_get ().cycle_get ();
  }

  static libport::Symbol
  system_fresh()
  {
    return libport::Symbol::fresh();
  }

  static rObject
  system_class_lobby (objects_type args)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    check_arg_count(args.size() - 1, 0);
    return r.lobby_get();
  }

  static void
  system_nonInterruptible()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    r.non_interruptible_set(true);
  }

  static void
  system_quit()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    r.lobby_get()->connection_get().close();
  }

  static void
  system_spawn(const rObject&,
	       const rCode& code,
               const rObject& clear_tags)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

    const runner::Interpreter& current_runner =
      dynamic_cast<runner::Interpreter&>(r);
    runner::Interpreter* new_runner =
      new runner::Interpreter (current_runner,
			       rObject(code),
			       libport::Symbol::fresh(r.name_get()));

    if (is_true(clear_tags))
      new_runner->tag_stack_clear();

    new_runner->time_shift_set (r.time_shift_get ());
    new_runner->start_job ();
  }

  static rObject
  system_class_stats(objects_type args)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    check_arg_count(args.size() - 1, 0);
    Dictionary::value_type res;
    const sched::scheduler_stats_type& stats =
      r.scheduler_get().stats_get();

    // If statistics have just been reset, return "nil" since we cannot
    // return anything significant.
    if (stats.empty())
      return nil_class;

    // The space after "Symbol(" is mandatory to avoid triggering an error in
    // symbol generation code
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
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    r.scheduler_get().stats_reset();
  }

  // This should give a backtrace as an urbi object.
  static void
  system_backtrace()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    // FIXME: This method sucks a bit, because show_backtrace sucks a
    // bit, because our channeling/message-sending system sucks a lot.
    runner::Runner::backtrace_type bt = r.backtrace_get();
    bt.pop_back();
    foreach (const runner::Runner::frame_type& elt,
	     boost::make_iterator_range(boost::rbegin(bt),
					boost::rend(bt)))
      r.send_message("backtrace", elt.first + " (" + elt.second + ")");
  }

  static List::value_type
  system_jobs()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    List::value_type res;
    foreach(sched::rJob job, r.scheduler_get().jobs_get())
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

  static rObject system_getenv(rObject, const std::string& name)
  {
    char* res = getenv(name.c_str());
    return res ? new String(res) : nil_class;
  }

  static rObject system_setenv(rObject, const std::string& name, rObject value)
  {
    rString v = value->call(SYMBOL(asString))->as<String>();
    setenv(name.c_str(), v->value_get().c_str(), 1);
    return v;
  }

  static rObject system_unsetenv(rObject, const std::string& name)
  {
    rObject res = system_getenv(0, name);
    unsetenv(name.c_str());
    return res;
  }

  static libport::InstanceTracker<Lobby>::set_type system_lobbies()
  {
    return Lobby::instances_get();
  }


  static void system_loadModule(rObject, const std::string& name)
  {
    static bool initialized = false;

    if (!initialized)
    {
      initialized = true;
      lt_dlinit();
    }
    lt_dlhandle handle = lt_dlopenext(name.c_str());
    if (!handle)
      runner::raise_primitive_error
        ("Failed to open `" + name + "': " + lt_dlerror());

    // Reload uobjects
    uobjects_reload();

    // Reload CxxObjects
    CxxObject::create();
    CxxObject::initialize(global_class);
    CxxObject::cleanup();
  }

  static libport::cli_args_type urbi_arguments_;
  static boost::optional<std::string> urbi_program_name_;

  void system_push_argument(const std::string& arg)
  {
    urbi_arguments_.push_back(arg);
  }

  void system_set_program_name(const std::string& name)
  {
    urbi_program_name_ = name;
  }

  static const libport::cli_args_type& system_arguments()
  {
    return urbi_arguments_;
  }

  static boost::optional<std::string> system_programName()
  {
    return urbi_program_name_;
  }

  static void system__exit(rObject, int status)
  {
    exit(status);
  }

  void
  system_class_initialize ()
  {
#define DECLARE(Name)                                                   \
    system_class->slot_set(SYMBOL(Name),                                \
                           make_primitive(&system_##Name))              \

    DECLARE(_exit);
    DECLARE(aliveJobs);
    DECLARE(arguments);
    DECLARE(backtrace);
    DECLARE(breakpoint);
    DECLARE(cycle);
    DECLARE(fresh);
    DECLARE(getenv);
    DECLARE(jobs);
    DECLARE(loadModule);
    DECLARE(lobbies);
    DECLARE(nonInterruptible);
    DECLARE(programName);
    DECLARE(quit);
    DECLARE(reboot);
    DECLARE(resetStats);
    DECLARE(searchPath);
    DECLARE(setenv);
    DECLARE(shiftedTime);
    DECLARE(shutdown);
    DECLARE(spawn);
    DECLARE(stopall);
    DECLARE(time);
    DECLARE(unsetenv);

#undef DECLARE

    /// \a Call gives the name of the C++ function, and \a Name that in Urbi.
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(system, Name)

    DECLARE(assert_);
    DECLARE(currentRunner);
    DECLARE(eval);
    DECLARE(loadFile);
    DECLARE(lobby);
    DECLARE(registerAtJob);
    DECLARE(scopeTag);
    DECLARE(searchFile);
    DECLARE(stats);
    DECLARE(sleep);
#undef DECLARE
  }

}; // namespace object
