/**
 ** \file object/object-class.cc
 ** \brief Creation of the URBI object object.
 */

#include "kernel/userver.hh"

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
  object_class_clone (objects_type args)
  {
    return clone(args[0]);
  }

  rObject
  object_class_init (objects_type args)
  {
    return args[0];
  }

  rObject
  object_class_print (objects_type args)
  {
    std::cout << *args[0] << std::endl;
    return args[0];
  }

#define SERVER_FUNCTION(Function)				\
  rObject							\
  object_class_ ## Function (objects_type args)			\
  {								\
    ::urbiserver->Function();					\
    /* Return the current object to return something. */	\
    return args[0];						\
  }

  SERVER_FUNCTION(reboot)
  SERVER_FUNCTION(shutdown)

#undef SERVER_FUNCTION


  rObject
  object_class_wait (objects_type args)
  {
    // FIXME: Currently does nothing.  A stub so that we
    // accept "wait 2s" as is used in the test suite.
    return args[0];
  }

  /// Get slots' list.
  static rObject
  object_class_slotNames (objects_type args)
  {
    rObject obj = args[0];

    object::list_traits::type l;
    BOOST_FOREACH (const Object::slot_type& p, obj->slots_get())
      l.push_back(p.second);

    return new object::List(l);
  }

  /// Get parents' list.
  static rObject
  object_class_parents (objects_type args)
  {
    rObject obj = args[0];

    object::list_traits::type l;
    BOOST_FOREACH (const Object::parent_type& o, obj->parents_get())
      l.push_back(o);

    return new object::List(l);
  }

  /// Get a slot.
  static rObject
  object_class_getSlot (objects_type args)
  {
    rObject obj = args[0];
    FETCH_ARG(1, String);

    return (*obj)[libport::Symbol(arg1->value_get())];
  }

  /// Set a slot.
  static rObject
  object_class_setSlot (objects_type args)
  {
    rObject obj = args[0];
    FETCH_ARG(1, String);
    rObject val = args[2];

    (*obj)[libport::Symbol(arg1->value_get())] = val;
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

      DECLARE(print);
      DECLARE(reboot);
      DECLARE(shutdown);
      DECLARE(wait);
#undef DECLARE
  }

}; // namespace object
