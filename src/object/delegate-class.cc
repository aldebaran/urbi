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
  delegate_class_asString(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    static rObject delegateString = new String(SYMBOL(LT_delegate_GT));
    return delegateString;
  }

  void
  delegate_class_initialize ()
  {
#define DECLARE(Name)                           \
        DECLARE_PRIMITIVE(delegate, Name)
    DECLARE (asString);
#undef DECLARE
  }

} // namespace object
