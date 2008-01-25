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
  closure_class_init (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    FETCH_ARG (1, Code);

    args[0]->slot_set ("func", args[1]);
    args[0]->slot_set ("context", r.locals_get ());

    return args[0];
  }



  void
  closure_class_initialize ()
  {
#define DECLARE(Name)							\
    closure_class->slot_set (#Name,					\
			  new Primitive(closure_class_ ## Name));
    DECLARE (init);
#undef DECLARE
  }

}; // namespace object
