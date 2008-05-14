#include <iostream>

#include <libport/compiler.hh>     // For ECHO
#include <libport/foreach.hh>

#include "job.hh"

namespace scheduler
{
  struct TerminateException : public SchedulerException
  {
    virtual std::string
    what() const throw ()
    {
      return "TerminateException";
    }

    COMPLETE_EXCEPTION (TerminateException)
  };

  void
  Job::run ()
  {
    assert (state_ == to_start);
    ECHO ("In Job::run for " << this);
    try {
      // The first yield, performed just after the job structure has
      // been setup here by calling run(), has to be done from within
      // the exception handler as this job may be killed by a "stop"
      // or "block" of a tag that has been inherited at job creation
      // time.
      try {
	yield ();
	work ();
      }
      catch (TerminateException&)
      {
	// Normal termination requested
      }
      catch (BlockedException&)
      {
	// Termination through "stop" or "block" on a top-level tag,
	// that is a tag inherited at the job creation time.
      }
      terminate ();
    }
    catch (const kernel::exception& e)
    {
      // Signal the exception to each linked job in turn.
      jobs_type to_signal = links_;
      foreach (Job* job, to_signal)
      {
	  unlink (job);
	  job->async_throw (e);
      }
    }
    catch (...)
    {
      // Exception is lost and cannot be propagated properly
      std::cerr << "Exception caught in job " << this << ", loosing it\n";
    }

    terminate_cleanup ();

    // We should never go there as the scheduler will have terminated us.
    assert (false);
  }

  void
  Job::terminate_now ()
  {
    if (!terminated ())
      async_throw (TerminateException ());
  }

  void
  Job::terminate_cleanup ()
  {
    // Remove pending links.
    foreach (Job* job, links_)
      job->links_.remove (this);
    // Wake-up waiting jobs.
    foreach (Job* job, to_wake_up_)
      job->state_set (running);
    to_wake_up_.clear ();
    // Return to the scheduler and release the possibly sole reference.
    scheduler_->take_job_reference (myself_);
    state_ = zombie;
    scheduler_->resume_scheduler (this);
  }

  void
  Job::yield_until_terminated (Job& other)
  {
    if (non_interruptible_)
      throw object::SchedulingError
	("dependency on other task in non-interruptible code");

    if (!other.terminated ())
    {
      other.to_wake_up_.push_back (this);
      state_ = joining;
      scheduler_->resume_scheduler (this);
    }
  }

  void
  Job::yield_until_things_changed ()
  {
    if (non_interruptible_ && !frozen ())
      throw object::SchedulingError
	("attempt to wait for condition changes in non-interruptible code");

    state_ = waiting;
    scheduler_->resume_scheduler (this);
  }

  bool
  Job::frozen () const
  {
    foreach (const rTag& tag, tags_)
      if (tag->frozen ())
	return true;
    return false;
  }

  bool
  Job::blocked () const
  {
    foreach (const rTag& tag, tags_)
      if (tag->blocked ())
	return true;
    return false;
  }

  void
  Job::async_throw (const kernel::exception& e)
  {
    pending_exception_ = e.clone ();
    // Now that we acquired an exception to raise, we are active again,
    // even if we were previously sleeping or waiting for something.
    if (state_ != to_start && state_ != zombie)
      state_ = running;
  }

  void
  Job::check_for_pending_exception ()
  {
    // If we have been blocked, try to get up the chain. No need to
    // handle another exception here.
    if (blocked ())
    {
      pending_exception_ = 0;
      side_effect_free_ = false;
      throw BlockedException ();
    }

    // If an exception has been stored for further rethrow, now is
    // a good time to do so.
    if (pending_exception_)
    {
      current_exception_ = pending_exception_;
      pending_exception_ = 0;
      // If an exception is propagated, it may have side effects.
      side_effect_free_ = false;
      kernel::rethrow (current_exception_);
    }
  }

}
