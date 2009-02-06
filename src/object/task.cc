/**
 ** \file object/task-class.cc
 ** \brief Creation of the URBI object task.
 */

#include <sstream>

#include <boost/any.hpp>

#include <kernel/userver.hh>
#include <object/float.hh>
#include <object/list.hh>
#include <object/object.hh>
#include <object/string.hh>
#include <object/tag.hh>
#include <object/task.hh>
#include <runner/interpreter.hh>
#include <runner/runner.hh>

namespace object
{
  Task::Task()
    : value_(0)
  {
    proto_add(proto ? proto : object_class);
  }

  Task::Task(const value_type& value)
    : value_(value)
  {
    proto_add(Task::proto);
    slot_set(SYMBOL(exceptionHandlerTag), nil_class);
  }

  Task::Task(rTask model)
    : value_(model->value_)
  {
    proto_add(Tag::proto);
  }

  const Task::value_type&
  Task::value_get() const
  {
    return value_;
  }

  libport::Symbol
  Task::name()
  {
    return value_->name_get();
  }

  const runner::tag_stack_type
  Task::tags()
  {
    return dynamic_cast<runner::Interpreter*>(value_.get())->tag_stack_get();
  }

  std::string
  Task::status()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

    std::stringstream status;
    switch(value_->state_get())
    {
    case sched::to_start:
      status << "starting";
      break;
    case sched::running:
      status << "running";
      break;
    case sched::sleeping:
      status << "sleeping";
      break;
    case sched::waiting:
      status << "idle";
      break;
    case sched::joining:
      status << "waiting";
      break;
    case sched::zombie:
      status << "terminated";
      break;
    }
    if (value_ == &r)
      status << " (current task)";
    if (value_->frozen())
      status << " (frozen)";
    if (value_->has_pending_exception())
      status << " (pending exception)";
    if (value_->side_effect_free_get())
      status << " (side effect free)";
    if (value_->non_interruptible_get())
      status << " (non interruptible)";
    return status.str();
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
      frame.push_back(new String(line.first));
      frame.push_back(new String(line.second));
      libport::push_front(res, new List(frame));
    }
    return new List(res);
  }

  void
  Task::waitForTermination()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    r.yield_until_terminated(*value_);
  }

  void
  Task::waitForChanges()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
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
    value_->side_effect_free_set(is_true(b));
  }

  rFloat
  Task::timeShift()
  {
    return new Float(value_->time_shift_get() / 1000000.0);
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

  rObject
  Task::proto_make()
  {
    return new Task();
  }

  URBI_CXX_OBJECT_REGISTER(Task);
}; // namespace object
