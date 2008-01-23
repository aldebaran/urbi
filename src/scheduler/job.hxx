/**
 ** \file scheduler/job.hxx
 ** \brief Inline implementation of scheduler::Job.
 */

#ifndef SCHEDULER_JOB_HXX
# define SCHEDULER_JOB_HXX

# include <cassert>
# include "scheduler/scheduler.hh"

namespace scheduler
{

  inline
  Job::Job (Scheduler& scheduler)
    : scheduler_ (&scheduler),
      terminated_ (false),
      self_ (Coro_new ())
  {
  }

  inline
  Job::Job (const Job& model)
    : scheduler_ (model.scheduler_),
      terminated_ (false),
      self_ (Coro_new ())
  {
  }

  inline
  Job::~Job ()
  {
    assert (terminated_);
    Coro_free (self_);
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
    scheduler_->resume_scheduler (this);
  }

  inline void
  Job::yield_front ()
  {
    scheduler_->resume_scheduler_front (this);
  }

  inline void
  Job::yield_until (libport::ufloat deadline)
  {
    scheduler_->resume_scheduler_until (this, deadline);
  }

  inline Coro*
  Job::coro_get () const
  {
    assert (self_);
    return self_;
  }

} // namespace scheduler

#endif // !SCHEDULER_JOB_HXX
