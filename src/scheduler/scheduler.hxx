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

} // namespace scheduler

#endif // !SCHEDULER_SCHEDULER_HXX
