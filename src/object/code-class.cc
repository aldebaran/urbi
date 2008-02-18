/**
 ** \file object/code-class.cc
 ** \brief Creation of the URBI object code.
 */

#include <libport/foreach.hh>

#include "object/code-class.hh"

#include "object/atom.hh"
#include "object/object.hh"

#include "runner/runner.hh"

namespace object
{
  rObject code_class;

  /*------------------.
  | Code primitives.  |
  `------------------*/

  static rObject
  code_class_apply (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG (1, List);

    objects_type apply_args;
    apply_args.push_back (args[0]);
    foreach (rObject arg, arg1->value_get ())
      apply_args.push_back (arg);

    return r.apply (args[0], apply_args);
  }



  void
  code_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(code, Name)
    DECLARE (apply);
#undef DECLARE
  }

}; // namespace object
