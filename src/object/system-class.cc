/**
 ** \file object/system-class.cc
 ** \brief Creation of the URBI object system.
 */

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include "parser/uparser.hh"
#include "kernel/userver.hh"
#include "kernel/uconnection.hh"

#include "runner/runner.hh"

#include "object/alien.hh"
#include "object/system-class.hh"

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
    FETCH_ARG(1, Float);
    libport::utime_t deadline =
      ::urbiserver->getTime() +
      static_cast<libport::utime_t>(arg1->value_get() * 1000.0);
    r.yield_until (deadline);
    return void_class;
  }

  static rObject
  system_class_time (runner::Runner&, objects_type)
  {
    return Float::fresh(::urbiserver->getTime() / 1000.0);
  }

  static rObject
  execute_parsed (runner::Runner& r, parser::UParser& p, UrbiException e)
  {
    ast::Nary errs;
    p.process_errors(&errs);
    errs.accept(r);

    if (ast::Nary* ast = p.ast_get())
    {
      ast->accept(r);
      //FIXME: deleting the tree now causes segv.
      //delete p.command_tree_get();
      //p.command_tree_set (0);
      return r.current_get();
    }
    else
      throw e;
  }

  static rObject
  system_class_assert_(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(3);
    FETCH_ARG(2, String);
    if (!IS_TRUE(args[1]))
      throw PrimitiveError
	("assert_",
	 "assertion `" + arg2->value_get().name_get() + "' failed");
    return void_class;
  }

  static rObject
  system_class_eval (runner::Runner& r, objects_type args)
  {
    FETCH_ARG(1, String);
    parser::UParser p = parser::parse(arg1->value_get());
    return execute_parsed(r, p, PrimitiveError("",
	std::string("Error executing command.")));
  }

  static rObject
  system_class_searchFile (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG(1, String);

    UServer& s = r.lobby_get()->value_get().connection.server_get();
    try
    {
      return String::fresh(libport::Symbol(
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
    FETCH_ARG(1, String);

    std::string filename = arg1->value_get().name_get();

    if (!libport::path (filename).exists ())
      throw PrimitiveError("loadFile",
			   "No such file: " + filename);

    parser::UParser p = parser::parse_file(filename);
    return
      execute_parsed(r, p,
		     PrimitiveError("", //same message than k1
				    "Error loading file: " + filename));
  }

  static rObject
  system_class_currentRunner (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    rObject res = object::Object::fresh();
    res->proto_add (task_class);
    res->slot_set (SYMBOL (runner), box (scheduler::rJob, r.myself_get ()));
    return res;
  }

  static rObject
  system_class_cycle (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return Float::fresh (r.scheduler_get ().cycle_get ());
  }

  static rObject
  system_class_fresh (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return String::fresh(libport::Symbol::fresh());
  }

  static rObject
  system_class_lobby (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return r.lobby_get();
  }

  static rObject
  system_class_quit (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    r.lobby_get()->value_get().connection.close();
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
    DECLARE(currentRunner);
    DECLARE(cycle);
    DECLARE(debugoff);
    DECLARE(debugon);
    DECLARE(eval);
    DECLARE(fresh);
    DECLARE(loadFile);
    DECLARE(lobby);
    DECLARE(quit);
    DECLARE(reboot);
    DECLARE(searchFile);
    DECLARE(shutdown);
    DECLARE(sleep);
    DECLARE(stopall);
    DECLARE(time);
#undef DECLARE
  }

}; // namespace object
