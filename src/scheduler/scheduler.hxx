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
  Scheduler::Scheduler (boost::function0<libport::utime_t> get_time)
    : get_time_ (get_time)
    , current_job_ (0)
    , coro_ (coroutine_new ())
    , possible_side_effect_ (true)
    , cycle_ (0)
    , ready_to_die_(false)
  {
    ECHO ("Initializing main coroutine");
    coroutine_initialize_main (coro_);
  }

  inline
  Scheduler::~Scheduler ()
  {
    ECHO ("Destroying scheduler");
  }

  inline Job&
  Scheduler::current_job () const
  {
    assert (current_job_);
    return *current_job_;
  }

  inline bool
  Scheduler::is_current_job(const Job& job) const
  {
    return current_job_ == &job;
  }

  inline unsigned int
  Scheduler::cycle_get () const
  {
    return cycle_;
  }

  inline libport::utime_t
  Scheduler::get_time () const
  {
    return get_time_ ();
  }

  inline void
  Scheduler::signal_world_change()
  {
    possible_side_effect_ = true;
  }

} // namespace scheduler

#endif // !SCHEDULER_SCHEDULER_HXX
