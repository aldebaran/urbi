/**
 ** \file object/system-class.cc
 ** \brief Creation of the URBI object system.
 */

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include <memory>
#include <sstream>
#include <signal.h>

#include <binder/bind.hh>

#include <flower/flow.hh>

#include <kernel/userver.hh>
#include <kernel/uconnection.hh>

#include <object/code-class.hh>
#include <object/dictionary-class.hh>
#include <object/float-class.hh>
#include <object/list-class.hh>
#include <object/system-class.hh>
#include <object/tag-class.hh>
#include <object/task-class.hh>

#include <runner/at-handler.hh>
#include <runner/runner.hh>
#include <runner/interpreter.hh>

#include <ast/nary.hh>

namespace object
{
  rObject system_class;

  /*--------------------.
  | System primitives.  |
  `--------------------*/


#define SERVER_FUNCTION(Function)					\
  static rObject							\
  system_class_ ## Function (runner::Runner&, objects_type args)	\
  {									\
    CHECK_ARG_COUNT (1);						\
    ::urbiserver->Function();						\
    return void_class;							\
  }

  SERVER_FUNCTION(reboot)
  SERVER_FUNCTION(shutdown)

#undef SERVER_FUNCTION


  static rObject
  system_class_sleep (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    type_check<Float>(args[1], SYMBOL(sleep));
    rFloat arg1 = args[1]->as<Float>();
    libport::utime_t deadline;
    if (arg1->value_get() == std::numeric_limits<ufloat>::infinity())
      deadline = std::numeric_limits<libport::utime_t>::max();
    else
      deadline = r.scheduler_get().get_time() +
	static_cast<libport::utime_t>(arg1->value_get() * 1000.0);
    r.yield_until (deadline);
    return void_class;
  }

  static rObject
  system_class_time (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return new Float(r.scheduler_get().get_time() / 1000.0);
  }

  static rObject
  system_class_shiftedTime (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return new Float((r.scheduler_get().get_time() -
			  r.time_shift_get()) / 1000.0);
  }

  rObject
  execute_parsed (runner::Runner& r,
                  parser::parse_result_type p, UrbiException e)
  {
    ast::rNary errs = new ast::Nary();
    p->process_errors(*errs);
    dynamic_cast<runner::Interpreter&>(r)(errs);
    if (ast::rNary ast = p->ast_get())
    {
      ast = binder::bind(flower::flow(ast));
      assert(ast);
      return dynamic_cast<runner::Interpreter&>(r).eval(ast);
    }
    else
      throw e;
  }

  static rObject
  system_class_assert_(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(3);
    type_check<String>(args[2], SYMBOL(assert));
    rString arg2 = args[2]->as<String>();
    if (!is_true(args[1], SYMBOL(assert_)))
      throw PrimitiveError
	("assert_",
	 "assertion `" + arg2->value_get().name_get() + "' failed");
    return void_class;
  }

  static rObject
  system_class_eval (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    type_check<String>(args[1], SYMBOL(assert));
    rString arg1 = args[1]->as<String>();
    return
      execute_parsed(r,
                     parser::parse(arg1->value_get()),
                     PrimitiveError("", "error executing command"));
  }

  static rObject
  system_class_registerAtJob (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT(4);
    runner::register_at_job(dynamic_cast<runner::Interpreter&>(r),
			    args[1], args[2], args[3]);
    return object::void_class;
  }

  static rObject
  system_class_scopeTag(runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    scheduler::rTag scope_tag =
      dynamic_cast<runner::Interpreter&>(r).scope_tag();
    return new Tag(scope_tag);
  }

  static rObject
  system_class_searchFile (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    type_check<String>(args[1], SYMBOL(assert));
    rString arg1 = args[1]->as<String>();

    UServer& s = r.lobby_get()->value_get().connection.server_get();
    try
    {
      return new String(libport::Symbol(
			     s.find_file(arg1->value_get ().name_get ())));
    }
    catch (libport::file_library::Not_found&)
    {
      throw
	PrimitiveError("searchFile",
		       "Unable to find file: "
		       + arg1->value_get().name_get());
      // Never reached
      assertion(false);
      return 0;
    }
  }

