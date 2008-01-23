/**
 ** \file scheduler/scheduler.hxx
 ** \brief Inline implementation of scheduler::Scheduler.
 */

#ifndef SCHEDULER_SCHEDULER_HXX
# define SCHEDULER_SCHEDULER_HXX

# include "scheduler/scheduler.hh"
# include "scheduler/job.hh"

namespace scheduler
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

} // namespace scheduler

#endif // !SCHEDULER_SCHEDULER_HXX
