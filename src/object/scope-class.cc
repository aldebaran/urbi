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

namespace object
{
  rObject scope_class;


  static rObject
  scope_class_target(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    FETCH_ARG(1, String);

    return target(args[0], arg1->value_get());
  }

  void
  scope_class_initialize ()
  {
#define DECLARE(Name)                   \
    DECLARE_PRIMITIVE(scope, Name)
    DECLARE(target);
#undef DECLARE
  }

} // namespace object

