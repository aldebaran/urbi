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
    static int cycle = 0;
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
    active_job_ = job;
    try {
      job->run ();
    }
    catch (const CoroutineYield&)
    {
      add_job (job);
    }
    active_job_ = 0;
  }

  void
  Scheduler::killall_jobs ()
  {
    ECHO ("killing all jobs!");
    jobs pending;
    pending.swap (jobs_);
    jobs::size_type n = pending.size ();
    unsigned i = 0;
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
    assert (active_job_ == job
            || std::find (jobs_.begin (), jobs_.end (), job) != jobs_.end ());
    job->terminate ();
    ECHO ("deleting job " << job);
    delete job;
  }

} // namespace runner
