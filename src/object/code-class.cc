/**
 ** \file object/code-class.cc
 ** \brief Creation of the URBI object code.
 */

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
    return r.apply (args[0], arg1);
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
