/**
 ** \file scheduler/scheduler.hh
 ** \brief Definition of scheduler::Scheduler.
 */

#ifndef SCHEDULER_SCHEDULER_HH
# define SCHEDULER_SCHEDULER_HH

# include <boost/utility.hpp>
# include <libport/utime.hh>

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
    /// Do one cycle of work, and return the next time we expect to be called.
    /// If we have work to do, 0 will be returned in order to be called again
    /// as soon as possible. If we only have time-suspended or dependent
    /// jobs, we will return the time of the next scheduled one. In short,
    /// calling work() again before the returned time is useless as there will
    /// be nothing to do except if some new work has been entered in.
    libport::utime_t work ();

    /// Add \a job to the list of jobs to be run later. Jobs will be started
    /// at the next cycle by the scheduler. It is advised to call \a start_job
    /// on \a *job rather than invoking this method.
    void add_job (Job* job);

    /// Remove all jobs.
    void killall_jobs ();

    /// Kill \a job.
    void kill_job (Job* job);

    /// Unschedule a job but do not delete it. Also here, the currently
    /// executing job cannot unschedule itself.
    void unschedule_job (Job* job);

    /// Resume scheduler execution. Must be called from the job being
    /// interrupted with itself as argument.
    void resume_scheduler (Job* job);

    /// Take the (maybe last) reference on a job and swap it with 0.
    void take_job_reference (rJob&);

    /// Return the currently executing job
    Job& current_job () const;

    /// Signal that a stop (or block) has been issued on a tag, and that
    /// queued jobs should be checked soon to see whether they need to
    /// get some treatment.
    void signal_stop (rTag);

    /// Return the current cycle
    unsigned int cycle_get () const;

  private:
    libport::utime_t execute_round (bool blocked_only);

    /// Check if we have stopped tags to handle. In this case, we will
    /// wake up any blocked jobs so that they can handle the stop
    /// situation. This is used one after each scheduler round.
    libport::utime_t check_for_stopped_tags (libport::utime_t old_deadline);

  private:
    /// List of jobs_ we are aware of
    jobs_type jobs_;

    /// The following fields represent running structures used in
    /// work(). Jobs be removed from here just before they are started
    /// or scheduled.
    jobs_type pending_;

    /// Current job
    Job* current_job_;

    /// Job to maybe kill
    rJob to_kill_;

    /// Coroutine support
    Coro* coro_;

    /// Has there been a possible side-effect since last time we reset
    /// this field?
    bool possible_side_effect_;

    /// Has a new job been added to the list of jobs to start in the current
    /// cycle?
    bool jobs_to_start_;

    /// Cycles counter
    unsigned int cycle_;

    /// List of tags that have been stopped or blocked
    typedef std::pair<rTag, bool> tag_state_type;
    std::vector<tag_state_type> stopped_tags_;
  };

} // namespace scheduler

# include "scheduler.hxx"

#endif // !SCHEDULER_SCHEDULER_HH