  static rObject
  system_class_loadFile (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    type_check<String>(args[1], SYMBOL(assert));
    rString arg1 = args[1]->as<String>();

    std::string filename = arg1->value_get().name_get();

    if (!libport::path (filename).exists ())
      throw PrimitiveError("loadFile",
			   "No such file: " + filename);

    parser::parse_result_type res = parser::parse_file(filename);

    // Report potential errors
    {
      ast::rNary errs = new ast::Nary();
      res->process_errors(*errs);
      dynamic_cast<runner::Interpreter&>(r)(errs);
    }

    ast::rConstAst ast = binder::bind(flower::flow(res->ast_get()));
    if (!ast)
      throw PrimitiveError("", //same message than k1
                           "Error loading file: " + filename);

    runner::Interpreter* sub =
      new runner::Interpreter(dynamic_cast<runner::Interpreter&>(r),
                              ast, SYMBOL(load));
    dynamic_cast<runner::Interpreter&>(r).link(sub);
    sub->start_job();
    r.yield_until_terminated(*sub);

    return object::void_class;
  }

  static rObject
  system_class_currentRunner (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return new Task(&r);
  }

  static rObject
  system_class_cycle (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return new Float(r.scheduler_get ().cycle_get ());
  }

  static rObject
  system_class_fresh (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return new String(libport::Symbol::fresh());
  }

  static rObject
  system_class_lobby (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return r.lobby_get();
  }

  static rObject
  system_class_nonInterruptible (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    r.non_interruptible_set (true);
    return void_class;
  }

  static rObject
  system_class_quit (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    r.lobby_get()->value_get().connection.close();
    return void_class;
  }

  static rObject
  system_class_spawn(runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT_RANGE (2, 3);
    rObject arg1 = args[1]->as<Code>();
    assert(arg1);

    runner::Interpreter* new_runner =
      new runner::Interpreter (dynamic_cast<runner::Interpreter&>(r),
			       rObject(arg1));
    new_runner->copy_tags (r);
    new_runner->time_shift_set (r.time_shift_get ());

    if (args.size () == 3)
    {
      if (is_true (args[2], SYMBOL(spawn)))
	r.link (new_runner);
    }

    new_runner->start_job ();

    return object::void_class;
  }

  static rObject
  system_class_stats(runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    Dictionary::value_type res;
    const scheduler::scheduler_stats_type& stats =
      r.scheduler_get().stats_get();
    // The space after "Symbol(" is mandatory to avoid triggering an error in
    // symbol generation code
#define ADDSTAT(Suffix, Function, Divisor)				\
    res[libport::Symbol( "cycles" # Suffix)] = new Float(stats.Function() / Divisor)
    ADDSTAT(, samples, 1);
    ADDSTAT(Max, max, 1000.0);
    ADDSTAT(Mean, mean, 1000.0);
    ADDSTAT(Min, min, 1000.0);
    ADDSTAT(StdDev, standard_deviation, 1000.0);
    ADDSTAT(Variance, variance, 1000.0);
#undef ADDSTAT
    return new Dictionary(res);
  }

  // This should give a backtrace as an urbi object.
  static rObject
  system_class_backtrace(runner::Runner& r, objects_type args)
  {
    // FIXME: This method sucks a bit, because show_backtrace sucks a
    // bit, because our channeling/message-sending system sucks a lot.
    CHECK_ARG_COUNT (1);
    runner::Runner::backtrace_type bt = r.backtrace_get();
    bt.pop_back();
    foreach (const runner::Runner::frame_type& elt,
	     boost::make_iterator_range(boost::rbegin(bt),
					boost::rend(bt)))
      r.send_message("backtrace", elt.first + " (" + elt.second + ")\n");
    return void_class;
  }

  static rObject
  system_class_jobs(runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    List::value_type res;
    foreach(scheduler::rJob job, r.scheduler_get().jobs_get())
      res.push_back(new Task(job));
    return new List(res);
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
    CHECK_ARG_COUNT (1);						\
    ::urbiserver->Variable = Value;					\
    return void_class;							\
  }

  SERVER_SET_VAR(debugoff, debugOutput, false)
  SERVER_SET_VAR(debugon, debugOutput, true)
  SERVER_SET_VAR(stopall, stopall, true)

#undef SERVER_SET_VAR


  void
  system_class_initialize ()
  {
    /// \a Call gives the name of the C++ function, and \a Name that in Urbi.
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(system, Name)

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
    DECLARE(shiftedTime);
    DECLARE(stats);
    DECLARE(shutdown);
    DECLARE(sleep);
    DECLARE(spawn);
    DECLARE(stopall);
    DECLARE(time);
#undef DECLARE
  }

}; // namespace object
