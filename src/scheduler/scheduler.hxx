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
    : current_job_ (0),
      coro_ (coroutine_new ()),
      possible_side_effect_ (true),
      cycle_ (0)
  {
    ECHO ("Initializing main coroutine");
    coroutine_initialize_main (coro_);
  }

  inline
  Scheduler::~Scheduler ()
  {
    ECHO ("Destroying scheduler");
  }

  inline
  Job& Scheduler::current_job () const
  {
    assert (current_job_);
    return *current_job_;
  }

  inline
  void Scheduler::take_job_reference (rJob& job)
  {
    assert (!to_kill_);
    std::swap (job, to_kill_);
  }

  inline
  unsigned int Scheduler::cycle_get () const
  {
    return cycle_;
  }

} // namespace scheduler

#endif // !SCHEDULER_SCHEDULER_HXX
