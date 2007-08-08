/**
 ** \file runner/scheduler.cc
 ** \brief Implementation of runner::Scheduler.
 */

#define ENABLE_DEBUG_TRACES

#include <cassert>

#include <boost/foreach.hpp>

#include "libport/compiler.hh"

#include "runner/fwd.hh"
#include "runner/scheduler.hh"
#include "runner/job.hh"

namespace runner
{

  void
  Scheduler::work ()
  {
    jobs pending;
    pending.swap (jobs_);
    jobs::size_type n = pending.size ();
    ECHO (n << " jobs pending");
    unsigned i = 0;
    BOOST_FOREACH (Job* job, pending)
    {
      assert (job);
      ECHO ("scheduling job " << ++i << '/' << n << ": " << job);
      schedule_immediately (job);
    }
  }

  void
  Scheduler::schedule_immediately (Job* job)
  {
    assert (job);
    try {
      job->run ();
    }
    catch (const CoroutineYield&)
    {
      add_job (job);
    }
  }

} // namespace runner
