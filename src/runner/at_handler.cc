#include "runner/at_handler.hh"

#include <iostream>
#include <list>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_list.hpp>

#include <libport/finally.hh>

#include "scheduler/tag.hh"

namespace runner
{

  class AtJob
  {
  public:
    AtJob(rObject condition, rObject clause, rObject on_leave,
	  scheduler::tags_type tags);
    bool blocked() const;
    bool frozen() const;
    const rObject& condition_get() const;
    const rObject& clause_get() const;
    const rObject& on_leave_get() const;
    bool triggered_get() const;
    void triggered_set(bool);
    const scheduler::tags_type& tags_get() const;
  private:
    rObject condition_;
    rObject clause_;
    rObject on_leave_;
    bool triggered_;
    scheduler::tags_type tags_;
  };

  class AtHandler : public Interpreter
  {
  public:
    AtHandler(const Interpreter&);
    virtual ~AtHandler();
    virtual void work();
    virtual bool frozen() const;
    virtual bool blocked() const;
    void add_job(AtJob*);
  private:
    void complete_tags(const AtJob&);
    void rebuild_tags();
    typedef boost::ptr_list<AtJob> at_jobs_type;
    at_jobs_type jobs_;
    bool yielding;
    scheduler::tags_type tags_;
  };

  // There is only one at job handler. It will be created if it doesn't exist
  // and destroyed when it has no more jobs to handle.
  static AtHandler* at_job_handler;

  AtHandler::AtHandler(const Interpreter& model)
    : Interpreter(model,
		  object::rObject(0),
		  libport::Symbol("<at jobs handler>")),
      yielding(false)
  {
  }

  AtHandler::~AtHandler()
  {
    jobs_.release();
  }

  void
  AtHandler::work()
  {
    bool tags_need_rebuilding = false;

    while (true)
    {
      non_interruptible_set(true);
      yielding = false;

      // If we have to check for blocked jobs, do it at the beginning
      // to make sure we do not miss a "stop" because some condition
      // evaluation mistakenly reenters the scheduler. We know that we
      // have to check for blocked jobs because we have had an indication
      // that tags needed to be rebuilt.

      if (tags_need_rebuilding)
	jobs_.erase_if(boost::bind(&AtJob::blocked, _1));

      for (at_jobs_type::iterator job = jobs_.begin();
	   job != jobs_.end();
	   /* Do not increment as we will also use erase() to advance */)
      {
	// If job has been frozen, we will not consider it for the moment.
	if (job->frozen())
	{
	  ++job;
	  continue;
	}

	// Check the job condition and continue if it has not changed.
	bool new_state;
	try
	{
	  new_state =
	    object::is_true(urbi_call(*this, job->condition_get(), SYMBOL(eval)));
	}
	catch (const object::UrbiException& ue)
	{
	  show_error_(ue);
	  tags_need_rebuilding = true;
	  job = jobs_.erase(job);
	  continue;
	}
	catch (const kernel::exception& ke)
	{
	  std::cerr << "at condition triggered an exception: " << ke.what()
		    << std::endl;
	  tags_need_rebuilding = true;
	  job = jobs_.erase(job);
	  continue;
	}
	catch (...)
	{
	  std::cerr << "at condition triggered an exception\n";
	  tags_need_rebuilding = true;
	  job = jobs_.erase(job);
	  continue;
	}
	if (new_state == job->triggered_get())
	{
	  ++job;
	  continue;
	}

	// There has been a change in the condition, act accordingly depending
	// on whether we have seen a rising or a falling edge and save the
	// condition evaluation result.
	const rObject& to_launch =
	  new_state ? job->clause_get() : job->on_leave_get();
	if (to_launch != object::nil_class)
	{
	  // Temporarily install the needed tags as the current tags.
	  tags_set(job->tags_get());
	  libport::Finally finally(boost::bind(&AtHandler::tags_set,
					       this,
					       tags_get()));

	  // We do not need to check for an exception here as "detach",
	  // which is the function being called, will not throw and any
	  // exception thrown in the detached runner will not be caught
	  // here anyway.
	  urbi_call(*this, to_launch, SYMBOL(eval));
	}
	job->triggered_set(new_state);
	++job;
      }

      // If we have no more jobs, we can destroy ourselves.
      if (jobs_.empty())
      {
	at_job_handler = 0;
	terminate_now();
      }

      non_interruptible_set(false);
      yielding = true;

      // Rebuild tags if our list of jobs has changed.
      if (tags_need_rebuilding)
	rebuild_tags();
      tags_need_rebuilding = false;

      // Go to sleep
      try
      {
	// We want to appear blocked only when explicitly yielding and
	// catching the exception. If, by mistake, a condition evaluation
	// yields and is blocked, we do not want it to get the bogus
        // exception.
	side_effect_free_set(true);
	yield_until_things_changed();
      }
      catch (const scheduler::BlockedException& e)
      {
	// We have at least one "at" job which needs to be blocked.
	tags_need_rebuilding = true;
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
    foreach (const AtJob& job, jobs_)
      complete_tags(job);
  }

  void
  AtHandler::complete_tags(const AtJob& job)
  {
    foreach (scheduler::rTag t, job.tags_get())
    {
      foreach (const scheduler::rTag& u, tags_)
	if (t == u)
	  goto already_found;
      tags_.push_back(t);
    already_found:
      ;
    }
  }

  AtJob::AtJob(rObject condition, rObject clause, rObject on_leave,
	       scheduler::tags_type tags)
    : condition_(condition),
      clause_(clause),
      on_leave_(on_leave),
      triggered_(false),
      tags_(tags)
  {
  }

  bool
  AtJob::blocked() const
  {
    foreach(const scheduler::rTag& tag, tags_)
      if (tag->blocked())
	return true;
    return false;
  }

  bool
  AtJob::frozen() const
  {
    foreach(const scheduler::rTag& tag, tags_)
      if (tag->frozen())
	return true;
    return false;
  }

  const rObject&
  AtJob::condition_get() const
  {
    return condition_;
  }

  const rObject&
  AtJob::clause_get() const
  {
    return clause_;
  }

  const rObject&
  AtJob::on_leave_get() const
  {
    return on_leave_;
  }

  bool
  AtJob::triggered_get() const
  {
    return triggered_;
  }

  void
  AtJob::triggered_set(bool t)
  {
    triggered_ = t;
  }

  const scheduler::tags_type&
  AtJob::tags_get() const
  {
    return tags_;
  }

  void
  register_at_job(const runner::Interpreter& starter,
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
