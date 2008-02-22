/**
 ** \file object/delegate-class.cc
 ** \brief Creation of the URBI object delegate.
 */

#include "object/delegate-class.hh"
#include "object/object.hh"
#include "object/primitives.hh"

#include "runner/runner.hh"

namespace object
{
  rObject delegate_class;

  /*-----------------------.
  | Primitive primitives.  |
  `-----------------------*/

  static rObject
  delegate_class_apply (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG (1, List);
    return r.apply (args[0], arg1);
  }

  void
  delegate_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(delegate, Name)
    DECLARE (apply);
#undef DECLARE
  }

} // namespace object
