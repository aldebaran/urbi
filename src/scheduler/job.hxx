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
      name_ (name == SYMBOL () ? libport::Symbol::fresh (SYMBOL (job)) : name),
      terminated_ (false),
      coro_ (Coro_new ()),
      side_effect_free_ (false),
      pending_exception_ (0)
  {
  }

  inline
  Job::Job (const Job& model, const libport::Symbol& name)
    : state_ (to_start),
      scheduler_ (model.scheduler_),
      name_ (name == SYMBOL () ? libport::Symbol::fresh (model.name_get ()) : name),
      terminated_ (false),
      coro_ (Coro_new ()),
      tags_ (model.tags_),
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
    return terminated_;
  }

  inline void
  Job::yield ()
  {
    state_ = running;
    scheduler_->resume_scheduler (this);
  }

  inline void
  Job::yield_front ()
  {
    state_ = running;
    scheduler_->resume_scheduler (this);
  }

  inline void
  Job::yield_until (libport::utime_t deadline)
  {
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
      kernel::rethrow (current_exception_);
    }
    if (blocked ())
      throw BlockedException ();
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
