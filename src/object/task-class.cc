/**
 ** \file object/task-class.cc
 ** \brief Creation of the URBI object task.
 */

#include <sstream>

#include <boost/any.hpp>

#include <object/task-class.hh>

#include <object/alien.hh>
#include <object/atom.hh>
#include <object/float-class.hh>
#include <object/list-class.hh>
#include <object/object.hh>
#include <object/string-class.hh>
#include <object/tag-class.hh>

#include <runner/interpreter.hh>
#include <runner/runner.hh>

namespace object
{
  rObject task_class;

  /*-------------------.
  | Helper functions.  |
  `-------------------*/

  rObject
  create_task_from_job(const scheduler::rJob& job)
  {
    rObject res = task_class->clone();
    res->slot_set(SYMBOL(job),
		  box(scheduler::rJob, job));
    return res;
  }

  static scheduler::rJob
  extract_job(const rObject& o)
  {
    return unbox(scheduler::rJob, o->slot_get(SYMBOL(job)));
  }

  /*-------------------.
  | Task primitives.  |
  `-------------------*/

  static rObject
  task_class_name(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    return new String(extract_job(args[0])->name_get());
  }

  static rObject
  task_class_tags(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    List::value_type res;
    foreach(scheduler::rTag tag, extract_job(args[0])->tags_get())
      res.push_back(new Tag(tag));
    return new List(res);
  }

  static rObject
  task_class_status(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    scheduler::rJob job = extract_job(args[0]);
    std::stringstream status;
    switch(job->state_get())
    {
    case scheduler::to_start:
      status << "starting";
      break;
    case scheduler::running:
      status << "running";
      break;
    case scheduler::sleeping:
      status << "sleeping";
      break;
    case scheduler::waiting:
      status << "idle";
      break;
    case scheduler::joining:
      status << "waiting";
      break;
    case scheduler::zombie:
      status << "terminated";
      break;
    }
    if (job->frozen())
      status << " (frozen)";
    if (job->has_pending_exception())
      status << " (pending exception)";
    if (job->side_effect_free_get())
      status << " (side effect free)";
    if (job->non_interruptible_get())
      status << " (non interruptible)";
    return new String(libport::Symbol(status.str()));
  }

  static rObject
  task_class_backtrace(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);

    List::value_type res;

    scheduler::rJob job = extract_job(args[0]);
    const runner::Runner* runner = dynamic_cast<runner::Runner*>(job.get());

    if (!runner)
      return new List(res);

    foreach(runner::Runner::frame_type line, runner->backtrace_get())
    {
      List::value_type frame;
      frame.push_back(new String(libport::Symbol(line.first)));
      frame.push_back(new String(libport::Symbol(line.second)));
      res.push_front(new List(frame));
    }
    return new List(res);
  }

  static rObject
  task_class_waitForTermination (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    r.yield_until_terminated(*extract_job(args[0]));
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
    extract_job(args[0])->terminate_now();
    return void_class;
  }

  static rObject
  task_class_setSideEffectFree (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (2);
    r.side_effect_free_set (is_true (args[1], SYMBOL(setSideEffectFree)));
    return void_class;
  }

  static rObject
  task_class_timeShift (runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT (1);
    return new Float(r.time_shift_get () / 1000.0);
  }

  void
  task_class_initialize ()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(task, Name);
    DECLARE(backtrace);
    DECLARE(name);
    DECLARE(setSideEffectFree);
    DECLARE(status);
    DECLARE(tags);
    DECLARE(terminate);
    DECLARE(timeShift);
    DECLARE(waitForChanges);
    DECLARE(waitForTermination);
#undef DECLARE
  }

}; // namespace object
