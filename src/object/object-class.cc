/**
 ** \file object/object-class.cc
 ** \brief Creation of the URBI object object.
 */

//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include <boost/lexical_cast.hpp>

#include "kernel/userver.hh"
#include "kernel/uconnection.hh"

#include "object/atom.hh"
#include "object/object-class.hh"
#include "object/object.hh"

namespace object
{
  rObject object_class;

  /*--------------------.
  | Object primitives.  |
  `--------------------*/

  rObject
  object_class_clone (rLobby, objects_type args)
  {
    return clone(args[0]);
  }

  rObject
  object_class_init (rLobby, objects_type args)
  {
    return args[0];
  }


  /// Send pretty-printed self on the connection.
  /// args[1], if present, can be the tag to use.
  rObject
  object_class_print (rLobby c, objects_type args)
  {
    // Second argument is the tag name.
    const char* tag = 0;
    if (args.size() == 2)
    {
      FETCH_ARG(1, String);
      tag = arg1->value_get().name_get().c_str();
    }

    c->value_get().connection.send (args[0], tag);
    return args[0];
  }

  /// Send pretty-printed args[1] to the connection.
  // FIXME: Lots of duplication with the previous primitive :(
  rObject
  object_class_echo (rLobby c, objects_type args)
  {
    // Second argument is the tag name.
    const char* tag = 0;
    if (args.size() == 3)
    {
      FETCH_ARG(2, String);
      tag = arg2->value_get().name_get().c_str();
    }

    c->value_get().connection.send (args[1], tag, "*** ");
    return args[0];
  }

#define SERVER_FUNCTION(Function)				\
  rObject							\
  object_class_ ## Function (rLobby, objects_type args)	\
  {								\
    ::urbiserver->Function();					\
    /* Return the current object to return something. */	\
    return args[0];						\
  }

  SERVER_FUNCTION(reboot)
  SERVER_FUNCTION(shutdown)

#undef SERVER_FUNCTION


  rObject
  object_class_wait (rLobby, objects_type args)
  {
    // FIXME: Currently does nothing.  A stub so that we
    // accept "wait 2s" as is used in the test suite.
    return args[0];
  }



  /*----------.
  | Parents.  |
  `----------*/

  /// Adding or removing parents. \a Verb is "add" or "remove".
#define CHANGE_PARENTS(Verb)					\
  static rObject						\
  object_class_ ## Verb ## Parent (rLobby, objects_type args)	\
  {								\
    CHECK_ARG_COUNT(2);						\
    args[0]->parent_ ## Verb (args[1]);				\
    return args[0];						\
  }

  /// Add a parent.
  CHANGE_PARENTS(add);
  /// Remove a parent.
  CHANGE_PARENTS(remove);
#undef CHANGE_PARENTS

  /// Get parents' list.
  static rObject
  object_class_parents (rLobby, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    rObject obj = args[0];

    object::list_traits::type l;
    BOOST_FOREACH (const rObject o, obj->parents_get())
      l.push_back(o);

    return new object::List(l);
  }


  /*--------.
  | Slots.  |
  `--------*/


  /// List of slot names.
  static rObject
  object_class_slotNames (rLobby, objects_type args)
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
  object_class_getSlot (rLobby, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    rObject obj = args[0];
    FETCH_ARG(1, String);
    return obj->slot_get(arg1->value_get());
  }

  /// Remove a slot.
  static rObject
  object_class_removeSlot (rLobby, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    rObject obj = args[0];
    FETCH_ARG(1, String);
    obj->slot_remove(arg1->value_get());
    return obj;
  }

  /// Define setSlot or updateSlot.  \a Verb is "set" or "update".
#define SLOT_CHANGE(Verb)					\
  static rObject						\
  object_class_ ## Verb ## Slot (rLobby, objects_type args)	\
  {								\
    CHECK_ARG_COUNT(3);						\
    FETCH_ARG(1, String);					\
    args[0]->slot_ ## Verb (arg1->value_get(), args[2]);	\
    return args[2];						\
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

    DECLARE1(parents);
    DECLARE1(addParent);
    DECLARE1(removeParent);

    DECLARE1(slotNames);
    DECLARE1(getSlot);
    DECLARE1(removeSlot);
    DECLARE1(setSlot);
    DECLARE1(updateSlot);

    DECLARE1(echo);
    DECLARE1(print);
    DECLARE1(reboot);
    DECLARE1(shutdown);
    DECLARE1(wait);
#undef DECLARE1
  }

}; // namespace object
