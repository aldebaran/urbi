/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
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

#include <urbi/kernel/userver.hh>
#include <urbi/object/float.hh>
#include <urbi/object/list.hh>
#include <urbi/object/object.hh>
#include <urbi/object/string.hh>
#include <urbi/object/symbols.hh>
#include <urbi/object/lobby.hh>
#include <urbi/object/job.hh>
#include <urbi/object/tag.hh>
#include <runner/interpreter.hh>
#include <runner/runner.hh>

namespace urbi
{
  namespace object
  {
    using runner::Runner;

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

    URBI_CXX_OBJECT_INIT(Job)
    {
#define DECLARE(Name, Cxx)          \
      bind(SYMBOL_(Name), &Job::Cxx)

      DECLARE(DOLLAR_backtrace,   backtrace);
      DECLARE(name,               name);
      DECLARE(setSideEffectFree,  setSideEffectFree);
      DECLARE(status,             status);
      DECLARE(tags,               tags);
      DECLARE(terminate,          terminate);
      DECLARE(timeShift,          timeShift);
      DECLARE(waitForChanges,     waitForChanges);
      DECLARE(waitForTermination, waitForTermination);

#undef DECLARE
    }

    const Job::value_type&
    Job::value_get() const
    {
      return value_;
    }

    const std::string&
    Job::name() const
    {
      static std::string otherwise("Job");
      return value_ ? value_->name_get() : otherwise;
    }

    tag_stack_type
    Job::tags() const
    {
      return value_
        ? dynamic_cast<runner::Interpreter*>(value_.get())->tag_stack_get()
        : tag_stack_type();
    }

    std::string
    Job::status()
    {
      if (!value_)
        return "";

      Runner& r = ::kernel::runner();

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
      if (!value_)
        return new List();

      List::value_type res;
      if (const Runner* runner = dynamic_cast<Runner*>(value_.get()))
      {
        foreach(Runner::frame_type frame, runner->backtrace_get())
          res.push_front(frame);
      }
      return new List(res);
    }

    void
    Job::waitForTermination()
    {
      if (!value_)
        return;
      Runner& r = ::kernel::runner();
      r.yield_until_terminated(*value_);
    }

    void
    Job::waitForChanges()
    {
      if (!value_)
        return;
      Runner& r = ::kernel::runner();
      r.yield_until_things_changed();
    }

    void
    Job::terminate()
    {
      if (!value_)
        return;
      value_->terminate_now();
    }

    void
    Job::setSideEffectFree(rObject b)
    {
      if (!value_)
        return;
      value_->side_effect_free_set(b->as_bool());
    }

    rFloat
    Job::timeShift() const
    {
      if (!value_)
        return new Float(0);
      return new Float(value_->time_shift_get() / 1000000.0);
    }
  }; // namespace object
}
