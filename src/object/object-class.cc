/**
 ** \file object/object-class.cc
 ** \brief Creation of the URBI object object.
 */

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>
#include <libport/throw-exception.hh>
#include <libport/tokenizer.hh>
#include <boost/lexical_cast.hpp>

#include "parser/uparser.hh"
#include "kernel/userver.hh"
#include "kernel/uconnection.hh"

#include "runner/runner.hh"

#include "object/atom.hh"
#include "object/object-class.hh"
#include "object/object.hh"

namespace object
{
  rObject object_class;

  /*--------------------.
  | Object primitives.  |
  `--------------------*/

  static rObject
  object_class_clone (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return clone(args[0]);
  }

  static rObject
  object_class_init (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return args[0];
  }

  /// Send dumped self on the connection.
  /// args[1], if present, can be the tag to use.
  static rObject
  object_class_dump (runner::Runner& r, objects_type args)
  {
    // Second argument is the tag name.
    std::string tag;
    if (args.size() == 2)
    {
      FETCH_ARG(1, String);
      tag = arg1->value_get().name_get();
    }
    std::ostringstream os;
    args[0]->dump(os);
    //for now our best choice is to dump line by line in "system" messages.
    const std::string stream = os.str();
    boost::tokenizer< boost::char_separator<char> > tok =
      libport::make_tokenizer(stream, "\n");
    std::string system_header("*** ");
    BOOST_FOREACH(std::string line, tok)
      r.lobby_get()->value_get().connection  << UConnection::send (
	(system_header+line+"\n").c_str(), tag.c_str());
    return void_class;
  }

  static rObject
  object_echo(runner::Runner& r, objects_type args, bool is_echo)
  {
    // Second argument is the tag name.
    std::string tag;
    if (args.size() == 3)
    {
      FETCH_ARG(2, String);
      tag = arg2->value_get().name_get();
    }
    //special case for Strings
    if (is_echo && args[1]->kind_is(Object::kind_string))
    {
      r.lobby_get()->value_get().connection
      << UConnection::send((std::string("*** ")
			   + std::string(VALUE(args[1], String))
			   + "\n").c_str(),
			   tag.c_str());
    }
    else
      r.lobby_get()->value_get().connection.send (args[1], tag.c_str(),
	is_echo?"*** ":"");
    return void_class;
  }

  /// Send pretty-printed args[1] to the connection.
  static rObject
  object_class_echo (runner::Runner& r, objects_type args)
  {
    return object_echo(r, args, true);
  }

  /// Send pretty-printed self on the connection.
  /// args[1], if present, can be the tag to use.
  static rObject
  object_class_print (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT_RANGE (1, 2);
    objects_type nargs;
    nargs.push_back(args[0]);
    nargs.insert(nargs.end(),  args.begin(), args.end());
    return object_echo(r, nargs, false);
  }

  /// Send pretty-printed self on the connection.
  /// args[1], if present, can be the tag to use.
  static rObject
  object_class_asString (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    std::ostringstream os;
    args[0]->print(os);
    return new String(libport::Symbol(os.str()));
  }

  /// Return the address of an object as a number, mostly
  /// for debugging purpose.

  static rObject
  object_class_uid (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    std::string str = (boost::format ("0x%x") % reinterpret_cast<long long> (args[0].get ())).str ();
    return new String (libport::Symbol (str));
  }

  static rObject
  object_class_sameAs(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    return args[0]->slot_get((args[0] == args[1])
			     ? SYMBOL(true)
			     : SYMBOL(false));
  }


#define SERVER_FUNCTION(Function)					\
  static rObject							\
  object_class_ ## Function (runner::Runner&, objects_type args)	\
  {									\
    CHECK_ARG_COUNT (1);						\
    ::urbiserver->Function();						\
    return void_class;							\
  }

  SERVER_FUNCTION(reboot)
  SERVER_FUNCTION(shutdown)

#undef SERVER_FUNCTION


  static rObject
  object_class_sleep (runner::Runner& r, objects_type args)
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
  object_class_time (runner::Runner&, objects_type)
  {
    return new Float(::urbiserver->getTime() / 1000.0);
  }

  static rObject
  execute_parsed (runner::Runner& r, UParser& p, UrbiException e)
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
      libport::throw_exception (e);
  }

  static rObject
  object_class_eval (runner::Runner& r, objects_type args)
  {
    FETCH_ARG(1, String);
    UParser p;
    p.process(arg1->value_get());
    return execute_parsed(r, p, PrimitiveError("",
	std::string("Error executing command.")));
  }

  static rObject
  object_class_load (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG(1, String);
    UConnection& c = r.lobby_get()->value_get().connection;
    UParser p;
    try
    {
      libport::path path =
	c.server_get().find_file(libport::path(arg1->value_get()));
      p.process_file(path);
      return
	execute_parsed(r, p,
		       PrimitiveError("", //same message than k1
				      "Error loading file: "
				      + arg1->value_get().name_get()));
    }
    catch (libport::file_library::Not_found&)
    {
      boost::throw_exception(
	PrimitiveError("",
		       "Unable to find file: "
		       + arg1->value_get().name_get()));
      // Never reached
      assertion(false);
      return 0;
    }
  }

  static rObject
  object_class_apply (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG (1, List);
    if (!arg1->value_get ().empty ())
      boost::throw_exception
	(PrimitiveError ("apply", "first argument must be an empty list"));
    return args[0];
  }

  static rObject
  object_class_fresh (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return new String(libport::Symbol::fresh());
  }

  static rObject
  object_class_lobby (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return r.lobby_get();
  }

  static rObject
  object_class_quit (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    r.lobby_get()->value_get().connection.closeConnection();
    return void_class;
  }

