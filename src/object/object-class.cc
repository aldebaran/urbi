/**
 ** \file object/object-class.cc
 ** \brief Creation of the URBI object object.
 */

//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"
#include "libport/tokenizer.hh"
#include <boost/lexical_cast.hpp>

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
    return clone(args[0]);
  }

  static rObject
  object_class_init (runner::Runner&, objects_type args)
  {
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
  object_echo(runner::Runner& r, objects_type args, const char * prefix)
  {
    // Second argument is the tag name.
    std::string tag;
    if (args.size() == 3)
    {
      FETCH_ARG(2, String);
      tag = arg2->value_get().name_get();
    }
    r.lobby_get()->value_get().connection.send (args[1], tag.c_str(), prefix);
    return void_class;
  }
  /// Send pretty-printed args[1] to the connection.
  // FIXME: Lots of duplication with the previous primitive :(
  static rObject
  object_class_echo (runner::Runner& r, objects_type args)
  {
    return object_echo(r, args, "*** ");
  }

  /// Send pretty-printed self on the connection.
  /// args[1], if present, can be the tag to use.
  static rObject
  object_class_print (runner::Runner& r, objects_type args)
  {
    objects_type nargs;
    nargs.push_back(args[0]);
    nargs.insert(nargs.end(),  args.begin(), args.end());
    return object_echo(r, nargs, "");
  }

#define SERVER_FUNCTION(Function)				\
  static rObject						\
  object_class_ ## Function (runner::Runner&, objects_type)	\
  {								\
    ::urbiserver->Function();					\
    /* Return the current object to return something. */	\
    return void_class;						\
  }

  SERVER_FUNCTION(reboot)
  SERVER_FUNCTION(shutdown)

#undef SERVER_FUNCTION


  static rObject
  object_class_wait (runner::Runner&, objects_type)
  {
    // FIXME: Currently does nothing.  A stub so that we
    // accept "wait 2s" as is used in the test suite.
    return void_class;
  }

  static rObject
  object_class_load (runner::Runner& r, objects_type args)
  {
    FETCH_ARG(1, String);
    if (::urbiserver->loadFile(arg1->value_get(),
      &r.lobby_get()->value_get().connection.recvQueue()) != USUCCESS)
    throw PrimitiveError("load",
      std::string("Error loading file:") + arg1->value_get().name_get());
    return void_class;
  }


  /*----------.
  | Protos.  |
  `----------*/

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
      l.push_back (new object::String (p.first.name_get ()));

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

  void
  object_class_initialize ()
  {
    /// \a Call gives the name of the C++ function, and \a Name that in Urbi.
#define DECLARE2(Call, Name)						\
    object_class->slot_set (#Name,					\
			    new Primitive(object_class_ ## Call))
#define DECLARE1(Name)				\
    DECLARE2(Name, Name)

    DECLARE1(clone);
    DECLARE1(init);

    DECLARE1(protos);
    DECLARE1(addProto);
    DECLARE1(removeProto);

    DECLARE1(slotNames);
    DECLARE1(getSlot);
    DECLARE1(removeSlot);
    DECLARE1(setSlot);
    DECLARE1(updateSlot);

    DECLARE1(echo);
    DECLARE1(dump);
    DECLARE1(print);
    DECLARE1(reboot);
    DECLARE1(shutdown);
    DECLARE1(wait);
    DECLARE1(load);
#undef DECLARE1
  }

}; // namespace object
