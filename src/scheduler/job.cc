#include <iostream>

#include <boost/mem_fn.hpp>

#include <libport/compiler.hh>     // For ECHO
#include <libport/containers.hh>
#include <libport/foreach.hh>

#include <scheduler/job.hh>

namespace scheduler
{
  StopException::StopException(int depth, boost::any payload)
    : depth_(depth)
    , payload_(payload)
  {
  }

  void
  Job::run()
  {
    assert(state_ == to_start);
    ECHO("In Job::run for " << this);
    try {
      // The first yield, performed just after the job structure has
      // been setup here by calling run(), has to be done from within
      // the exception handler as this job may be killed by a "stop"
      // or "block" of a tag that has been inherited at job creation
      // time.
      yield();
      work();
    }
    catch (TerminateException&)
    {
      // Normal termination requested
    }
    catch (StopException&)
    {
      // Termination through "stop" or "block" on a top-level tag,
      // that is a tag inherited at the job creation time.
    }
    catch (const kernel::exception& e)
    {
      // Signal the exception to each linked job in turn.
      foreach (rJob job, links_)
      {
	job->links_.remove(this);
	job->async_throw(e);
      }
    }
    catch (...)
    {
      // Exception is lost and cannot be propagated properly
      std::cerr << "Exception caught in job " << this << ", loosing it\n";
    }

    terminate_cleanup();

    // We should never go there as the scheduler will have terminated us.
    assert(false);
  }

  void
  Job::terminate_now()
  {
    if (!terminated())
      async_throw(TerminateException());
  }

  void
  Job::terminate_cleanup()
  {
    // Remove pending links.
    foreach (rJob job, links_)
      job->links_.remove(this);
    links_.clear();
    // Wake-up waiting jobs.
    foreach (rJob job, to_wake_up_)
      if (!job->terminated())
	job->state_set(running);
    to_wake_up_.clear();
    state_ = zombie;
    scheduler_.resume_scheduler(this);
  }

  void
  Job::yield_until_terminated(Job& other)
  {
    if (non_interruptible_)
      throw object::SchedulingError
	("dependency on other task in non-interruptible code");

    if (!other.terminated())
    {
      other.to_wake_up_.push_back(this);
      state_ = joining;
      scheduler_.resume_scheduler(this);
    }
  }

  void
  Job::yield_until_terminated(jobs_type jobs)
  {
    foreach (rJob job, jobs)
      yield_until_terminated(*job);
  }

  void
  Job::yield_until_things_changed()
  {
    if (non_interruptible_ && !frozen())
      throw object::SchedulingError
	("attempt to wait for condition changes in non-interruptible code");

    state_ = waiting;
    scheduler_.resume_scheduler(this);
  }

  bool
  Job::frozen() const
  {
    return libport::has_if(tags_, boost::mem_fn(&scheduler::Tag::frozen));
  }

  void
  Job::async_throw(const kernel::exception& e)
  {
    pending_exception_ = e.clone();
    // A job which has received an exception is no longer side effect
    // free or non-interruptible.
    side_effect_free_ = false;
    non_interruptible_ = false;
    // If this is the current job we are talking about, the exception
    // is synchronous.
    if (scheduler_.is_current_job(*this))
      check_for_pending_exception();
    // Now that we acquired an exception to raise, we are active again,
    // even if we were previously sleeping or waiting for something.
    if (state_ != to_start && state_ != zombie)
      state_ = running;
  }

  void
  Job::register_stopped_tag(const rTag& tag, boost::any payload)
  {
    int max_tag_check = tags_.size();
    if (pending_exception_)
    {
      // If we are going to terminate, do nothing
      if (dynamic_cast<TerminateException*>(pending_exception_.get()))
	return;
      // If we already have a StopException stored, do not go any
      // further.
      StopException* exc =
	dynamic_cast<StopException*>(pending_exception_.get());
      if (exc)
	max_tag_check = exc->depth_get();
    }

    // Check if we are affected by this tag, up-to max_tag_check from
    // the beginning of the tag list.
    for (int i = 0; i < max_tag_check; i++)
      if (tags_[i]->derives_from(*tag))
      {
	async_throw(StopException(i, payload));
	return;
      }
  }

  void
  Job::check_for_pending_exception()
  {
    // If an exception has been stored for further rethrow, now is
    // a good time to do so.
    if (pending_exception_)
    {
      current_exception_ = pending_exception_;
      pending_exception_ = 0;
      kernel::rethrow(current_exception_);
    }
  }

}