#define SERVER_SET_VAR(Function, Variable, Value)					\
  static rObject							\
  object_class_ ## Function (runner::Runner&, objects_type args)	\
  {									\
    CHECK_ARG_COUNT (1);						\
    ::urbiserver->Variable = Value;					\
    return void_class;							\
  }

  SERVER_SET_VAR(debugoff, debugOutput, false)
  SERVER_SET_VAR(debugon, debugOutput, true)
  SERVER_SET_VAR(stopall, stopall, true)

#undef SERVER_SET_VAR

  /*---------.
  | Protos.  |
  `---------*/

  /// Adding or removing protos. \a Verb is "add" or "remove".
#define CHANGE_PARENTS(Verb)						\
  static rObject							\
  object_class_ ## Verb ## Proto (runner::Runner&, objects_type args)	\
  {									\
    CHECK_ARG_COUNT(2);							\
    args[0]->proto_ ## Verb (args[1]);					\
    return args[0];							\
  }

  /// Add a proto.
  CHANGE_PARENTS(add);
  /// Remove a proto.
  CHANGE_PARENTS(remove);
#undef CHANGE_PARENTS

  /// Get protos' list.
  static rObject
  object_class_protos (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    rObject obj = args[0];

    object::list_traits::type l;
    BOOST_FOREACH (const rObject o, obj->protos_get())
      l.push_back(o);

    return new object::List(l);
  }


  /*--------.
  | Slots.  |
  `--------*/


  /// List of slot names.
  static rObject
  object_class_slotNames (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    rObject obj = args[0];

    object::list_traits::type l;
    BOOST_FOREACH (const Object::slot_type& p, obj->slots_get())
      l.push_back (new object::String (p.first));

    return new object::List (l);
  }

  /// Get a slot content.
  static rObject
  object_class_getSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    rObject obj = args[0];
    FETCH_ARG(1, String);
    return obj->slot_get(arg1->value_get());
  }

  static rObject
  object_class_getLazyLocalSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (4);

    FETCH_ARG (1, String);
    Object::key_type slot_name = arg1->value_get ();

    // If the slot already exists, return its content.
    rObject slot = args[0]->own_slot_get (slot_name, 0);
    if (slot)
      return slot;

    // The slot doesn't exist. Should we create it?
    if (IS_TRUE (args[3]))
      args[0]->slot_set (slot_name, args[2]);

    // Return the default value for this slot.
    return args[2];
  }

  /// Remove a slot.
  static rObject
  object_class_removeSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    rObject obj = args[0];
    FETCH_ARG(1, String);
    obj->slot_remove(arg1->value_get());
    return obj;
  }

   /// Locate a slot.
  static rObject
  object_class_locateSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    FETCH_ARG(1, String);

    rObject o = slot_locate(args[0], arg1->value_get());
    return o ? o : nil_class;
  }

  static rObject
  object_class_setSlot (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (3);
    FETCH_ARG (1, String);
    args[0]->slot_set (arg1->value_get (), args[2]);
    return args[2];
  }

  static rObject
  object_class_updateSlot (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (3);
    FETCH_ARG (1, String);
    slot_update (r, args[0], arg1->value_get (), args[2]);
    return args[2];
  }

  static rObject
  object_class_makeScope (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    args[0]->locals_set (IS_TRUE (args[1]));
    return args[0];
  }

  static rObject
  object_class_isScope (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return new Float (!!args[0]->locals_get ());
  }

  void
  object_class_initialize ()
  {
    /// \a Call gives the name of the C++ function, and \a Name that in Urbi.
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(object, Name)
    DECLARE(clone);
    DECLARE(init);

    DECLARE(protos);
    DECLARE(addProto);
    DECLARE(removeProto);

    DECLARE(slotNames);
    DECLARE(getLazyLocalSlot);
    DECLARE(getSlot);
    DECLARE(removeSlot);
    DECLARE(setSlot);
    DECLARE(updateSlot);
    DECLARE(locateSlot);
    DECLARE(makeScope);
    DECLARE(isScope);

    DECLARE(apply);

    DECLARE(asString);
    DECLARE(debugoff);
    DECLARE(debugon);
    DECLARE(dump);
    DECLARE(echo);
    DECLARE(eval);
    DECLARE(fresh);
    DECLARE(load);
    DECLARE(lobby);
    DECLARE(print);
    DECLARE(quit);
    DECLARE(reboot);
    DECLARE(sameAs);
    DECLARE(shutdown);
    DECLARE(sleep);
    DECLARE(stopall);
    DECLARE(time);
    DECLARE(uid);
#undef DECLARE
  }

}; // namespace object
