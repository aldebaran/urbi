/**
 ** \file object/call-class.cc
 ** \brief Creation of the URBI object call.
 */

#include "object/call-class.hh"

#include "object/atom.hh"
#include "object/object.hh"

#include "runner/runner.hh"

namespace object
{
  rObject call_class;

  /*------------------.
  | Call primitives.  |
  `------------------*/




  void
  call_class_initialize ()
  {
#define DECLARE(Name)							\
    call_class->slot_set (#Name,					\
			  new Primitive(call_class_ ## Name));
#undef DECLARE
  }

}; // namespace object
