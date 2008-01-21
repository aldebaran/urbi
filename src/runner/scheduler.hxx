/**
 ** \file runner/scheduler.hxx
 ** \brief Inline implementation of runner::Scheduler.
 */

#ifndef RUNNER_SCHEDULER_HXX
# define RUNNER_SCHEDULER_HXX

# include "runner/scheduler.hh"
# include "runner/job.hh"

namespace runner
{

  inline
  Scheduler::Scheduler ()
    : jobs_ (),
      self_ (Coro_new ())
  {
    ECHO ("Initializing main coroutine");
    Coro_initializeMainCoro (self_);
  }

  inline
  Scheduler::~Scheduler ()
  {
    ECHO ("Destroying scheduler");
  }

  inline void
  Scheduler::resume_scheduler (Job* job)
  {
    // Switch back to the scheduler
    Coro_switchTo_ (job->coro_get (), self_);
    // We regained control, we are again in the context of the job.
    ECHO ("job " << job << " resumed");
  }

} // namespace runner

#endif // !RUNNER_SCHEDULER_HXX
