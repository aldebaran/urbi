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
  {
  }

  inline
  Scheduler::~Scheduler ()
  {
  }

  inline
  void
  Scheduler::add_job (Job* job)
  {
    assert (job);
    jobs_.push_back (job);
  }

} // namespace runner

#endif // !RUNNER_SCHEDULER_HXX
