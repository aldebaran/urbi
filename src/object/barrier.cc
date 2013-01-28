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

#include <libport/cassert>
#include <libport/containers.hh>
#include <libport/foreach.hh>
#include <libport/range.hh>

#include <urbi/kernel/userver.hh>

#include <urbi/object/barrier.hh>
#include <urbi/object/cxx-object.hh>
#include <urbi/object/float.hh>
#include <runner/job.hh>
#include <urbi/object/symbols.hh>

namespace urbi
{
  namespace object
  {
    using kernel::runner;

    Barrier::Barrier(rBarrier model)
      : value_(model->value_)
      , rest_(value_.end())
    {
      proto_add(model);
    }

    Barrier::Barrier(const value_type& value)
      : value_(value)
      , rest_(value_.end())
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    URBI_CXX_OBJECT_INIT(Barrier)
      : value_()
      , rest_()
    {
      BIND(new, _new);
      BIND(signal);
      BIND(signalAll);
      BIND(wait);
    }

    rBarrier
    Barrier::_new(rObject)
    {
      return new Barrier(value_type());
    }

    // The rest_ iterator is used to keep track of the elements which are
    // still waiting to receive a signal.  The left side of the rest_
    // iterator are jobs that received a signal and that are waiting
    // to get spawned.  The right side of rest_ iterator (included) are jobs
    // waiting to get a signal.  Thus the rest_ iterator is
    // pointing to the next job that should be signaled.
    //
    // The current implementation uses a list to queue jobs
    // waiting to receive a signal.  A waiter is responsible to insert
    // itself at the end and to remove itself from the list.
    //
    //  value_ |- - - signaled - - - - - - waiters - - - -|
    //                             ^
    //                           rest_
    //
    // When the rest_ iterator is at the end, no jobs are waiting on
    // the barrier.

    rObject
    Barrier::wait()
    {
      runner::Job& r = runner();
      value_type::iterator i =
        value_.insert(value_.end(), value_type::value_type(&r, rObject()));

      // No waiters are in the list, so mark it-self as the next waiter.
      if (rest_ == value_.end())
        rest_ = i;

      try
      {
        r.frozen_set(true);
        r.yield();

        // In no circumstances a signaled process could be marked as being a
        // waiter, this could only mean that the rest_ iterator has not been
        // correctly updated.
        aver(rest_ != i);

        // Regular wake up from a barrier wait.
        // Get the payload and remove us from the queue.
        rObject payload = i->second;
        value_.erase(i);
        return payload;
      }
      catch (...)
      {
        // In some rare cases when we receive a signal and an exception, the
        // payload is ignored and not forwarded to another waiter.  The
        // reason is to avoid getting his own signal back.  This differs
        // from the semaphore implementation.

        // Unfreeze the job.
        r.frozen_set(false);

        // Signal that we should no longer stay queued.  If the current
        // waiter is the next to be signaled, then skip it.  Remove it-self
        // from the list of waiters.
        if (rest_ == i)
          rest_++;

        // The erase operation should not go before the potential rest_
        // update, because this would invalidate the rest_ pointer.
        value_.erase(i);

        // Rethrow the exception received.
        throw;
      }
    }

    unsigned int
    Barrier::signal(rObject payload)
    {
      // No waiters are in the queue.
      if (rest_ == value_.end())
        return 0;

      // Signal the job by making it free to continue after the yield.
      rest_->first->frozen_set(false);
      rest_->second = payload;

      // mark the next waiter as the next job to be signaled.
      rest_++;
      return 1;
    }

    unsigned int
    Barrier::signalAll(rObject payload)
    {
      unsigned int res = 0;
      value_type::iterator end = value_.end();

      // Signal all jobs in the queue by making them free to continue after
      // their yields.
      foreach (value_type::value_type& jp,
               boost::make_iterator_range(rest_, end))
      {
        jp.first->frozen_set(false);
        jp.second = payload;
        res++;
      }

      // No jobs are waiting to be signaled.
      rest_ = end;
      return res;
    }
  } // namespace object
}
