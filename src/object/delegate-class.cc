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

  void
  delegate_class_initialize ()
  {
#define DECLARE(Name)                           \
        DECLARE_PRIMITIVE(delegate, Name)
#undef DECLARE
  }

} // namespace object
