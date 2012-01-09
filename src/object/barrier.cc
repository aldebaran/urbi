/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <boost/lambda/lambda.hpp>

#include <libport/containers.hh>

#include <urbi/kernel/userver.hh>

#include <urbi/object/cxx-object.hh>
#include <urbi/object/barrier.hh>
#include <urbi/object/float.hh>
#include <urbi/object/symbols.hh>

# include <runner/runner.hh>

namespace urbi
{
  namespace object
  {
    using kernel::runner;

    Barrier::Barrier(rBarrier model)
      : value_(model->value_)
    {
      proto_add(model);
    }

    Barrier::Barrier(const value_type& value)
      : value_(value)
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    URBI_CXX_OBJECT_INIT(Barrier)
    {
#define DECLARE(Name, Cxx)              \
      bind(SYMBOL_(Name), &Barrier::Cxx)

      DECLARE(new,       _new);
      DECLARE(signal,    signal);
      DECLARE(signalAll, signalAll);
      DECLARE(wait,      wait);

#undef DECLARE
    }

    struct BarrierException : public sched::SchedulerException
    {
      BarrierException(rObject payload) : payload_(payload) {};
      ~BarrierException() throw() {}
      ADD_FIELD(rObject, payload)
      COMPLETE_EXCEPTION(BarrierException)
    };

    rBarrier
    Barrier::_new(rObject)
    {
      return new Barrier(value_type());
    }

    static bool
    job_compare(sched::rJob lhs, sched::rJob rhs)
    {
      return lhs == rhs;
    }

    rObject
    Barrier::wait()
    {
      runner::Runner& r = runner();
      value_.push_back(&r);
      try
      {
        r.yield_until_terminated(r);
        // We cannot return from here.
        pabort("yield_until_terminated returned");
      }
      catch (const BarrierException& be)
      {
        // Regular wake up from a barrier wait.
        return be.payload_get();
      }
      catch (...)
      {
        // Signal that we should no longer stay queued.
        libport::erase_if(value_, boost::bind(job_compare, &r, _1));
        throw;
      }
    }

    unsigned int
    Barrier::signal(rObject payload)
    {
      if (value_.empty())
        return 0;

      value_.front()->async_throw(BarrierException(payload));
      value_.pop_front();
      return 1;
    }

    unsigned int
    Barrier::signalAll(rObject payload)
    {
      const unsigned int res = value_.size();

      foreach (sched::rJob job, value_)
        job->async_throw(BarrierException(payload));
      value_.clear();

      return res;
    }
  } // namespace object
}
