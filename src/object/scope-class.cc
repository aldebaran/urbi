/**
 ** \file object/scope-class.cc
 ** \brief Creation of the URBI scope object.
 */

#include <boost/bind.hpp>
#include <libport/foreach.hh>

#include "object/scope-class.hh"
#include "object/atom.hh"
#include "object/object.hh"
#include "primitives.hh"
#include "runner/runner.hh"

namespace object
{
  rObject scope_class;

  static rObject
  scope_class_locals(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);

    return args[0];
  }

  void
  scope_class_initialize ()
  {
#define DECLARE(Name)                   \
    DECLARE_PRIMITIVE(scope, Name)
    DECLARE(locals);
#undef DECLARE
  }

} // namespace object

