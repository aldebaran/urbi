/**
 ** \file scheduler/scheduler.hh
 ** \brief Definition of scheduler::Scheduler.
 */

#ifndef SCHEDULER_SCHEDULER_HH
# define SCHEDULER_SCHEDULER_HH

# include <list>
# include <queue>
# include <boost/tuple/tuple.hpp>
# include <boost/utility.hpp>

# include <libport/ufloat.hh>

# include "scheduler/fwd.hh"
# include "scheduler/libcoroutine/Coro.h"

namespace scheduler
{

  typedef boost::tuple<libport::ufloat, Job*> deferred_job;
  bool operator> (const deferred_job&, const deferred_job&);

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

    /// Ditto, but put the job at the front of the run queue.
    void resume_scheduler_front (Job* job);

    /// Dutto, but put the job in the deferred run queue until the deadline
    /// is reached.
    void resume_scheduler_until (Job* job, libport::ufloat deadline);

  private:
    void switch_back (Job *job);

  private:
    typedef std::list<Job*> jobs;
    typedef std::priority_queue
    <deferred_job, std::vector<deferred_job>, std::greater<deferred_job> >
      deferred_jobs;

    /// Regular jobs to schedule inconditionally during the next run
    jobs jobs_;

    /// Jobs registered for initialization but not yet started
    jobs jobs_to_start_;

    /// Deferred jobs
    deferred_jobs deferred_jobs_;

    /// Coroutine support
    Coro* self_;
  };

} // namespace scheduler

# include "scheduler.hxx"

#endif // !SCHEDULER_SCHEDULER_HH
