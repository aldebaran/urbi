/**
 ** \file runner/scheduler.cc
 ** \brief Implementation of runner::Scheduler.
 */

#define ENABLE_DEBUG_TRACES

#include <cassert>

#include <boost/foreach.hpp>

#include "libport/compiler.hh"
#include "libport/containers.hh"

#include "runner/fwd.hh"
#include "runner/scheduler.hh"
#include "runner/job.hh"
#include "runner/coroutine-yield.hh"

namespace runner
{

  void
  Scheduler::add_job (Job* job)
  {
    assert (job);
    ECHO ("adding job " << job << " to the task queue.");
    jobs_.push_back (job);
  }

  void
  Scheduler::work ()
  {
#ifdef ENABLE_DEBUG_TRACES
    static int cycle = 0;
#endif
    static int nothing_todo = 0;
    if (nothing_todo >= 5)
    {
      ECHO ("It looks like I have nothing to do.  You are so boring.");
      exit (0);
    }
    ECHO ("======================================================== cycle "
	  << ++cycle);

    jobs pending;
    pending.swap (jobs_);
    jobs::size_type n = pending.size ();
    ECHO (n << " jobs pending");
    if (n == 0)
      ++nothing_todo;
    else
      nothing_todo = 0;
#ifdef ENABLE_DEBUG_TRACES
    unsigned i = 0;
#endif
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
    active_job_ = job;
    try {
      job->run ();
    }
    catch (const CoroutineYield&)
    {
    }
    active_job_ = 0;
  }

  void
  Scheduler::killall_jobs ()
  {
    ECHO ("killing all jobs!");
    jobs pending;
    pending.swap (jobs_);
#ifdef ENABLE_DEBUG_TRACES
    jobs::size_type n = pending.size ();
    unsigned i = 0;
#endif
    BOOST_FOREACH (Job* job, pending)
    {
      assert (job);
      ECHO ("stopping job " << ++i << '/' << n << ": " << job);
      kill_job (job);
    }
    if (active_job_)
    {
      ECHO ("stopping active job: " << active_job_);
      kill_job (active_job_);
    }
  }

  void
  Scheduler::kill_job (Job* job)
  {
    assert (job);
    assert (active_job_ == job || libport::has (jobs_, job));
    job->terminate ();
    ECHO ("deleting job " << job);
    delete job;
  }

} // namespace runner
