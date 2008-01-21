/**
 ** \file runner/job.hxx
 ** \brief Inline implementation of runner::Job.
 */

#ifndef RUNNER_JOB_HXX
# define RUNNER_JOB_HXX

# include <cassert>
# include "runner/job.hh"
# include "runner/scheduler.hh"

namespace runner
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

  inline void
  Job::terminate_now ()
  {
    terminate ();
    terminated_ = true;
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

  inline Coro*
  Job::coro_get () const
  {
    assert (self_);
    return self_;
  }

} // namespace runner

#endif // !RUNNER_JOB_HXX
