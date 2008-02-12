/**
 ** \file object/object-class.cc
 ** \brief Creation of the URBI object object.
 */

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>
#include <libport/tokenizer.hh>
#include <libport/ufloat.hh>
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

  static rObject
  object_class_sameAs(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    return args[0]->slot_get((args[0] == args[1]) ? SYMBOL(true): SYMBOL(false));
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
    r.yield_until (::urbiserver->getTime() + arg1->value_get() * 1000.0);
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

    if (!p.command_tree_get())
      throw e;
    else
      p.command_tree_get()->accept(r);
    //FIXME: deleting the tree now causes segv.
    //delete p.command_tree_get();
    //p.command_tree_set (0);
    return r.current_get();
  }

  static rObject
  object_class_eval (runner::Runner& r, objects_type args)
  {
    FETCH_ARG(1, String);
    UConnection& c = r.lobby_get()->value_get().connection;
    UParser p(c);
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
    UParser p(c);
    p.process_file(c.server_get().find_file(libport::path(arg1->value_get())));
    return
      execute_parsed(r, p,
		     PrimitiveError("", //same message than k1
				    std::string("Error loading file: ")
				    + arg1->value_get().name_get()));
  }

  static rObject
  object_class_apply (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG (1, List);
    if (!arg1->value_get ().empty ())
      throw PrimitiveError ("apply", "first argument must be an empty list");
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
    if (o)
      return o;
    else
    return nil_class;
  }

  /// Define setSlot or updateSlot.  \a Verb is "set" or "update".
#define SLOT_CHANGE(Verb)						\
  static rObject							\
  object_class_ ## Verb ## Slot (runner::Runner&, objects_type args)	\
  {									\
    CHECK_ARG_COUNT(3);							\
    FETCH_ARG(1, String);						\
    args[0]->slot_ ## Verb (arg1->value_get(), args[2]);		\
    return args[2];							\
  }

  /// Set a slot.
  SLOT_CHANGE(set)
  /// Update a slot.
  SLOT_CHANGE(update)
#undef SLOT_CHANGE

  static rObject
  object_class_makeScope (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG (1, Float);
    try {
      int is_local = libport::ufloat_to_boolean (arg1->value_get ());
      args[0]->locals_set (is_local);
    }
    catch (boost::numeric::bad_numeric_cast& ue)
    {
      throw BadInteger (arg1->value_get (), "makeScope");
    }
    return args[0];
  }

  void
  object_class_initialize ()
  {
    /// \a Call gives the name of the C++ function, and \a Name that in Urbi.
#define DECLARE(Name)							\
    DECLARE_PRIMITIVE(object, Name, Name)
    DECLARE(clone);
    DECLARE(init);

    DECLARE(protos);
    DECLARE(addProto);
    DECLARE(removeProto);

    DECLARE(slotNames);
    DECLARE(getSlot);
    DECLARE(removeSlot);
    DECLARE(setSlot);
    DECLARE(updateSlot);
    DECLARE(locateSlot);
    DECLARE(makeScope);

    DECLARE(apply);

    DECLARE(dump);
    DECLARE(echo);
    DECLARE(eval);
    DECLARE(fresh);
    DECLARE(load);
    DECLARE(lobby);
    DECLARE(print);
    DECLARE(reboot);
    DECLARE(sameAs);
    DECLARE(shutdown);
    DECLARE(sleep);
    DECLARE(time);
#undef DECLARE
  }

}; // namespace object
