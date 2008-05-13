#include "runner/at_handler.hh"

#include <iostream>
#include <list>

#include "scheduler/tag.hh"

namespace runner
{

  struct AtJob
  {
    AtJob(rObject condition, rObject clause, rObject on_leave,
	  scheduler::tags_type tags) :
      condition_(condition),
      clause_(clause),
      on_leave_(on_leave),
      triggered_(false),
      tags_(tags)
      {
      }
    rObject condition_;
    rObject clause_;
    rObject on_leave_;
    bool triggered_;
    scheduler::tags_type tags_;
  };

  class AtHandler : public Runner
  {
  public:
    AtHandler(const Runner&);
    virtual ~AtHandler();
    virtual void work();
    virtual bool frozen() const;
    virtual bool blocked() const;
    void add_job(AtJob*);
  private:
    void complete_tags(const AtJob&);
    void rebuild_tags();
    std::list<AtJob*> jobs_;
    bool yielding;
    scheduler::tags_type tags_;
  };

  // There is only one at job handler. It will be created if it doesn't exist
  // and destroyed when it has no more jobs to handle.
  static AtHandler* at_job_handler;

  AtHandler::AtHandler(const Runner& model) :
    Runner(model, 0),
    yielding(false)
  {
  }

  AtHandler::~AtHandler()
  {
  }

  void
  AtHandler::work()
  {
    side_effect_free_set(true);

    while (true)
    {
      non_interruptible_set(true);

      // We have been woken up, either because we may have something to do
      // or because some tag conditions have changed.

      std::list<AtJob*> pending;
      swap(jobs_, pending);

      foreach (AtJob* job, pending)
      {
	bool blocked = false;
	bool frozen = false;
	foreach (const scheduler::rTag& tag, job->tags_)
	{
	  blocked |= tag->blocked();
	  frozen  |= tag->frozen();
	}

	// If job has been blocked or stopped, we will not keep it any longer.
	if (blocked)
	{
	  delete job;
	  continue;
	}

	// If job has been frozen, we will not consider it for the moment.
	if (frozen)
	{
	  jobs_.push_back(job);
	  continue;
	}

	// Check the job condition and continue if it has not changed.
	bool new_state;
	try
	{
	  new_state =
	    object::is_true(urbi_call(*this, job->condition_, SYMBOL(eval)));
	}
	catch (const kernel::exception& ke)
	{
	  std::cerr << "at condition triggered an exception: " << ke.what()
		    << std::endl;
	  delete job;
	  continue;
	}
	catch (...)
	{
	  std::cerr << "at condition triggered an exception\n";
	  delete job;
	  continue;
	}
	if (new_state == job->triggered_)
	{
	  jobs_.push_back(job);
	  continue;
	}

	// There has been a change in the condition, act accordingly depending
	// on whether we have seen a rising or a falling edge and save the
	// condition evaluation result.
	rObject to_launch = new_state ? job->clause_ : job->on_leave_;
	if (to_launch != object::nil_class)
	{
	  // Temporarily install the needed tags as the current tags.
	  tags_set(job->tags_);
	  // We do not need to check for an exception here as "detach",
	  // which is the function being called, will not throw and any
	  // exception thrown in the detached runner will not be caught
	  // here anyway.
	  urbi_call(*this, to_launch, SYMBOL(eval));
	}
	job->triggered_ = new_state;
	jobs_.push_back(job);
      }

      // If we have no more jobs, we can destroy ourselves.
      if (jobs_.empty())
      {
	at_job_handler = 0;
	terminate_now();
      }

      non_interruptible_set(false);

      // Go to sleep
      try
      {
	// We want to appear blocked only when explicitly yielding and
	// catching the exception. If, by mistake, a condition evaluation
	// yields and is blocked, we do not want it to get the bogus
        // exception.
	yielding = true;
	yield_until_things_changed();
	yielding = false;
      }
      catch (const scheduler::BlockedException& e)
      {
	// Ignore this exception, we will check every condition against this
	// anyway.
      }
      catch (const kernel::exception& e)
      {
	std::cerr << "at job handler exited with exception " << e.what()
		  << std::endl;
	throw;
      }
      catch (...)
      {
	std::cerr << "at job handler exited with unknown exception\n";
	throw;
      }
    }
  }

  bool
  AtHandler::frozen() const
  {
    return false;
  }

  bool
  AtHandler::blocked() const
  {
    if (!yielding)
      return false;
    foreach (const scheduler::rTag& t, tags_)
      if (t->blocked())
	return true;
    return false;
  }

  void
  AtHandler::add_job(AtJob* job)
  {
    jobs_.push_back(job);
    complete_tags(*job);
  }

  void
  AtHandler::rebuild_tags()
  {
    tags_.clear();
    foreach (const AtJob* job, jobs_)
      complete_tags(*job);
  }

  void
  AtHandler::complete_tags(const AtJob& job)
  {
    foreach (scheduler::rTag t, job.tags_)
    {
      foreach (const scheduler::rTag& u, tags_)
	if (t == u)
	  goto already_found;
      tags_.push_back(t);
    already_found:
      ;
    }
  }

  void
  register_at_job(const runner::Runner& starter,
		  rObject condition,
		  rObject clause,
		  rObject on_leave)
  {
    if (!at_job_handler)
    {
      at_job_handler = new AtHandler(starter);
      at_job_handler->start_job();
    }
    AtJob* job = new AtJob(condition,
			   clause,
			   on_leave,
			   starter.tags_get());
    at_job_handler->add_job(job);
  }

} // namespace runner
