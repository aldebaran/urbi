/**
 ** \file object/globals-class.cc
 ** \brief Creation of the URBI object globals.
 */

#include "object/globals-class.hh"
#include "object/object.hh"

namespace object
{
  rObject globals_class;

  /*---------------------.
  | Globals primitives.  |
  `---------------------*/

  void
  globals_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(globals, Name)

#undef DECLARE
  }

}; // namespace object
