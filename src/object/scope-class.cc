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
  scope_class_target(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    FETCH_ARG(1, String);

    return target(args[0], arg1->value_get());
  }

  static rObject
  scope_class_tryTarget(runner::Runner& runner, objects_type args)
  {
    try
    {
      return scope_class_target(runner, args);
    }
    catch (LookupError&)
    {
      return nil_class;
    }
  }

#define FORWARD(Name, Argc)						\
  static rObject							\
  scope_class_ ## Name (runner::Runner& runner, objects_type args)	\
  {									\
    CHECK_ARG_COUNT(Argc);						\
    FETCH_ARG(1, String);						\
									\
    rObject fwd = object_class->slot_get(SYMBOL(Name));			\
    args[0] = target(args[0], arg1->value_get());			\
    return runner.apply(fwd, args);					\
  }

  FORWARD(getSlot, 2);
  FORWARD(removeSlot, 2);
  FORWARD(updateSlot, 3);

#undef FORWARD

  static rObject
  scope_class_locateSlot (runner::Runner& runner, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    FETCH_ARG(1, String);

    args[0] = scope_class_tryTarget(runner, args);
    if (args[0] == nil_class)
      return nil_class;
    else
    {
      rObject fwd = object_class->slot_get(SYMBOL(locateSlot));
      return runner.apply(fwd, args);
    }
  }

  static rObject
  scope_class_doSetSlot(runner::Runner& runner, objects_type args)
  {
    CHECK_ARG_COUNT(3);
    FETCH_ARG(1, String);


    rObject fwd = object_class->slot_get(SYMBOL(setSlot));
    rObject outer = args[0]->slot_locate(SYMBOL(self));
    if (args[0] == outer)
    {
      args[0] = outer->own_slot_get(SYMBOL(self));
      return runner.apply(fwd, args);
    }
    else
      return runner.apply(fwd, args);
  }

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
    DECLARE(doSetSlot);
    DECLARE(getSlot);
    DECLARE(locals);
    DECLARE(locateSlot);
    DECLARE(removeSlot);
    DECLARE(target);
    DECLARE(tryTarget);
    DECLARE(updateSlot);
#undef DECLARE
  }

} // namespace object

