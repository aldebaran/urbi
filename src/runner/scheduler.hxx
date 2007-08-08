/**
 ** \file runner/scheduler.hxx
 ** \brief Inline implementation of runner::Scheduler.
 */

#ifndef RUNNER_SCHEDULER_HXX
# define RUNNER_SCHEDULER_HXX

# include "runner/scheduler.hh"

namespace runner
{

  inline
  Scheduler::Scheduler ()
    : jobs_ (),
      active_job_ (0)
  {
  }

  inline
  Scheduler::~Scheduler ()
  {
  }

} // namespace runner

#endif // !RUNNER_SCHEDULER_HXX
