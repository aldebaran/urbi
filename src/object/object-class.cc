/**
 ** \file object/object-class.cc
 ** \brief Creation of the URBI object object.
 */

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
  object_class_clone (rContext, objects_type args)
  {
    return clone(args[0]);
  }

  rObject
  object_class_init (rContext, objects_type args)
  {
    return args[0];
  }

  /// Send pretty-printed self on the connection.
  rObject
  object_class_print (rContext c, objects_type args)
  {
    std::ostringstream os;
    args[0]->print (os);
    c->value_get().connection.send (os.str().c_str());
    c->value_get().connection.endline();
    return args[0];
  }

  /// Send pretty-printed args[1] to the connection.
  rObject
  object_class_echo (rContext c, objects_type args)
  {
    // First argument (self) is ignored, print args[1].
    object_class_print(c, objects_type(args.begin()+1, args.end()));
    return args[0];
  }

#define SERVER_FUNCTION(Function)				\
  rObject							\
  object_class_ ## Function (rContext, objects_type args)	\
  {								\
    ::urbiserver->Function();					\
    /* Return the current object to return something. */	\
    return args[0];						\
  }

  SERVER_FUNCTION(reboot)
  SERVER_FUNCTION(shutdown)

#undef SERVER_FUNCTION


  rObject
  object_class_wait (rContext, objects_type args)
  {
    // FIXME: Currently does nothing.  A stub so that we
    // accept "wait 2s" as is used in the test suite.
    return args[0];
  }

  /// Get slots' list.
  static rObject
  object_class_slotNames (rContext, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    rObject obj = args[0];

    object::list_traits::type l;
    BOOST_FOREACH (const Object::slot_type& p, obj->slots_get())
      l.push_back (new object::String (p.first.name_get ()));

    return new object::List (l);
  }

  /// Get parents' list.
  static rObject
  object_class_parents (rContext, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    rObject obj = args[0];

    object::list_traits::type l;
    BOOST_FOREACH (const Object::parent_type& o, obj->parents_get())
      l.push_back(o);

    return new object::List(l);
  }

  /// Get a slot.
  static rObject
  object_class_getSlot (rContext, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    rObject obj = args[0];
    FETCH_ARG(1, String);

    return (*obj)[arg1->value_get()];
  }

  /// Set a slot.
  static rObject
  object_class_setSlot (rContext, objects_type args)
  {
    CHECK_ARG_COUNT(3);
    rObject obj = args[0];
    FETCH_ARG(1, String);
    rObject val = args[2];

    (*obj)[arg1->value_get()] = val;
    return obj;
  }

  void
  object_class_initialize ()
  {
#define DECLARE(Name)							\
      object_class->slot_set (#Name,					\
			      new Primitive(object_class_ ## Name));
      DECLARE(clone);
      DECLARE(init);

      DECLARE(slotNames);
      DECLARE(parents);
      DECLARE(getSlot);
      DECLARE(setSlot);

      DECLARE(echo);
      DECLARE(print);
      DECLARE(reboot);
      DECLARE(shutdown);
      DECLARE(wait);
#undef DECLARE
  }

}; // namespace object
