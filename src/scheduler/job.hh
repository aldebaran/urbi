/**
 ** \file scheduler/job.hh
 ** \brief Definition of scheduler::Job.
 */

#ifndef SCHEDULER_JOB_HH
# define SCHEDULER_JOB_HH

# include <iosfwd>

# include <libport/symbol.hh>
# include <libport/utime.hh>

# include "kernel/exception.hh"
# include "object/urbi-exception.hh"
# include "scheduler/fwd.hh"

// From libcoroutine/Coro.h.
class Coro;

namespace scheduler
{

  class Job
  {
  public:
    Job (const Job&, const libport::Symbol& name = SYMBOL ());
    explicit Job (Scheduler& scheduler, const libport::Symbol& name = SYMBOL ());
    virtual ~Job ();

    Scheduler& scheduler_get () const;

    /// Get the underlying coroutine corresponding to this job.
    Coro* coro_get () const;

    /// Has this job terminated?
    bool terminated () const;

    /// Start job by adding it into the scheduler.
    void start_job ();

    /// Run the job -- called from the scheduler.
    void run ();

    /// Kill the job -- called from the scheduler.
    void terminate_now ();

    /// Register this Job on its Scheduler so that it is rescheduled next
    /// cycle. This should be called from the currently scheduled job
    /// only but must be kept visible to be callable from the primitives.
    void yield ();

    /// Ditto, but put the job at the front of the execution queue.
    void yield_front ();

    /// Ditto, but ask not to be woken up before the deadline.
    void yield_until (libport::utime_t deadline);

    /// Wait for another job to terminate before resuming execution of
    /// the current one. If the other job has already terminated, the
    /// caller will continue its execution.
    void yield_until_terminated (Job&);

    /// Wait for any other task to be scheduled.
    void yield_until_things_changed ();

    /// Indicate whether the current state of a job may influence other
    /// parts of the system. This is used by the scheduler to choose
    /// whether other jobs need scheduling or not. The default value
    /// for \a side_effect_free is false.
    void side_effect_free_set (bool);
    bool side_effect_free_get () const;

    /// Raise an exception next time this job will be resumed.
    void async_throw (const kernel::exception&);

    /// Maybe raise a deferred exception. Must be called from the scheduler
    /// while resuming the job execution.
    void check_for_pending_exception ();

    /// Establish a bi-directional link between two jobs.
    void link (Job*);

    /// Destroy a bi-directional link if it exists.
    void unlink (Job*);

    /// Get this job name
    const libport::Symbol& name_get () const;

  protected:

    /// Must be implemented to do something useful. If an exception is
    /// raised, it will be lost, but before that, it will be propagated
    // into linked jobs.
    virtual void work () = 0;

    /// Will be called if the job is killed prematurely or arrives at
    /// its end. It is neither necessary nor advised to call yield
    /// from this function. Any exception raised here will be lost.
    virtual void terminate ();

  private:
    /// Ensure proper cleanup;
    void terminate_cleanup ();

    /// Scheduler in charge of this job.  Do not delete.
    Scheduler* scheduler_;

    /// This job name
    libport::Symbol name_;

    /// Has the coroutine terminated? Set by run ().
    bool terminated_;

    /// Other jobs to wake up when we terminate.
    jobs to_wake_up_;

    /// Coro structure corresponding to this job.
    Coro* self_;

    /// List of jobs having a link to this one. If the current job
    /// terminates with an exception, any linked job will throw the
    /// exception as well when they resume.
    jobs links_;

    bool side_effect_free_;
    kernel::exception_ptr pending_exception_;
    kernel::exception_ptr current_exception_;
  };

  std::ostream& operator<< (std::ostream&, const Job&);

} // namespace scheduler

# include "scheduler/job.hxx"

#endif // !SCHEDULER_JOB_HH
