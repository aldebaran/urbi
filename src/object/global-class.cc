/**
 ** \file object/global-class.cc
 ** \brief Creation of the URBI object global.
 */

#include <object/global-class.hh>
#include <object/object.hh>
#include <object/object-class.hh>

namespace object
{
  rObject global_class;

  /*--------------------.
  | Global primitives.  |
  `--------------------*/

  void
  global_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(global, Name)

#undef DECLARE
  }

}; // namespace object
