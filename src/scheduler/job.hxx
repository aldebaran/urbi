/**
 ** \file scheduler/job.hxx
 ** \brief Inline implementation of scheduler::Job.
 */

#ifndef SCHEDULER_JOB_HXX
# define SCHEDULER_JOB_HXX

# include <cassert>
# include "scheduler/scheduler.hh"
# include "scheduler/coroutine.hh"

namespace scheduler
{

  inline
  Job::Job (Scheduler& scheduler, const libport::Symbol& name)
    : RefCounted (),
      state_ (to_start),
      frozen_since_ (0),
      time_shift_ (0),
      scheduler_ (scheduler),
      name_ (name == SYMBOL () ? libport::Symbol::fresh (SYMBOL (job)) : name),
      coro_ (coroutine_new ()),
      non_interruptible_ (false),
      side_effect_free_ (false),
      pending_exception_ (0)
  {
  }

  inline
  Job::Job (const Job& model, const libport::Symbol& name)
  :   RefCounted (),
      state_ (to_start),
      frozen_since_ (0),
      time_shift_ (model.time_shift_),
      scheduler_ (model.scheduler_),
      name_ (name == SYMBOL () ? libport::Symbol::fresh (model.name_get ()) : name),
      coro_ (coroutine_new ()),
      tags_ (model.tags_),
      non_interruptible_ (false),
      side_effect_free_ (false),
      pending_exception_ (0)
  {
  }

  inline
  Job::~Job ()
  {
    pending_exception_ = 0;
    coroutine_free (coro_);
  }

  inline Scheduler&
  Job::scheduler_get () const
  {
    return scheduler_;
  }

  inline bool
  Job::terminated () const
  {
    return state_ == zombie;
  }

  inline void
  Job::yield ()
  {
    // The only case where we yield while being non-interruptible
    // is when we get frozen. This is used to suspend a task
    // and resume it in non-interruptible contexts.
    if (non_interruptible_ && !frozen ())
      return;

    state_ = running;
    scheduler_.resume_scheduler (this);
  }

  inline void
  Job::yield_until (libport::utime_t deadline)
  {
    if (non_interruptible_)
      throw object::SchedulingError ("attempt to sleep in non-interruptible code");

    state_ = sleeping;
    deadline_ = deadline;
    scheduler_.resume_scheduler (this);
  }

  inline Coro*
  Job::coro_get () const
  {
    assert (coro_);
    return coro_;
  }

  inline void
  Job::start_job ()
  {
    assert (state_ == to_start);
    scheduler_.add_job (this);
  }

  inline void
  Job::side_effect_free_set (bool s)
  {
    side_effect_free_ = s;
  }

  inline bool
  Job::side_effect_free_get () const
  {
    return side_effect_free_;
  }

  inline void
  Job::link (rJob other)
  {
    links_.push_back (other);
    other->links_.push_back (this);
  }

  inline const libport::Symbol&
  Job::name_get () const
  {
    return name_;
  }

  inline bool
  Job::non_interruptible_get () const
  {
    return non_interruptible_;
  }

  inline void
  Job::non_interruptible_set (bool ni)
  {
    non_interruptible_ = ni;
  }

  inline void
  Job::check_stack_space () const
  {
    if (coroutine_stack_space_almost_gone (coro_))
      throw object::StackExhaustedError ("stack space exhausted");
  }

  inline job_state
  Job::state_get () const
  {
    return state_;
  }

  inline void
  Job::state_set (job_state state)
  {
    state_ = state;
  }

  inline libport::utime_t
  Job::deadline_get () const
  {
    return deadline_;
  }

  inline void
  Job::notice_frozen (libport::utime_t current_time)
  {
    if (!frozen_since_)
      frozen_since_ = current_time;
  }

  inline void
  Job::notice_not_frozen (libport::utime_t current_time)
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
  Job::frozen_since_get () const
  {
    return frozen_since_;
  }

  inline libport::utime_t
  Job::time_shift_get () const
  {
    return time_shift_;
  }

  inline void
  Job::time_shift_set (libport::utime_t ts)
  {
    time_shift_ = ts;
  }

  inline void
  Job::push_tag (rTag tag)
  {
    tags_.push_back (tag);
  }

  inline void
  Job::pop_tag ()
  {
    tags_.pop_back ();
  }

  inline void
  Job::copy_tags (const Job& other)
  {
    tags_set (other.tags_get());
  }

  inline tags_type
  Job::tags_get () const
  {
    return tags_;
  }

  inline void
  Job::tags_set (tags_type tags)
  {
    tags_ = tags;
  }

  inline std::ostream&
  operator<< (std::ostream& o, const Job& j)
  {
    return o << "Job(" << j.name_get () << ")";
  }

  inline const char*
  state_name (job_state state)
  {
#define CASE(State) case State: return #State;
    switch (state)
    {
      CASE(to_start)
      CASE (running)
      CASE(sleeping)
      CASE(waiting)
      CASE(joining)
      CASE(zombie)
    }
#undef CASE
    return "<unknown state>";
  }

} // namespace scheduler

#endif // !SCHEDULER_JOB_HXX
