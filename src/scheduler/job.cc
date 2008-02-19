#include <iostream>

#include <libport/compiler.hh>     // For ECHO
#include <libport/foreach.hh>

#include "job.hh"

namespace scheduler
{
  void
  Job::run ()
  {
    ECHO ("In Job::run for " << this);
    yield_front ();
    try {
      work ();
      terminate ();
    }
    catch (boost::exception &e)
    {
      // Signal the exception to each linked job in turn.
      boost::exception_ptr ep = boost::clone_exception (e);
      jobs to_signal = links_;
      foreach (Job* job, to_signal)
      {
	  job->async_throw (ep);
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
    {
      terminate ();
      terminate_cleanup ();
    }
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

}
