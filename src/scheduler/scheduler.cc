/**
 ** \file scheduler/scheduler.cc
 ** \brief Implementation of scheduler::Scheduler.
 */

//#define ENABLE_DEBUG_TRACES

#include <cassert>
#include <cstdlib>

#include <libport/compiler.hh>
#include <libport/containers.hh>
#include <libport/contract.hh>
#include <libport/foreach.hh>

#include <object/urbi-exception.hh>

#include <scheduler/scheduler.hh>
#include <scheduler/job.hh>

namespace scheduler
{

  // This function is required to start a new job using the libcoroutine.
  // Its only purpose is to create the context and start the execution
  // of the new job.
  static void
  run_job(Job* job)
  {
    job->run();
  }

  void
  Scheduler::add_job(rJob job)
  {
    assert(job);
    assert(!libport::has(jobs_, job));
    jobs_.push_back(job);
    jobs_to_start_ = true;
  }

  libport::utime_t
  Scheduler::work()
  {
    ++cycle_;
    ECHO("======================================================== cycle "
	  << cycle_);

    libport::utime_t deadline = execute_round();

#ifdef ENABLE_DEBUG_TRACES
    if (deadline)
      ECHO("Scheduler asking to be woken up in "
	    << (deadline - get_time_()) / 1000000L << " seconds");
    else
      ECHO("Scheduler asking to be woken up ASAP");
#endif

    return deadline;
  }

  libport::utime_t
  Scheduler::execute_round()
  {
    // Run all the jobs in the run queue once. If any job declares upon entry or
    // return that it is not side-effect free, we remember that for the next
    // cycle.
    pending_.clear();
    std::swap(pending_, jobs_);

    // By default, wake us up after one hour and consider that we have no
    // new job to start. Also, run waiting jobs only if the previous round
    // may have add a side effect and reset this indication for the current
    // job.
    libport::utime_t deadline = get_time_() +  3600000000LL;
    jobs_to_start_ = false;
    bool start_waiting = possible_side_effect_;
    possible_side_effect_ = false;

    ECHO(pending_.size() << " jobs in the queue for this round");
    foreach (rJob job, pending_)
    {
      // If the job has terminated during the previous round, remove the
      // references we have on it by just skipping it.
      if (job->terminated())
	continue;

      // Should the job be started?
      bool start = false;

      // Save the current time since we will use it several times during this job
      // analysis.
      libport::utime_t current_time = get_time_();

      ECHO("Considering " << *job << " in state " << state_name(job->state_get()));

      switch (job->state_get())
      {
      case to_start:
      {
	// New job. Start its coroutine but do not start the job as it would be queued
	// twice otherwise. It will start doing real work at the next cycle, so set
	// deadline to 0. Note that we use "continue" here to avoid having the job
	// requeued because it hasn't been started by setting "start".
	//
	// The code below takes care of destroying the rJob reference to the job, so
	// that it does not stay in the call stack as a local variable. If it did,
	// the job would never be destroyed. However, to prevent the job from being
	// prematurely destroyed, we set current_job_ (global to the scheduler) to
	// the rJob.
	ECHO("Starting job " << *job);
	current_job_ = job;
	ECHO("Job " << *job << " is starting");
	job = 0;
	coroutine_start(coro_, current_job_->coro_get(), run_job, current_job_.get());
	current_job_ = 0;
	deadline = SCHED_IMMEDIATE;
	continue;
      }
      case zombie:
	assert(false);
	break;
      case running:
	start = true;
	break;
      case sleeping:
	{
	  libport::utime_t job_deadline = job->deadline_get();

	  // If the job has been frozen, extend its deadline by the
	  // corresponding amount of time. The deadline will be adjusted
	  // later when the job is unfrozen using notice_not_frozen(),
	  // but in the meantime we do not want to cause an early wakeup.
	  if (libport::utime_t frozen_since = job->frozen_since_get())
	    job_deadline += current_time - frozen_since;

	  if (job_deadline <= current_time)
	    start = true;
	  else
	    deadline = std::min(deadline, job_deadline);
	}
	break;
      case waiting:
	// Since jobs keep their orders in the queue, start waiting jobs if
	// previous jobs in the run have had a possible side effect or if
	// the previous run may have had some. Without it, we may miss some
	// changes if the watching job is after the modifying job in the queue
	// and the watched condition gets true for only one cycle.
	start = start_waiting || possible_side_effect_;
	break;
      case joining:
	break;
      }

      // Tell the job whether it is frozen or not so that it can remember
      // since when it has been in this state.
      if (job->frozen())
	job->notice_frozen(current_time);
      else
	job->notice_not_frozen(current_time);

      // A job with an exception will start unconditionally.
      if (start || job->has_pending_exception())
      {
	ECHO("will resume job " << *job
	      << (job->side_effect_free_get() ? " (side-effect free)" : ""));
	possible_side_effect_ |= !job->side_effect_free_get();
	assert(!current_job_);
	coroutine_switch_to(coro_, job->coro_get());
	assert(!current_job_);
	possible_side_effect_ |= !job->side_effect_free_get();
	ECHO("back from job " << *job
	      << (job->side_effect_free_get() ? " (side-effect free)" : ""));
	switch (job->state_get())
	{
	case running:
	  deadline = SCHED_IMMEDIATE;
	  break;
	case sleeping:
	  deadline = std::min(deadline, job->deadline_get());
	  break;
	default:
	  break;
	}
      }
      else
	jobs_.push_back(job);   // Job not started, keep it in queue
    }

    /// If during this cycle a new job has been created by an existing job,
    /// start it. Also start if a possible side effect happened, it may have
    /// occurred later then the waiting jobs in the cycle.
    if (jobs_to_start_ || possible_side_effect_)
      deadline = SCHED_IMMEDIATE;

    /// If we are ready to die and there are no jobs left, then die.
    if (ready_to_die_ && jobs_.empty())
      deadline = SCHED_EXIT;

    return deadline;
  }

