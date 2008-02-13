/**
 ** \file scheduler/scheduler.cc
 ** \brief Implementation of scheduler::Scheduler.
 */

//#define ENABLE_DEBUG_TRACES

#include <cassert>

#include <libport/compiler.hh>
#include <libport/containers.hh>
#include <libport/foreach.hh>

#include "kernel/userver.hh"

#include "object/urbi-exception.hh"

#include "scheduler/scheduler.hh"
#include "scheduler/job.hh"

namespace scheduler
{

  // This function is required to start a new job using the libcoroutine.
  // Its only purpose is to create the context and start the execution
  // of the new job.
  static void
  run_job (void* job)
  {
    static_cast<Job*>(job)->run();
  }

  void
  Scheduler::add_job (Job* job)
  {
    assert (job);
    assert (!libport::has (jobs_, job));
    assert (!libport::has (jobs_to_start_, job));
    jobs_to_start_.push_back (job);
  }

  libport::utime_t
  Scheduler::work ()
  {
#ifdef ENABLE_DEBUG_TRACES
    static int cycle = 0;
#endif
    ECHO ("======================================================== cycle "
	  << ++cycle);

    // Start new jobs
    to_start_.clear ();
    std::swap (to_start_, jobs_to_start_);
    foreach (Job* job, to_start_)
    {
      assert (job);
      ECHO ("will start job " << job);
      // Job will start for a very short time and do a yield_front() to
      // be restarted below in the course of the regular cycle.
      assert (!current_job_);
      current_job_ = job;
      Coro_startCoro_ (self_, job->coro_get(), job, run_job);
      // We have to assume that a new job may have had side-effects.
      possible_side_effect_ = true;
    }

    // Start deferred jobs
    for (libport::utime_t current_time = ::urbiserver->getTime ();
	 !deferred_jobs_.empty();
	 deferred_jobs_.pop ())
    {
      deferred_job j = deferred_jobs_.top ();
      if (current_time < j.get<0>())
	break;
      jobs_.push_back (j.get<1> ());
    }

    // If something is going to happen, or if we have just started a
    // new job, add the list of jobs waiting for something to happen
    // on the pending jobs queue.
    if (!if_change_jobs_.empty() && (possible_side_effect_ || !jobs_.empty()))
    {
      foreach (Job* job, if_change_jobs_)
	jobs_.push_back (job);
      if_change_jobs_.clear ();
    }

    // Run all the jobs in the run queue once. If any job declares upon entry or
    // return that it is not side-effect free, we remember that for the next
    // cycle.
    possible_side_effect_ = false;
    pending_.clear ();
    std::swap (pending_, jobs_);

    ECHO (pending_.size() << " jobs in the queue for this cycle");
    foreach (Job* job, pending_)
    {
      assert (job);
      assert (!job->terminated ());
      ECHO ("will resume job " << job);
      possible_side_effect_ |= !job->side_effect_free_get ();
      Coro_switchTo_ (self_, job->coro_get ());
      possible_side_effect_ |= !job->side_effect_free_get ();
      ECHO ("back from job " << job);
    }

    // Do we have some work to do now?
    if (!jobs_.empty ())
      return 0;

    // Do we have deferred jobs?
    if (!deferred_jobs_.empty ())
      return deferred_jobs_.top ().get<0> ();

    // Ok, let's say, we'll be called again in one hour.
    return ::urbiserver->getTime() + 3600000000LL;
  }

  void
  Scheduler::switch_back (Job* job)
  {
    // Switch back to the scheduler.
    assert (current_job_ == job);
    current_job_ = 0;
    Coro_switchTo_ (job->coro_get (), self_);
    // We regained control, we are again in the context of the job.
    assert (!current_job_);
    current_job_ = job;
    ECHO ("job " << job << " resumed");
    // Check that we are not near exhausting the stack space.
    if (Coro_stackSpaceAlmostGone (job->coro_get ()))
      throw object::StackExhaustedError ("stack space exhausted");
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
  Scheduler::resume_scheduler_front (Job* job)
  {
    // Put the job in front of the queue. If the job asks to be requeued,
    // it is not terminated.
    assert (!job->terminated ());
    jobs_.push_front (job);
    switch_back (job);
  }

  void
  Scheduler::resume_scheduler_until (Job* job, libport::utime_t deadline)
  {
    // Put the job in the deferred queue. If the job asks to be requeued,
    // it is not terminated.
    assert (!job->terminated ());
    deferred_jobs_.push (boost::make_tuple (deadline, job));
    switch_back (job);
  }

  void
  Scheduler::resume_scheduler_suspend (Job* job)
  {
    suspended_jobs_.push_back (job);
    switch_back (job);
  }

  void
  Scheduler::resume_scheduler_things_changed (Job* job)
  {
    if_change_jobs_.push_back (job);
    switch_back (job);
  }

  void
  Scheduler::resume_job (Job* job)
  {
    // Suspended job may have been killed externally, in which case it
    // will not appear in the list of suspended jobs.

    if (libport::has (suspended_jobs_, job))
    {
      jobs_.push_back (job);
      suspended_jobs_.remove (job);
    }
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

    while (!if_change_jobs_.empty ())
      kill_job (if_change_jobs_.front ());

    while (!jobs_.empty ())
      kill_job (jobs_.front ());

    while (!deferred_jobs_.empty ())
      kill_job (deferred_jobs_.top ().get<1>());
  }

  void
  Scheduler::unschedule_job (Job* job)
  {
    assert (job);
    assert (job != current_job_);

    ECHO ("unscheduling job " << job);

    // First of all, tell the job it has been terminated unless it has
    // been registered with us but not yet started.
    if (!libport::has (jobs_to_start_, job))
      job->terminate_now ();

    // Remove the job from the queues where it could be stored.
    jobs_to_start_.remove (job);
    jobs_.remove (job);
    suspended_jobs_.remove (job);
    if_change_jobs_.remove (job);

    // Remove it from live queues as well if the job is destroyed.
    to_start_.remove (job);
    pending_.remove (job);

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
  }

  void
  Scheduler::kill_job (Job* job)
  {
    assert (job != current_job_);

    ECHO ("deleting job " << job);
    delete job;
  }

  bool operator> (const deferred_job& left, const deferred_job& right)
  {
    return left.get<0>() > right.get<0>();
  }

} // namespace scheduler
