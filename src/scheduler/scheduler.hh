/**
 ** \file scheduler/scheduler.hh
 ** \brief Definition of scheduler::Scheduler.
 */

#ifndef SCHEDULER_SCHEDULER_HH
# define SCHEDULER_SCHEDULER_HH

# include <queue>
# include <boost/tuple/tuple.hpp>
# include <boost/utility.hpp>
# include <libport/utime.hh>

# include "scheduler/fwd.hh"
# include "scheduler/libcoroutine/Coro.h"

namespace scheduler
{

  typedef boost::tuple<libport::utime_t, Job*> deferred_job;
  bool operator> (const deferred_job&, const deferred_job&);

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

    /// Remove all jobs but the caller one. It will have to terminate
    /// after that.
    void killall_jobs ();

    /// Kill \a job, and delete it. It is not allowed to kill the currently
    /// executing job as it will do very bad things.
    void kill_job (Job* job);

    /// Unschedule a job but do not delete it. Also here, the currently
    /// executing job cannot unschedule itself.
    void unschedule_job (Job* job);

    /// Resume scheduler execution. Must be called from the job being
    /// interrupted with itself as argument.
    void resume_scheduler (Job* job);

    /// Ditto, but put the job at the front of the run queue.
    void resume_scheduler_front (Job* job);

    /// Ditto, but put the job in the deferred run queue until the deadline
    /// is reached.
    void resume_scheduler_until (Job* job, libport::utime_t deadline);

    /// Ditto, but put the job in a queue that will be used only after at
    /// last one job has been scheduled during the next cycle. This may be
    /// used for jobs watching some value, which will not change spontaneously
    /// unless someone changes it.
    void resume_scheduler_things_changed (Job* job);

    /// Suspend the current job.
    void resume_scheduler_suspend (Job* job);

    /// Resume a job that has been previously suspended and add it at
    /// the back of the run queue.
    void resume_job (Job* job);

    /// Take the (maybe last) reference on a job and swap it with 0.
    void take_job_reference (rJob&);

    /// Return the currently executing job
    Job& current_job () const;

    /// Signal that a stop (or block) has been issued on a tag, and that
    /// queued jobs should be checked soon to see whether they need to
    /// get some treatment.
    void signal_stop (Tag*);

  private:
    void switch_back (Job* job);
    void execute_round (const jobs_type&);
    void check_for_stopped_tags ();

  private:
    typedef std::priority_queue
    <deferred_job, std::vector<deferred_job>, std::greater<deferred_job> >
      deferred_jobs;

    /// Regular jobs to schedule inconditionally during the next run
    jobs_type jobs_;

    /// Jobs registered for initialization but not yet started
    jobs_type jobs_to_start_;

    /// Deferred jobs
    deferred_jobs deferred_jobs_;

    /// Suspended jobs
    jobs_type suspended_jobs_;

    /// Jobs waiting for something interesting to happen
    jobs_type if_change_jobs_;

    /// The following fields represent running structures used in
    /// work(). Jobs be removed from here just before they are started
    /// or scheduled.
    jobs_type to_start_;
    jobs_type pending_;

    /// Current job
    Job* current_job_;

    /// Job to maybe kill
    rJob to_kill_;

    /// Coroutine support
    Coro* self_;

    /// Has there been a possible side-effect since last time we reset
    /// this field?
    bool possible_side_effect_;

    /// List of tags that have been stopped or blocked
    typedef std::pair<Tag*, bool> tag_state_type;
    std::vector<tag_state_type> stopped_tags_;
  };

} // namespace scheduler

# include "scheduler.hxx"

#endif // !SCHEDULER_SCHEDULER_HH
