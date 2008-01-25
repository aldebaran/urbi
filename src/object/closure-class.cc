/**
 ** \file object/closure-class.cc
 ** \brief Creation of the URBI object closure.
 */

#include "object/closure-class.hh"

#include "object/atom.hh"
#include "object/object.hh"

#include "runner/runner.hh"

namespace object
{
  rObject closure_class;

  /*------------------.
  | Closure primitives.  |
  `------------------*/

  static rObject
  closure_class_create (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG (1, Code);

    rObject new_closure = clone (closure_class);
    new_closure->slot_set ("func", args[1]);
    new_closure->slot_set ("context", r.locals_get ());

    return new_closure;
  }



  void
  closure_class_initialize ()
  {
#define DECLARE(Name)							\
    closure_class->slot_set (#Name,					\
			  new Primitive(closure_class_ ## Name));
    DECLARE (create);
#undef DECLARE
  }

}; // namespace object
