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
#include <urbi/object/dictionary.hh>
#include <urbi/object/float.hh>
#include <urbi/object/job.hh>
#include <urbi/object/list.hh>
#include <urbi/object/lobby.hh>
#include <urbi/object/object.hh>
#include <urbi/object/string.hh>
#include <urbi/object/symbols.hh>
#include <urbi/object/tag.hh>

#include <runner/job.hh>

namespace urbi
{
  namespace object
  {
    Job::Job(const value_type& value)
      : value_(value)
    {
      proto_add(Job::proto);
      slot_set_value(SYMBOL(exceptionHandlerTag), nil_class);
    }

    Job::Job(rJob model)
      : value_(model->value_)
    {
      proto_add(Tag::proto);
    }

    URBI_CXX_OBJECT_INIT(Job)
      : value_(0)
    {
      BIND(DOLLAR_backtrace, backtrace);
      BINDG(lobby, lobby_get);
      BINDG(current);
      BINDG(jobs);
      BIND(resetStats);
      BINDG(stats);
      BINDG(status);
      BINDG(interruptible);
      BINDG(frozen);
      BINDG(tags);
      BIND(terminate);
      BINDG(timeShift);
      BIND(waitForTermination);
      BIND(breakTag);
    }

    const Job::value_type&
    Job::value_get() const
    {
      return value_;
    }

    rObject
    Job::lobby_get()
    {
      return value_->state.lobby_get();
    }

    rJob
    Job::current()
    {
      return ::kernel::runner().as_job();
    }

    List::value_type
    Job::jobs()
    {
      List::value_type res;
      foreach (sched::rJob job, ::kernel::scheduler().jobs_get())
        if (rJob o = static_cast<runner::Job*>(job.get())->as_job())
          res << o;
      return res;
    }

    const runner::State::tag_stack_type
    Job::tags() const
    {
      return value_
        ? value_.get()->state.tag_stack_get()
        : runner::State::tag_stack_type();
    }

    bool
    Job::interruptible() const
    {
      return !value_ || !value_->non_interruptible_get();
    }

    bool
    Job::frozen() const
    {
      return value_ && value_->frozen();
    }

    std::string
    Job::status() const
    {
      if (!value_)
        return "";

      runner::Job& r = ::kernel::runner();

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
      if (value_.get() == &r)
        status << " (current job)";
      if (value_->frozen())
        status << " (frozen)";
      if (value_->has_pending_exception())
        status << " (pending exception)";
      if (value_->non_interruptible_get())
        status << " (non interruptible)";
      return status.str();
    }

    rObject
    Job::stats() const
    {
      const sched::Job::stats_type& stats =
        value_->stats_get();

      // The space after "Symbol(" is mandatory to avoid triggering an error in
      // symbol generation code
      Dictionary::value_type res;
#define ADDENTRY(Name, Value, Divisor)          \
      res[new String(Name)] =                   \
        new Float(Value / Divisor)

#define ADDSTATS(Prefix, Stat)                                          \
      ADDENTRY(Prefix         , Stat.size(), 1);                        \
      if (Stat.size() != 0)                                             \
      {                                                                 \
        ADDENTRY(Prefix "Max" , Stat.max(), 1e6);                       \
        ADDENTRY(Prefix "Mean", Stat.mean(), 1e6);                      \
        ADDENTRY(Prefix "Min" , Stat.min(), 1e6);                       \
        ADDENTRY(Prefix "Dev" , Stat.standard_deviation(), 1e6);        \
        ADDENTRY(Prefix "Var" , Stat.variance(), 1e6);                  \
      }                                                                 \
      else                                                              \
      {                                                                 \
        ADDENTRY(Prefix "Max" , 0, 1e6);                                \
        ADDENTRY(Prefix "Mean", 0, 1e6);                                \
        ADDENTRY(Prefix "Min" , 0, 1e6);                                \
        ADDENTRY(Prefix "Dev" , 0, 1e6);                                \
        ADDENTRY(Prefix "Var" , 0, 1e6);                                \
      }

#define ADDJOBSTATS(Prefix, Thread)                             \
      ADDSTATS(Prefix "Cycles", Thread.running);                \
      ADDSTATS(Prefix "Waiting", Thread.waiting);               \
      ADDSTATS(Prefix "Sleeping", Thread.sleeping);             \
      ADDENTRY(Prefix "Fork", Thread.nb_fork, 1);               \
      ADDENTRY(Prefix "Join", Thread.nb_join, 1);               \
      ADDENTRY(Prefix "WorkflowBreak", Thread.nb_exn, 1);

      ADDJOBSTATS("", stats.job);
      ADDJOBSTATS("TerminatedChildren", stats.terminated_children);

#undef ADDJOBSTATS
#undef ADDSTATS
#undef ADDENTRY
      // Give infos about the import stack.
      const runner::State& state = value_->state;
      res[new String("ImportStack")] = to_urbi(state.import_stack);
      res[new String("ImportCapture")] = to_urbi(state.import_captured);
      return new Dictionary(res);
    }

    void
    Job::resetStats()
    {
      value_->stats_reset();
    }

    rList
    Job::backtrace() const
    {
      List::value_type res;
      if (const runner::Job* r = value_ ? value_.get() : 0)
      {
        foreach(runner::State::call_frame_type frame, r->state.backtrace_get())
          res.push_front(frame);
      }
      return new List(res);
    }

    void
    Job::waitForTermination()
    {
      if (value_)
        ::kernel::runner().yield_until_terminated(*value_);
    }

    void
    Job::terminate()
    {
      if (value_)
        value_->terminate_now();
    }

    libport::ufloat
    Job::timeShift() const
    {
      return value_ ? value_->time_shift_get() / 1000000.0 : 0;
    }

    void
    Job::breakTag(int depth, rObject value)
    {
      if (depth < 0)
      {
        depth = value_->state.tag_stack_size() + depth;
      }
      value_->async_throw(sched::StopException(depth, value));
    }
  }; // namespace object
}
