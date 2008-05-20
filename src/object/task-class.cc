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

  /*-------------------.
  | Task primitives.  |
  `-------------------*/

  static rObject
  task_class_init (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT_RANGE (2, 3);
    FETCH_ARG (1, Code);

    runner::Runner* new_runner =
      new runner::Runner (r.lobby_get(),
			  r.locals_get(),
			  r.scheduler_get (),
			  arg1);
    new_runner->copy_tags (r);
    new_runner->time_shift_set (r.time_shift_get ());
    args[0]->slot_set (SYMBOL (runner),
                       box (scheduler::rJob, new_runner->myself_get ()));

    if (args.size () == 3)
    {
      if (is_true (args[2]))
	r.link (new_runner);
    }

    new_runner->start_job ();

    return args[0];
  }

  // Helper function: check that there are no argument, unbox the
  // runner and return it once it has terminated.
  static inline runner::Runner*
  yield_until_terminated (runner::Runner& r, rObject o)
  {
    scheduler::rJob other = unbox (scheduler::rJob,
                                   o->slot_get (SYMBOL (runner)));
    r.yield_until_terminated (*other);
    return dynamic_cast<runner::Runner*>(other.get ());
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
  task_class_terminate (runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT (1);

    scheduler::rJob r = unbox (scheduler::rJob,
                               args[0]->slot_get (SYMBOL (runner)));
    r->terminate_now ();

    return void_class;
  }

  static rObject
  task_class_setSideEffectFree (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    r.side_effect_free_set (is_true (args[1]));
    return void_class;
  }

  static rObject
  task_class_timeShift (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return Float::fresh (r.time_shift_get () / 1000.0);
  }

  void
  task_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(task, Name);
    DECLARE (init);
    DECLARE (setSideEffectFree);
    DECLARE (terminate);
    DECLARE (timeShift);
    DECLARE (waitForChanges);
    DECLARE (waitForTermination);
#undef DECLARE
  }

}; // namespace object
