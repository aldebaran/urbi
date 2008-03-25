/**
 ** \file object/scope-class.cc
 ** \brief Creation of the URBI scope object.
 */

#include <libport/foreach.hh>

#include "object/scope-class.hh"
#include "object/atom.hh"
#include "object/object.hh"
#include "primitives.hh"

namespace object
{
  rObject scope_class;

  void
  scope_class_initialize ()
  {
#define DECLARE(Name)                   \
    DECLARE_PRIMITIVE(scope, Name)

#undef DECLARE
  }

} // namespace object

