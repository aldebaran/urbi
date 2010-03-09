/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/job.cc
 ** \brief Creation of the Urbi object job.
 */

#include <sstream>

#include <boost/any.hpp>

#include <kernel/userver.hh>
#include <urbi/object/float.hh>
#include <urbi/object/list.hh>
#include <urbi/object/object.hh>
#include <urbi/object/string.hh>
#include <object/symbols.hh>
#include <urbi/object/tag.hh>
#include <urbi/object/job.hh>
#include <runner/interpreter.hh>
#include <runner/runner.hh>

namespace urbi
{
  namespace object
  {
    using runner::Runner;

    Job::Job()
      : value_(0)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Job::Job(const value_type& value)
      : value_(value)
    {
      proto_add(Job::proto);
      slot_set(SYMBOL(exceptionHandlerTag), nil_class);
    }

    Job::Job(rJob model)
      : value_(model->value_)
    {
      proto_add(Tag::proto);
    }

    const Job::value_type&
    Job::value_get() const
    {
      return value_;
    }

    libport::Symbol
    Job::name()
    {
      return value_->name_get();
    }

    const runner::tag_stack_type
    Job::tags()
    {
      return dynamic_cast<runner::Interpreter*>(value_.get())->tag_stack_get();
    }

    std::string
    Job::status()
    {
      Runner& r = ::kernel::urbiserver->getCurrentRunner();

      std::stringstream status;
      switch (value_->state_get())
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
        status << " (current job)";
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
    Job::backtrace()
    {
      List::value_type res;
      if (const Runner* runner = dynamic_cast<Runner*>(value_.get()))
      {
        foreach(Runner::frame_type frame, runner->backtrace_get())
          libport::push_front(res, frame);
      }
      return new List(res);
    }

    void
    Job::waitForTermination()
    {
      Runner& r = ::kernel::urbiserver->getCurrentRunner();
      r.yield_until_terminated(*value_);
    }

    void
    Job::waitForChanges()
    {
      Runner& r = ::kernel::urbiserver->getCurrentRunner();
      r.yield_until_things_changed();
    }

    void
    Job::terminate()
    {
      value_->terminate_now();
    }

    void
    Job::setSideEffectFree(rObject b)
    {
      value_->side_effect_free_set(b->as_bool());
    }

    rFloat
    Job::timeShift()
    {
      return new Float(value_->time_shift_get() / 1000000.0);
    }

    void
    Job::initialize(CxxObject::Binder<Job>& bind)
    {
      bind(SYMBOL(backtrace), &Job::backtrace);
      bind(SYMBOL(name), &Job::name);
      bind(SYMBOL(setSideEffectFree), &Job::setSideEffectFree);
      bind(SYMBOL(status), &Job::status);
      bind(SYMBOL(tags), &Job::tags);
      bind(SYMBOL(terminate), &Job::terminate);
      bind(SYMBOL(timeShift), &Job::timeShift);
      bind(SYMBOL(waitForChanges), &Job::waitForChanges);
      bind(SYMBOL(waitForTermination), &Job::waitForTermination);
    }

    URBI_CXX_OBJECT_REGISTER(Job)
    {}

  }; // namespace object
}
