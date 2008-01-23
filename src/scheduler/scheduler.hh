/**
 ** \file scheduler/scheduler.hh
 ** \brief Definition of scheduler::Scheduler.
 */

#ifndef SCHEDULER_SCHEDULER_HH
# define SCHEDULER_SCHEDULER_HH

# include <list>
# include <boost/utility.hpp>

# include "scheduler/fwd.hh"
# include "scheduler/libcoroutine/Coro.h"

namespace scheduler
{

  class Scheduler : boost::noncopyable
  {
  public:
    Scheduler ();
    ~Scheduler ();

  public:
    void work ();

    // Add a job to the list of jobs to be run later. Jobs will be started
    // at the next cycle by the scheduler.
    void add_job (Job* job);

    /// Remove all jobs but the caller one. It will have to terminate
    /// after that.
    void killall_jobs ();

    /// Kill a job, and delete it. It is not allowed to kill the currently
    /// executing job as it will do very bad things.
    void kill_job (Job* job);

    /// Resume scheduler execution. Must be called from the job being
    /// interrupted with itself as argument.
    void resume_scheduler (Job* job);

  private:
    typedef std::list<Job*> jobs;
    jobs jobs_;
    jobs jobs_to_start_;
    Coro* self_;
  };

} // namespace scheduler

# include "scheduler.hxx"

#endif // !SCHEDULER_SCHEDULER_HH
