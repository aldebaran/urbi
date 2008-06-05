#include <runner/at-handler.hh>

#include <iostream>
#include <list>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_list.hpp>

#include <libport/finally.hh>

#include <scheduler/tag.hh>

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
    bool tag_held(const scheduler::rTag& tag) const;
  private:
    rObject condition_;
    rObject clause_;
    rObject on_leave_;
    bool triggered_;
    scheduler::tags_type tags_;
  };

  class AtHandler : public Interpreter
  {
    typedef Interpreter super_type;
  public:
    AtHandler(const Interpreter&);
    virtual ~AtHandler();
    virtual void work();
    virtual bool frozen() const;
    void add_job(AtJob*);
    virtual void register_stopped_tag(const scheduler::rTag& tag);
  private:
    typedef boost::ptr_list<AtJob> at_jobs_type;
    at_jobs_type jobs_;
  };

  // There is only one at job handler. It will be created if it doesn't exist
  // and destroyed when it has no more jobs to handle.
  static AtHandler* at_job_handler;

  AtHandler::AtHandler(const Interpreter& model)
    : Interpreter(model,
		  object::rObject(0),
		  SYMBOL(LT_at_SP_jobs_SP_handler_GT))
  {
    // There are no reason to inherit tags from our creator, as this service
    // is not tied to any particular connection.
    tags_.clear();
  }

  AtHandler::~AtHandler()
  {
    jobs_.release();
  }

  void
  AtHandler::register_stopped_tag(const scheduler::rTag& tag)
  {
    // Remove all the jobs holding this tag.
    jobs_.erase_if(boost::bind(&AtJob::tag_held, _1, tag));
  }

  void
  AtHandler::work()
  {
    while (true)
    {
      non_interruptible_set(true);

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
	  job = jobs_.erase(job);
	  continue;
	}
	catch (const kernel::exception& ke)
	{
	  std::cerr << "at condition triggered an exception: " << ke.what()
		    << std::endl;
	  job = jobs_.erase(job);
	  continue;
	}
	catch (...)
	{
	  std::cerr << "at condition triggered an exception\n";
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
	  tags_ = job->tags_get();
	  libport::Finally finally(boost::bind(&AtHandler::tags_set,
					       this,
					       tags_));

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

      // Go to sleep
      try
      {
	non_interruptible_set(false);
	side_effect_free_set(true);
	yield_until_things_changed();
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

  void
  AtHandler::add_job(AtJob* job)
  {
    jobs_.push_back(job);
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
  AtJob::tag_held(const scheduler::rTag& tag) const
  {
    foreach (const scheduler::rTag& t, tags_)
      if (t == tag)
	return true;
    return false;
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
