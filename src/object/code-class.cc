/**
 ** \file object/code-class.cc
 ** \brief Creation of the URBI object code.
 */

#include "object/code-class.hh"

#include "object/atom.hh"
#include "object/object.hh"

namespace object
{
  rObject code_class;

  /*------------------.
  | Code primitives.  |
  `------------------*/


  void
  code_class_initialize ()
  {
#define DECLARE(Name)							\
    code_class->slot_set (#Name,					\
			  new Primitive(code_class_ ## Name));
#undef DECLARE
  }

}; // namespace object
