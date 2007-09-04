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
  object_class_clone (rContext, objects_type args)
  {
    return clone(args[0]);
  }

  rObject
  object_class_init (rContext, objects_type args)
  {
    return args[0];
  }

  rObject
  object_class_print (rContext, objects_type args)
  {
    std::cout << *args[0] << std::endl;
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


  void
  object_class_initialize ()
  {
#define DECLARE(Name)							\
      object_class->slot_set (#Name,					\
			      new Primitive(object_class_ ## Name));
      DECLARE(clone);
      DECLARE(init);
      DECLARE(print);
      DECLARE(reboot);
      DECLARE(shutdown);
      DECLARE(wait);
#undef DECLARE
  }

}; // namespace object
