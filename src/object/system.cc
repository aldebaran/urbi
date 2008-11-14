/**
 ** \file object/system-class.cc
 ** \brief Creation of the URBI object system.
 */

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>
#include <libport/cstdlib>

#include <memory>
#include <sstream>

#include <kernel/userver.hh>
#include <kernel/uconnection.hh>

#include <object/code.hh>
#include <object/cxx-primitive.hh>
#include <object/dictionary.hh>
#include <object/float.hh>
#include <object/global.hh>
#include <object/lobby.hh>
#include <object/list.hh>
#include <object/path.hh>
#include <object/system.hh>
#include <object/tag.hh>
#include <object/task.hh>
#include <parser/transform.hh>
#include <runner/at-handler.hh>
#include <runner/call.hh>
#include <runner/interpreter.hh>
#include <runner/raise.hh>
#include <runner/runner.hh>

#include <ast/nary.hh>
#include <ast/routine.hh>

namespace object
{

  // Extract a filename from a String or a Path object
  static std::string
  filename_get(const rObject& o)
  {
    if (o.is_a<Path>())
      return o->as<Path>()->as_string();
    type_check(o, String::proto);
    return o->as<String>()->value_get();
  }

  rObject
  execute_parsed(runner::Runner& r,
                 parser::parse_result_type p,
                 libport::Symbol fun, std::string e)
  {
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
    scheduler::rJob job = sub;
    libport::Finally finally;
    r.register_child(sub, finally);
    sub->start_job();
    try
    {
      run.yield_until_terminated(*job);
    }
    catch (const scheduler::ChildException& ce)
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


#define SERVER_FUNCTION(Function)					\
  static rObject							\
  system_class_ ## Function (runner::Runner&, objects_type args)	\
  {									\
    check_arg_count(args.size() - 1, 0);                                \
    ::urbiserver->Function();						\
    return void_class;							\
  }

  SERVER_FUNCTION(reboot)
  SERVER_FUNCTION(shutdown)

#undef SERVER_FUNCTION


  static rObject
  system_class_sleep (runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 1);

    type_check(args[1], Float::proto);

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

  static rObject
  system_class_time(runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    return new Float(r.scheduler_get().get_time() / 1000000.0);
  }

  static rObject
  system_class_shiftedTime(runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    return new Float((r.scheduler_get().get_time() -
			  r.time_shift_get()) / 1000000.0);
  }

  static rObject
  system_class_assert_(runner::Runner&, objects_type args)
  {
    check_arg_count(args.size() - 1, 2);
    type_check(args[2], String::proto);
    rString arg2 = args[2]->as<String>();
    if (!is_true(args[1]))
      runner::raise_primitive_error("assertion `" + arg2->value_get() +
				    "' failed");
    return void_class;
  }

  static rObject
  system_class_eval(runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 1);
    type_check(args[1], String::proto);
    rString arg1 = args[1]->as<String>();
    return
      execute_parsed(r, parser::parse(arg1->value_get()),
                     SYMBOL(eval),
                     "error executing command: " + arg1->value_get());
  }

