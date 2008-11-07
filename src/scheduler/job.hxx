/**
 ** \file scheduler/job.hxx
 ** \brief Inline implementation of Job.
 */

#ifndef SCHEDULER_JOB_HXX
# define SCHEDULER_JOB_HXX

# include <cassert>

# include <boost/bind.hpp>

# include <scheduler/scheduler.hh>
# include <scheduler/coroutine.hh>

namespace scheduler
{

  inline void
  Job::init_common(const libport::Symbol& name)
  {
    state_ = to_start;
    frozen_since_ = 0;
    time_shift_ = 0;
    name_ = name;
    coro_ = coroutine_new();
    non_interruptible_ = false;
    prio_ = UPRIO_DEFAULT;
    side_effect_free_ = false;
    check_stack_space_ = true;
    alive_jobs_++;
  }

  inline
  Job::Job(Scheduler& scheduler, const libport::Symbol& name)
    : RefCounted()
    , scheduler_(scheduler)
  {
    init_common(name);
  }

  inline
  Job::Job(const Job& model, const libport::Symbol& name)
    : RefCounted()
    , scheduler_(model.scheduler_)
  {
    init_common(name);
    time_shift_ = model.time_shift_;
  }

  inline
  Job::~Job()
  {
    coroutine_free(coro_);
    alive_jobs_--;
  }

  inline Scheduler&
  Job::scheduler_get() const
  {
    return scheduler_;
  }

  inline bool
  Job::terminated() const
  {
    return state_ == zombie;
  }

  inline void
  Job::yield()
  {
    // The only case where we yield while being non-interruptible
    // is when we get frozen. This is used to suspend a task
    // and resume it in non-interruptible contexts.
    if (non_interruptible_ && !frozen())
      return;

    state_ = running;
    scheduler_.resume_scheduler(this);
  }

  inline void
  Job::yield_until(libport::utime_t deadline)
  {
    if (non_interruptible_)
      scheduling_error
	("attempt to sleep in non-interruptible code");

    state_ = sleeping;
    deadline_ = deadline;
    scheduler_.resume_scheduler(this);
  }

  inline Coro*
  Job::coro_get() const
  {
    assert(coro_);
    return coro_;
  }

  inline void
  Job::start_job()
  {
    assert(state_ == to_start);
    scheduler_.add_job(this);
  }

  inline void
  Job::side_effect_free_set(bool s)
  {
    side_effect_free_ = s;
  }

  inline bool
  Job::side_effect_free_get() const
  {
    return side_effect_free_;
  }

  inline const libport::Symbol&
  Job::name_get() const
  {
    return name_;
  }

  inline bool
  Job::non_interruptible_get() const
  {
    return non_interruptible_;
  }

  inline void
  Job::non_interruptible_set(bool ni)
  {
    non_interruptible_ = ni;
  }

  inline void
  Job::check_stack_space()
  {
    if (check_stack_space_ &&
	coroutine_stack_space_almost_gone(coro_))
    {
      libport::Finally finally(libport::scoped_set(check_stack_space_, false));
      scheduling_error("stack space exhausted");
    }
  }

  inline job_state
  Job::state_get() const
  {
    return state_;
  }

  inline void
  Job::state_set(job_state state)
  {
    state_ = state;
  }

  inline libport::utime_t
  Job::deadline_get() const
  {
    return deadline_;
  }

  inline void
  Job::notice_frozen(libport::utime_t current_time)
  {
    if (!frozen_since_)
      frozen_since_ = current_time;
  }

  inline void
  Job::notice_not_frozen(libport::utime_t current_time)
  {
    if (frozen_since_)
    {
      libport::utime_t time_spent_frozen = current_time - frozen_since_;
      time_shift_ += time_spent_frozen;
      if (state_ == sleeping)
	deadline_ += time_spent_frozen;
    }
    frozen_since_ = 0;
  }

  inline libport::utime_t
  Job::frozen_since_get() const
  {
    return frozen_since_;
  }

  inline libport::utime_t
  Job::time_shift_get() const
  {
    return time_shift_;
  }

  inline void
  Job::time_shift_set(libport::utime_t ts)
  {
    time_shift_ = ts;
  }

  inline void
  Job::apply_tag(const rTag& tag, libport::Finally* finally)
  {
    tag->apply_tag(tags_, finally);
    if (finally)
      *finally << boost::bind(&Job::recompute_prio, this, boost::ref(*tag));
    recompute_prio(*tag);
  }

  inline void
  Job::copy_tags(const Job& other)
  {
    tags_set(other.tags_get());
    recompute_prio();
  }

  inline const tags_type&
  Job::tags_get() const
  {
    return tags_;
  }

  inline void
  Job::tags_set(const tags_type& tags)
  {
    tags_ = tags;
    recompute_prio();
  }

  inline void
  Job::tags_clear()
  {
    tags_.clear();
    recompute_prio();
  }

  inline bool
  Job::has_pending_exception() const
  {
    return pending_exception_.get();
  }

  inline prio_type
  Job::prio_get() const
  {
    return prio_;
  }

  inline bool
  Job::child_job() const
  {
    return parent_;
  }

  inline std::ostream&
  operator<< (std::ostream& o, const Job& j)
  {
    return o << "Job(" << j.name_get() << ")";
  }

  inline const char*
  state_name(job_state state)
  {
#define CASE(State) case State: return #State;
    switch (state)
    {
      CASE(to_start)
      CASE(running)
      CASE(sleeping)
      CASE(waiting)
      CASE(joining)
      CASE(zombie)
    }
#undef CASE
    return "<unknown state>";
  }

  inline
  ChildException::ChildException(exception_ptr exc)
    : child_exception_(exc)
  {
  }

  inline
  ChildException::ChildException(const ChildException& exc)
    : SchedulerException(exc)
    , child_exception_(exc.child_exception_)
  {
  }

  inline exception_ptr
  ChildException::clone() const
  {
    return exception_ptr(new ChildException(child_exception_));
  }

  inline void
  ChildException::rethrow_() const
  {
    throw *this;
  }

  inline void
  ChildException::rethrow_child_exception() const
  {
    child_exception_->rethrow();
  }

} // namespace scheduler

#endif // !SCHEDULER_JOB_HXX
