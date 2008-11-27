#include <cstdlib>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/mem_fn.hpp>

#include <libport/compiler.hh>     // For ECHO
#include <libport/containers.hh>
#include <libport/foreach.hh>

#include <scheduler/job.hh>

namespace scheduler
{
  unsigned int Job::alive_jobs_;

  StopException::StopException(unsigned depth, boost::any payload)
    : depth_(depth)
    , payload_(payload)
  {
  }

  void
  Job::run()
  {
    assert(state_ == to_start);
    ECHO("In Job::run for " << this);

    // We may get interrupted during our first run, in which case
    // we better not be in the to_start state while we are executing
    // or we would get removed abruptly from the scheduler pending_
    // list.
    state_ = running;
    try {
      if (has_pending_exception() &&
	  dynamic_cast<SchedulerException*>(pending_exception_.get()))
	check_for_pending_exception();
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
    catch (const exception& e)
    {
      // Rethrow the exception into the parent job if it exists.
      if (parent_)
	parent_->async_throw(ChildException(e.clone()));
    }
    catch (const std::exception& se)
    {
      // Exception is lost and cannot be propagated properly but can be
      // printed onto the console for diagnostic purpose.
      std::cerr << "Exception `" << se.what() << "' caught in job "
		<< this << ", loosing it\n";
    }
    catch (...)
    {
      // Exception is lost and cannot be propagated properly.
      std::cerr << "Exception caught in job " << this << ", loosing it\n";
    }

    terminate_cleanup();

    // We should never go there as the scheduler will have terminated us.
    pabort("scheduler did not terminate us");
  }

  void
  Job::terminate_now()
  {
    // We have to terminate our children as well.
    foreach (const rJob& child, children_)
      child->terminate_now();
    children_.clear();
    if (!terminated())
      async_throw(TerminateException());
  }

  void
  Job::terminate_child(const rJob& child)
  {
    child->terminate_now();
    libport::erase_if(children_, boost::lambda::_1 == child);
  }

  void
  Job::terminate_cleanup()
  {
    // Wake-up waiting jobs.
    foreach (const rJob& job, to_wake_up_)
      if (!job->terminated())
	job->state_set(running);
    to_wake_up_.clear();
    state_ = zombie;
    scheduler_.resume_scheduler(this);
  }

  void
  Job::register_child(const rJob& child, libport::Finally& at_end)
  {
    assert(!child->parent_);
    child->parent_ = this;
    at_end << boost::bind(&Job::terminate_child, this, child);
    children_.push_back(child);
  }

  void
  Job::yield_until_terminated(Job& other)
  {
    if (non_interruptible_ && this != &other)
      scheduling_error
	("dependency on other task in non-interruptible code");

    if (!other.terminated())
    {
      // We allow enqueuing on ourselves, but without doing it for real.
      if (&other != this)
	other.to_wake_up_.push_back(this);
      state_ = joining;
      try
      {
	scheduler_.resume_scheduler(this);
      }
      catch (...)
      {
	// We have been awoken by an exception; in this case,
	// dequeue ourselves from the other thread queue if
	// we are still enqueued there.
	libport::erase_if(other.to_wake_up_, boost::lambda::_1 == this);
	throw;
      }
    }
  }

  void
  Job::yield_until_terminated(const jobs_type& jobs)
  {
    foreach (const rJob& job, jobs)
      yield_until_terminated(*job);
  }

  void
  Job::yield_until_things_changed()
  {
    if (non_interruptible_ && !frozen())
      scheduling_error
	("attempt to wait for condition changes in non-interruptible code");

    state_ = waiting;
    scheduler_.resume_scheduler(this);
  }

  void
  Job::async_throw(const exception& e)
  {
    // A job which has received an exception is no longer side effect
    // free or non-interruptible.
    side_effect_free_ = false;
    non_interruptible_ = false;
    // If this is the current job we are talking about, the exception
    // is synchronous.
    if (scheduler_.is_current_job(*this))
      e.rethrow();

    // Store the exception for later use.
    pending_exception_ = e.clone();

    // Now that we acquired an exception to raise, we are active again,
    // even if we were previously sleeping or waiting for something.
    if (state_ != to_start && state_ != zombie)
      state_ = running;
  }

  void
  Job::register_stopped_tag(const Tag& tag, const boost::any& payload)
  {
    size_t max_tag_check = (size_t)-1;
    if (has_pending_exception())
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
    if (size_t pos = has_tag(tag, max_tag_check))
      async_throw(StopException(pos - 1, payload));
  }

  void
  Job::check_for_pending_exception()
  {
    // If an exception has been stored for further rethrow, now is
    // a good time to do so.
    if (has_pending_exception())
    {
      // Reset pending_exception_ by copying it into a local variable
      // first. This ensures that the exception will be destroyed
      // properly after having been rethrown.
      exception_ptr e = pending_exception_;
      e->rethrow();
    }
  }

  unsigned int
  Job::alive_jobs()
  {
    return alive_jobs_;
  }

  void
  Job::scheduling_error(const std::string& s)
  {
    pabort(s);
  }

  void
  terminate_jobs(jobs_type& jobs)
  {
    foreach (rJob& job, jobs)
      job->terminate_now();
    jobs.clear();
  }

} // namespace scheduler
