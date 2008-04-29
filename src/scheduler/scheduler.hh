/**
 ** \file scheduler/scheduler.hh
 ** \brief Definition of scheduler::Scheduler.
 */

#ifndef SCHEDULER_SCHEDULER_HH
# define SCHEDULER_SCHEDULER_HH

# include <boost/function.hpp>
# include <boost/utility.hpp>
# include <libport/utime.hh>

# include "scheduler/coroutine.hh"
# include "scheduler/fwd.hh"

namespace scheduler
{

  class Scheduler : boost::noncopyable
  {
  public:
    /// Constructor.
    ///
    /// \param get_time A function which, when called, returns the
    ///        current system time.
    Scheduler (boost::function0<libport::utime_t> get_time);

    /// Destructor.
    ~Scheduler ();

  public:
    /// Do one cycle of work, and return the next time we expect to be called.
    ///
    /// \return If we have work to do, 0 will be returned in order to
    /// be called again as soon as possible. If we only have
    /// time-suspended or dependent jobs, we will return the time of
    /// the next scheduled one.
    ///
    /// Calling work() again before the returned time is useless as there will
    /// be nothing to do except if some new work has been entered in.
    libport::utime_t work ();

    /// Add a job to the list of jobs to be run later.
    ///
    /// \param job Job to be started. Please do not use this function
    ///        directly except from the \c Job::start_job() method.
    ///
    /// Jobs added during a cycle will be started at the next cycle by the
    /// scheduler.
    void add_job (Job* job);

    /// Terminate all jobs. This must be called only when the executable
    /// is going to terminate.
    void killall_jobs ();

    /// Unschedule a job on its way to deletion.
    ///
    /// \param job The job to unschedule. It must not be the currently
    ///        executing job. This function must be called only from
    ///        \a job destructor.
    void unschedule_job (Job* job);

    /// Resume scheduler execution.
    ///
    /// \param job Job currently being executed. This job will relinquish
    ///        the CPU to the scheduler. Note that the scheduler is free
    ///        to reschedule \a job immediately if it wishes to do so.
    void resume_scheduler (Job* job);

    /// Swap a job reference with 0.
    ///
    /// \param job A job reference which will be swapped with 0.
    ///
    /// This is used to give the scheduler a chance to delete the last
    /// reference on a job while the scheduler is the active
    /// task. This is necessary because a currently scheduled job
    /// cannot kill itself.
    void take_job_reference (rJob& job);

    /// Get the currently executing job.
    ///
    /// \return A reference onto the currently executing job.
    ///
    /// It is an error to call this method if no job is currently
    /// executing or to keep the reference after yielding since the
    /// Job may no longer be valid then.
    Job& current_job () const;

    /// Signal that a \c stop or a \c block has been issued on a tag.
    ///
    /// \param t The tag that has been recently stopped or blocked.
    ///
    /// After this function has been called, the scheduler will determine,
    /// at the end of the current cycle, which jobs need to react to this
    /// action.
    void signal_stop (rTag t);

    /// Get the current cycle number.
    ///
    /// \return The current cycle index, increasing by 1 at each cycle.
    unsigned int cycle_get () const;

    /// Get the time as seen by the scheduler.
    ///
    /// \return The current system time.
    libport::utime_t get_time () const;

  private:
    /// Execute one round in the scheduler.
    ///
    /// \param blocked_only If true, this round only effect must be to wake up
    ///        blocked jobs so that they can react to a \c stop or a
    ///        \c block action on a tag.
    ///
    /// \return See work().
    libport::utime_t execute_round (bool blocked_only);

    /// Check if we have stopped tags to handle.
    ///
    /// \param old_deadline The deadline to return if no tags have been
    ///        signalled through signal_stop() during the previous round.
    ///
    /// \return See work().
    libport::utime_t check_for_stopped_tags (libport::utime_t old_deadline);

    /// Function to retrieve the current system time.
    boost::function0<libport::utime_t> get_time_;

    /// List of jobs we are in charge of. During a cycle execution,
    /// this is where jobs will accumulate themselves after they have
    /// been executed.
    jobs_type jobs_;

    /// List of jobs currently being scheduled during the current round.
    jobs_type pending_;

    /// Current job.
    Job* current_job_;

    /// Job to kill, if any, when the scheduler regains control. \sa
    /// take_job_reference().
    rJob to_kill_;

    /// Coroutine corresponding to the scheduler.
    Coro* coro_;

    /// Has there been a possible side-effect since last time we reset
    /// this field?
    bool possible_side_effect_;

    /// Has a new job been added to the list of jobs to start in the current
    /// cycle?
    bool jobs_to_start_;

    /// Cycles counter.
    unsigned int cycle_;

    /// List of tags that have been stopped or blocked. \sa
    /// signal_stop(), check_for_stopped_tags().
    typedef std::pair<rTag, bool> tag_state_type;
    std::vector<tag_state_type> stopped_tags_;
  };

} // namespace scheduler

# include "scheduler.hxx"

#endif // !SCHEDULER_SCHEDULER_HH
