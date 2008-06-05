/**
 ** \file object/primitive-class.cc
 ** \brief Creation of the URBI object primitive.
 */

#include <object/primitive-class.hh>
#include <object/atom.hh>
#include <object/object.hh>

#include <runner/runner.hh>

namespace object
{
  rObject primitive_class;

  /*-----------------------.
  | Primitive primitives.  |
  `-----------------------*/

  static rObject
  primitive_class_apply (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG (1, List);
    if (arg1->value_get().empty())
      throw PrimitiveError ("apply", "list of arguments must begin with self");
    return r.apply (args[0], SYMBOL(apply), arg1);
  }

  void
  primitive_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(primitive, Name)
    DECLARE (apply);
#undef DECLARE
  }

}; // namespace object