  static rObject
  system_class_registerAtJob (runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 3);
    runner::register_at_job(dynamic_cast<runner::Interpreter&>(r),
			    args[1], args[2], args[3]);
    return object::void_class;
  }

  static rObject
  system_class_scopeTag(runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    const scheduler::rTag& scope_tag =
      dynamic_cast<runner::Interpreter&>(r).scope_tag();
    return new Tag(scope_tag);
  }

  static rObject
  system_class_searchFile (runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 1);
    const std::string filename = filename_get(args[1]);

    UServer& s = r.lobby_get()->value_get().connection.server_get();
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

  static rObject
  system_class_searchPath(runner::Runner&, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    List::value_type res;
    foreach (const libport::path& p,
	     ::urbiserver->search_path.search_path_get())
      res.push_back(new Path(p));
    return to_urbi(res);
  }

  static rObject
  system_class_loadFile(runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 1);
    const std::string filename = filename_get(args[1]);

    if (!libport::path(filename).exists())
      runner::raise_urbi(SYMBOL(FileNotFound), to_urbi(filename));
    return
      execute_parsed(r, parser::parse_file(filename),
                     SYMBOL(loadFile),
		     "error loading file: " + filename);
  }

  static rObject
  system_class_currentRunner (runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    return r.as_task();
  }

  static rObject
  system_class_cycle (runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    return new Float(r.scheduler_get ().cycle_get ());
  }

  static rObject
  system_class_fresh (runner::Runner&, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    return new String(libport::Symbol::fresh());
  }

  static rObject
  system_class_lobby (runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    return r.lobby_get();
  }

  static rObject
  system_class_nonInterruptible (runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    r.non_interruptible_set (true);
    return void_class;
  }

  static rObject
  system_class_quit (runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    r.lobby_get()->value_get().connection.close();
    return void_class;
  }

  static void
  system_spawn(runner::Runner& r, const rObject&,
	       const rCode& code, const rObject& clear_tags)
  {
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
  system_class_stats(runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    Dictionary::value_type res;
    const scheduler::scheduler_stats_type& stats =
      r.scheduler_get().stats_get();
    // The space after "Symbol(" is mandatory to avoid triggering an error in
    // symbol generation code
#define ADDSTAT(Suffix, Function, Divisor)				\
    res[libport::Symbol( "cycles" # Suffix)] = new Float(stats.Function() / Divisor)
    ADDSTAT(, size, 1);
    ADDSTAT(Max, max, 1000.0);
    ADDSTAT(Mean, mean, 1000.0);
    ADDSTAT(Min, min, 1000.0);
    ADDSTAT(StdDev, standard_deviation, 1000.0);
    ADDSTAT(Variance, variance, 1000.0);
#undef ADDSTAT
    return new Dictionary(res);
  }

  static rObject
  system_class_platform(runner::Runner&, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
#ifdef WIN32
    return to_urbi(SYMBOL(WIN32));
#else
    return to_urbi(SYMBOL(POSIX));
#endif
  }

  // This should give a backtrace as an urbi object.
  static rObject
  system_class_backtrace(runner::Runner& r, objects_type args)
  {
    // FIXME: This method sucks a bit, because show_backtrace sucks a
    // bit, because our channeling/message-sending system sucks a lot.
    check_arg_count(args.size() - 1, 0);
    runner::Runner::backtrace_type bt = r.backtrace_get();
    bt.pop_back();
    foreach (const runner::Runner::frame_type& elt,
	     boost::make_iterator_range(boost::rbegin(bt),
					boost::rend(bt)))
      r.send_message("backtrace", elt.first + " (" + elt.second + ")");
    return void_class;
  }

  static rObject
  system_class_jobs(runner::Runner& r, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    List::value_type res;
    foreach(scheduler::rJob job, r.scheduler_get().jobs_get())
      res.push_back(dynamic_cast<runner::Runner*>(job.get())->as_task());
    return new List(res);
  }

  static rObject
  system_class_aliveJobs(runner::Runner&, objects_type args)
  {
    check_arg_count(args.size() - 1, 0);
    return new Float(scheduler::Job::alive_jobs());
  }

  static rObject
  system_class_breakpoint(runner::Runner&, objects_type)
  {
    return void_class;
  }

#define SERVER_SET_VAR(Function, Variable, Value)			\
  static rObject							\
  system_class_ ## Function (runner::Runner&, objects_type args)	\
  {									\
    check_arg_count(args.size() - 1, 0);                                \
    ::urbiserver->Variable = Value;					\
    return void_class;							\
  }

  SERVER_SET_VAR(debugoff, debugOutput, false)
  SERVER_SET_VAR(debugon, debugOutput, true)
  SERVER_SET_VAR(stopall, stopall, true)

#undef SERVER_SET_VAR

  static rObject system_getenv(rObject, const std::string& name)
  {
    char* res = getenv(name.c_str());
    return res ? new String(res) : nil_class;
  }

  static rObject system_setenv(runner::Runner& r, rObject,
                               const std::string& name, rObject value)
  {
    rString v = urbi_call(r, value, SYMBOL(asString))->as<String>();
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

  void
  system_class_initialize ()
  {
#define DECLARE(Name)                                                   \
    system_class->slot_set                                              \
      (SYMBOL(Name),                                                    \
       make_primitive(&system_##Name))                                  \

    DECLARE(getenv);
    DECLARE(lobbies);
    DECLARE(setenv);
    DECLARE(spawn);
    DECLARE(unsetenv);

#undef DECLARE

    /// \a Call gives the name of the C++ function, and \a Name that in Urbi.
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(system, Name)

    DECLARE(aliveJobs);
    DECLARE(assert_);
    DECLARE(backtrace);
    DECLARE(breakpoint);
    DECLARE(currentRunner);
    DECLARE(cycle);
    DECLARE(debugoff);
    DECLARE(debugon);
    DECLARE(eval);
    DECLARE(fresh);
    DECLARE(jobs);
    DECLARE(loadFile);
    DECLARE(lobby);
    DECLARE(nonInterruptible);
    DECLARE(quit);
    DECLARE(reboot);
    DECLARE(registerAtJob);
    DECLARE(scopeTag);
    DECLARE(searchFile);
    DECLARE(searchPath);
    DECLARE(shiftedTime);
    DECLARE(stats);
    DECLARE(shutdown);
    DECLARE(sleep);
    DECLARE(stopall);
    DECLARE(platform);
    DECLARE(time);
#undef DECLARE
  }

}; // namespace object
