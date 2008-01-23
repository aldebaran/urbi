/**
 ** \file scheduler/scheduler.cc
 ** \brief Implementation of scheduler::Scheduler.
 */

//#define ENABLE_DEBUG_TRACES

#include <cassert>

#include <boost/foreach.hpp>

#include "libport/compiler.hh"
#include "libport/containers.hh"

#include "scheduler/scheduler.hh"
#include "scheduler/job.hh"

namespace scheduler
{

  // This function is required to start a new job using the libcoroutine.
  // It's only purpose is to create the context and start the execution
  // of the new job.
  static void
  run_job (void* job)
  {
    ((Job *)job)->run();
  }

  void
  Scheduler::add_job (Job* job)
  {
    assert (job);
    assert (!libport::has (jobs_, job));
    assert (!libport::has (jobs_to_start_, job));
    jobs_to_start_.push_back (job);
  }

  void
  Scheduler::work ()
  {
#ifdef ENABLE_DEBUG_TRACES
    static int cycle = 0;
#endif
    ECHO ("======================================================== cycle "
	  << ++cycle);

    jobs to_start;
    std::swap (to_start, jobs_to_start_);
    BOOST_FOREACH(Job* job, to_start)
    {
      assert (job);
      ECHO ("will start job " << job);
      // Job will start for a very short time and do a yield_front() to
      // be restarted below in the course of the regular cycle.
      Coro_startCoro_ (self_, job->coro_get(), job, run_job);
    }

    // Run all the jobs in the run queue once.
    jobs pending;
    std::swap (pending, jobs_);

    ECHO (pending.size() << " jobs in the queue for this cycle");
    BOOST_FOREACH(Job* job, pending)
    {
      assert (job);
      assert (!job->terminated ());
      ECHO ("will resume job " << job);
      Coro_switchTo_ (self_, job->coro_get ());
      ECHO ("back from job " << job);
    }
  }

  void
  Scheduler::switch_back (Job *job)
  {
    // Switch back to the scheduler
    Coro_switchTo_ (job->coro_get (), self_);
    // We regained control, we are again in the context of the job.
    ECHO ("job " << job << " resumed");
  }

  void
  Scheduler::resume_scheduler (Job* job)
  {
    // If the job has not terminated, put it at the back of the run queue
    // so that the run queue order is preserved between work cycles.
    if (!job->terminated ())
      jobs_.push_back (job);
    switch_back (job);
  }

  void
  Scheduler::resume_scheduler_front (Job *job)
  {
    // If the job has not terminated, put it at the front of the run queue.
    if (!job->terminated ())
      jobs_.push_front (job);
    switch_back (job);
  }

  void
  Scheduler::killall_jobs ()
  {
    ECHO ("killing all jobs!");
#ifdef ENABLE_DEBUG_TRACES
    jobs::size_type n = jobs_.size ();
    unsigned i = 0;
#endif
    BOOST_FOREACH (Job* job, jobs_)
    {
      assert (job);
      ECHO ("stopping job " << ++i << '/' << n << ": " << job);
      kill_job (job);
    }
  }

  void
  Scheduler::kill_job (Job* job)
  {
    assert (job);
    assert (libport::has (jobs_, job));
    job->terminate_now ();
    jobs_.remove (job);
    ECHO ("deleting job " << job);
    delete job;
  }

} // namespace scheduler
