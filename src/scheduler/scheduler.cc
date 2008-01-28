/**
 ** \file scheduler/scheduler.cc
 ** \brief Implementation of scheduler::Scheduler.
 */

//#define ENABLE_DEBUG_TRACES

#include <cassert>

#include <boost/foreach.hpp>

#include <libport/compiler.hh>
#include <libport/containers.hh>

#include "kernel/userver.hh"

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

    // Start new jobs
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

    // Start deferred jobs
    libport::ufloat current_time = ::urbiserver->getTime ();
    while (!deferred_jobs_.empty ()) {
      deferred_job j = deferred_jobs_.top ();
      if (j.get<0>() > current_time)
	break;
      jobs_.push_back (j.get<1>());
      deferred_jobs_.pop ();
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
    // Put the job in front of the queue. If the job asks to be requeued,
    // it is not terminated.
    assert (!job->terminated ());
    jobs_.push_front (job);
    switch_back (job);
  }

  void
  Scheduler::resume_scheduler_until (Job *job, libport::utime_t deadline)
  {
    // Put the job in the deferred queue. If the job asks to be requeued,
    // it is not terminated.
    assert (!job->terminated ());
    deferred_jobs_.push (boost::make_tuple (deadline, job));
    switch_back (job);
  }

  void
  Scheduler::resume_scheduler_suspend (Job *job)
  {
    suspended_jobs_.push_back (job);
    switch_back (job);
  }

  void
  Scheduler::resume_job (Job *job)
  {
    assert (libport::has (suspended_jobs_, job));
    jobs_.push_back (job);
    suspended_jobs_.remove (job);
  }

  void
  Scheduler::killall_jobs ()
  {
    ECHO ("killing all jobs!");

    // This implementation is quite inefficient because it will call
    // kill_job() for each job. But who cares? We are killing everyone
    // anyway.

    while (!jobs_to_start_.empty ())
      jobs_to_start_.pop_front ();

    while (!suspended_jobs_.empty ())
      kill_job (suspended_jobs_.front ());

    while (!jobs_.empty ())
      kill_job (jobs_.front ());

    while (!deferred_jobs_.empty ())
      kill_job (deferred_jobs_.top ().get<1>());
  }

  void
  Scheduler::kill_job (Job* job)
  {
    assert (job);

    ECHO ("killing job " << job);

    // First of all, tell the job it has been terminated unless it has
    // been registered with us but not yet started.
    if (!libport::has (jobs_to_start_, job))
      job->terminate_now ();

    // Remove the job from the queues where it could be stored
    jobs_to_start_.remove (job);
    jobs_.remove (job);
    suspended_jobs_.remove (job);

    // We have no remove() on a priority queue, regenerate a queue without
    // this job.
    {
      deferred_jobs old_deferred;
      std::swap (old_deferred, deferred_jobs_);
      while (!old_deferred.empty ())
      {
	deferred_job j = old_deferred.top ();
	old_deferred.pop ();
	if (j.get<1>() != job)
	  deferred_jobs_.push (j);
      }
    }

    ECHO ("deleting job " << job);
    delete job;
  }

  bool operator> (const deferred_job& left, const deferred_job& right)
  {
    return left.get<0>() > right.get<0>();
  }

} // namespace scheduler
