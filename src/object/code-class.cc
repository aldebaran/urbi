/**
 ** \file object/code-class.cc
 ** \brief Creation of the URBI object code.
 */

#include <libport/lexical-cast.hh>

#include "ast/function.hh"
#include "ast/print.hh"

#include "object/atom.hh"
#include "object/code-class.hh"
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
    if (arg1->value_get().empty())
      throw PrimitiveError ("apply", "list of arguments must begin with self");
    return r.apply (args[0], SYMBOL(apply), arg1);
  }

  static rObject
  code_class_asString(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    if (args[0] == code_class)
      return String::fresh(SYMBOL(LT_Code_GT));
    FETCH_ARG(0, Code);
    return String::fresh(libport::Symbol(string_cast(arg0->value_get())));
  }

  static rObject
  code_class_bodyString(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    if (args[0] == code_class)
      return String::fresh(SYMBOL(LT_Code_GT));
    FETCH_ARG(0, Code);
    return
      String::fresh(
        libport::Symbol(
          string_cast(arg0->value_get().body_get()->body_get())));
  }

  void
  code_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(code, Name)
    DECLARE (apply);
    DECLARE (asString);
    DECLARE (bodyString);
#undef DECLARE
  }

}; // namespace object
