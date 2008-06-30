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

  Task::Task()
    : value_(0)
  {
    proto_add(task_class);
  }

  Task::Task(const value_type& value)
    : value_(value)
  {
    proto_add(task_class);
  }

  Task::Task(rTask model)
    : value_(model->value_)
  {
    proto_add(tag_class);
  }

  const Task::value_type&
  Task::value_get() const
  {
    return value_;
  }

  rString
  Task::name()
  {
    return new String(value_->name_get());
  }

  rList
  Task::tags()
  {
    List::value_type res;
    foreach(scheduler::rTag tag, value_->tags_get())
      res.push_back(new Tag(tag));
    return new List(res);
  }

  rString
  Task::status()
  {
    std::stringstream status;
    switch(value_->state_get())
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
    if (value_->frozen())
      status << " (frozen)";
    if (value_->has_pending_exception())
      status << " (pending exception)";
    if (value_->side_effect_free_get())
      status << " (side effect free)";
    if (value_->non_interruptible_get())
      status << " (non interruptible)";
    return new String(libport::Symbol(status.str()));
  }

  rList
  Task::backtrace()
  {
    List::value_type res;

    const runner::Runner* runner = dynamic_cast<runner::Runner*>(value_.get());

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

  void
  Task::waitForTermination(runner::Runner& r)
  {
    r.yield_until_terminated(*value_);
  }

  void
  Task::waitForChanges(runner::Runner& r)
  {
    r.yield_until_things_changed();
  }

  void
  Task::terminate()
  {
    value_->terminate_now();
  }

  void
  Task::setSideEffectFree(rObject b)
  {
    value_->side_effect_free_set(is_true(b, SYMBOL(setSideEffectFree)));
  }

  rFloat
  Task::timeShift()
  {
    return new Float(value_->time_shift_get() / 1000.0);
  }

  void
  Task::initialize(CxxObject::Binder<Task>& bind)
  {
    bind(SYMBOL(backtrace), &Task::backtrace);
    bind(SYMBOL(name), &Task::name);
    bind(SYMBOL(setSideEffectFree), &Task::setSideEffectFree);
    bind(SYMBOL(status), &Task::status);
    bind(SYMBOL(tags), &Task::tags);
    bind(SYMBOL(terminate), &Task::terminate);
    bind(SYMBOL(timeShift), &Task::timeShift);
    bind(SYMBOL(waitForChanges), &Task::waitForChanges);
    bind(SYMBOL(waitForTermination), &Task::waitForTermination);
  }

  bool Task::task_added = CxxObject::add<Task>("Task", task_class);
  const std::string Task::type_name = "Task";
  std::string Task::type_name_get() const
  {
    return type_name;
  }
}; // namespace object
