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
    ECHO ("In Job::run for " << this);
    yield_front ();
    try {
      try {
	work ();
      }
      catch (TerminateException&)
      {
	// Normal termination requested
      }
      terminate ();
    }
    catch (kernel::exception &e)
    {
      // Signal the exception to each linked job in turn.
      jobs_type to_signal = links_;
      foreach (Job* job, to_signal)
      {
	  job->async_throw (e);
	  unlink (job);
      }
    }
    catch (...)
    {
      // Exception is lost and cannot be propagated properly
      std::cerr << "Exception caught in job " << this << ", loosing it\n";
    }

    terminate_cleanup ();
    yield ();

    // We should never go there as the scheduler will have terminated us.
    assert (false);
  }

  void
  Job::terminate_now ()
  {
    if (!terminated_)
      async_throw (TerminateException ());
  }

  void
  Job::terminate_cleanup ()
  {
    terminated_ = true;
    // Remove pending links.
    foreach (Job* job, links_)
      job->links_.remove (this);
    // Wake-up waiting jobs.
    foreach (Job* job, to_wake_up_)
      scheduler_->resume_job (job);
    to_wake_up_.clear ();
  }

  void
  Job::yield_until_terminated (Job& other)
  {
    if (!other.terminated ())
    {
      other.to_wake_up_.push_back (this);
      scheduler_->resume_scheduler_suspend (this);
    }
  }

  void
  Job::yield_until_things_changed ()
  {
    scheduler_->resume_scheduler_things_changed (this);
  }

  bool
  Job::frozen () const
  {
    foreach (rTag tag, tags_)
      if (tag->frozen ())
	return true;
    return false;
  }

  bool
  Job::blocked () const
  {
    foreach (rTag tag, tags_)
      if (tag->blocked ())
	return true;
    return false;
  }

  void
  Job::push_tag (rTag tag)
  {
    tags_.push_back (tag);
  }

  void
  Job::pop_tag ()
  {
    tags_.pop_back ();
  }

  void
  Job::copy_tags (const Job& other)
  {
    tags_ = other.tags_;
  }

}
