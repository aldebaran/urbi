/**
 ** \file scheduler/job.hxx
 ** \brief Inline implementation of scheduler::Job.
 */

#ifndef SCHEDULER_JOB_HXX
# define SCHEDULER_JOB_HXX

# include <cassert>
# include "scheduler/scheduler.hh"
# include "scheduler/libcoroutine/Coro.h"

namespace scheduler
{

  inline
  Job::Job (Scheduler& scheduler, const libport::Symbol& name)
    : state_ (to_start),
      scheduler_ (&scheduler),
      myself_ (this),
      name_ (name == SYMBOL () ? libport::Symbol::fresh (SYMBOL (job)) : name),
      coro_ (Coro_new ()),
      non_interruptible_ (false),
      side_effect_free_ (false),
      pending_exception_ (0)
  {
  }

  inline
  Job::Job (const Job& model, const libport::Symbol& name)
    : state_ (to_start),
      scheduler_ (model.scheduler_),
      myself_ (this),
      name_ (name == SYMBOL () ? libport::Symbol::fresh (model.name_get ()) : name),
      coro_ (Coro_new ()),
      tags_ (model.tags_),
      non_interruptible_ (false),
      side_effect_free_ (false),
      pending_exception_ (0)
  {
  }

  inline
  Job::~Job ()
  {
    scheduler_->unschedule_job (this);
    pending_exception_ = 0;
    Coro_free (coro_);
  }

  inline Scheduler&
  Job::scheduler_get () const
  {
    return *scheduler_;
  }

  inline void
  Job::terminate ()
  {
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
    scheduler_->resume_scheduler (this);
  }

  inline void
  Job::yield_until (libport::utime_t deadline)
  {
    if (non_interruptible_)
      throw object::SchedulingError ("attempt to sleep in non-interruptible code");

    state_ = sleeping;
    deadline_ = deadline;
    scheduler_->resume_scheduler (this);
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
    scheduler_->add_job (this);
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
  Job::async_throw (const kernel::exception& e)
  {
    pending_exception_ = e.clone ();
  }

  inline void
  Job::check_for_pending_exception ()
  {
    if (pending_exception_)
    {
      current_exception_ = pending_exception_;
      pending_exception_ = 0;
      // If an exception is propagated, it may have side effects
      side_effect_free_ = false;
      kernel::rethrow (current_exception_);
    }
    if (blocked ())
    {
      side_effect_free_ = false;
      throw BlockedException ();
    }
  }

  inline void
  Job::link (Job* other)
  {
    links_.push_back (other);
    other->links_.push_back (this);
  }

  inline void
  Job::unlink (Job* other)
  {
    links_.remove (other);
    other->links_.remove (this);
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
    if (Coro_stackSpaceAlmostGone (coro_))
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

  inline rJob
  Job::myself_get () const
  {
    return myself_;
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
