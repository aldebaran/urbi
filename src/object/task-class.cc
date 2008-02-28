/**
 ** \file object/task-class.cc
 ** \brief Creation of the URBI object task.
 */

#include <boost/any.hpp>

#include "object/task-class.hh"

#include "object/alien.hh"
#include "object/atom.hh"
#include "object/object.hh"

#include "runner/runner.hh"

namespace object
{
  rObject task_class;

  typedef libport::shared_ptr<runner::Runner> rRunner;

  /*-------------------.
  | Task primitives.  |
  `-------------------*/

  static rObject
  task_class_init (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT_RANGE (2, 3);
    FETCH_ARG (1, Code);

    rList locals = new List (std::list<rObject> ());
    ast::Exp *body = arg1->value_get ().body_get ();
    rRunner new_runner = new runner::Runner (r.lobby_get(),
					     args[1]->slot_get (SYMBOL(context)),
					     r.scheduler_get (),
					     body);
    args[0]->slot_set (SYMBOL (runner), box (rRunner, new_runner));

    if (args.size () == 3)
    {
      if (IS_TRUE (args[2]))
	r.link (new_runner.get ());
    }

    new_runner->start_job ();

    return args[0];
  }

  // Helper function: check that there are no argument, unbox the
  // runner and return it once it has terminated.
  static inline rRunner
  yield_until_terminated (runner::Runner& r, rObject o)
  {
    rRunner other = unbox (rRunner, o->slot_get (SYMBOL (runner)));
    r.yield_until_terminated (*other);
    return other;
  }

  static rObject
  task_class_waitForTermination (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);

    yield_until_terminated (r, args[0]);
    return void_class;
  }

  static rObject
  task_class_waitForChanges (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);

    r.yield_until_things_changed ();
    return void_class;
  }

  static rObject
  task_class_result (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);

    return yield_until_terminated (r, args[0])->current_get ();
  }

  static rObject
  task_class_terminate (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);

    rRunner r = unbox (rRunner, args[0]->slot_get (SYMBOL (runner)));
    r->terminate_now ();

    return void_class;
  }

  static rObject
  task_class_setSideEffectFree (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    r.side_effect_free_set (IS_TRUE (args[1]));
    return void_class;
  }

  void
  task_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(task, Name);
    DECLARE (init);
    DECLARE (result);
    DECLARE (setSideEffectFree);
    DECLARE (terminate);
    DECLARE (waitForChanges);
    DECLARE (waitForTermination);
#undef DECLARE
  }

}; // namespace object