  void
  Scheduler::resume_scheduler(rJob job)
  {
    // If the job has not terminated and is side-effect free, then we
    // assume it will not take a long time as we are probably evaluating
    // a condition. In order to reduce the number of cycles spent to evaluate
    // the condition, continue until it asks to be suspended in another
    // way or until it is no longer side-effect free.

    bool side_effect_free_save = job->side_effect_free_get();

    if (job->state_get() == running && side_effect_free_save)
      return;

    // We may have to suspend the job several time in case it makes no sense
    // to start it back. Let's do it in a loop and we'll break when we want
    // to resume the job.

    while (true)
    {
      // Add the job at the end of the scheduler queue unless the job has
      // already terminated.
      if (!job->terminated())
	jobs_.push_back(job);

      // Switch back to the scheduler. But in the case this job has been
      // destroyed, erase the local variable first so that it doesn't keep
      // a reference on it which will never be destroyed.
      assert(current_job_ == job);
      ECHO(*job << " has " << (job->terminated() ? "" : "not ") << "terminated\n\t"
	   << "state: " << state_name(job->state_get()) << std::endl);
      Coro* current_coro = job->coro_get();
      if (job->terminated())
	job = 0;
      current_job_ = 0;
      coroutine_switch_to(current_coro, coro_);

      // If we regain control, we are not dead.
      assert(job);

      // We regained control, we are again in the context of the job.
      assert(!current_job_);
      current_job_ = job;
      ECHO("job " << *job << " resumed");

      // Execute a deferred exception if any; this may break out of this loop
      job->check_for_pending_exception();

      // If we are not frozen, it is time to resume regular execution
      if (!job->frozen())
	break;

      // Ok, we are frozen. Let's requeue ourselves after setting
      // the side_effect_free flag, and we will be in waiting mode.
      job->side_effect_free_set(true);
      job->state_set(waiting);
    }

    // Check that we are not near exhausting the stack space.
    job->check_stack_space();

    // Restore the side_effect_free flag
    job->side_effect_free_set(side_effect_free_save);

    // Resume job execution
  }

  void
  Scheduler::killall_jobs()
  {
    ECHO("killing all jobs!");

    // Mark the scheduler as ready to die when all the jobs are
    // really dead.
    ready_to_die_ = true;

    // Since killing the current job will result in its immediate
    // termination, kill all other jobs before.
    foreach (rJob job, jobs_get())
      if (job != current_job_)
	job->terminate_now();
    if (current_job_)
      current_job_->terminate_now();
  }

  void
  Scheduler::signal_stop(const rTag& tag, boost::any payload)
  {
    // Tell the jobs that a tag has been stopped, ending with
    // the current job to avoid interrupting this method early.
    foreach (rJob job, jobs_get())
      if (job != current_job_)
	job->register_stopped_tag(tag, payload);
    if (current_job_)
      current_job_->register_stopped_tag(tag, payload);
  }

  std::vector<rJob>
  Scheduler::jobs_get() const
  {
    // If this method is called from within a job, return the currently
    // executing jobs (pending_), otherwise return the jobs_ content which
    // is complete.
    return current_job_ ? pending_ : jobs_;
  }

} // namespace scheduler
