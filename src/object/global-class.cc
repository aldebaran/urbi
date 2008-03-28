/**
 ** \file object/global-class.cc
 ** \brief Creation of the URBI object global.
 */

#include "object/global-class.hh"
#include "object/object.hh"
#include "object/object-class.hh"
#include "object/scope-class.hh"

namespace object
{
  rObject global_class;

  /*---------------------.
  | Global primitives.  |
  `---------------------*/

  void
  global_class_initialize ()
  {
    // Global is a scope. We thus remove useless inheritance from object.
    global_class->proto_remove(object_class);
    global_class->proto_add(scope_class);

    // Do not report children to be instance of 'Global', but instances of scope.
    global_class->slot_remove(SYMBOL(protoName));

#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(global, Name)

#undef DECLARE
  }

}; // namespace object
