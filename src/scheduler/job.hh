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

  // A Job represents a thread of control, implemented using coroutines.
  // The scheduler decides which job to launch and resumes its execution.
  // The lifetime of a job is the following
  //         Job ()                  : job creation
  //         start_job ()            : add job to the scheduler; the scheduler
  //                                   will call run (), and the job will
  //                                   yield () itself into the scheduler run
  //                                   queue
  //         work ()                 : this method, which must be overridden,
  //                                   does the real work
  //         terminate ()            : called when the job terminates, either
  //                                   because it has indicated it wanted to,
  //                                   because its work () method has
  //                                   returned or because an exception
  //                                   has been thrown from work ()
  //
  // When the job destructor is called, it will call the unschedule ()
  // routine of its runner which will take care of removing it from all
  // the working queues. The job will then free the space allocated for
  // its coroutine structure.
  //
  // However, due to the way coroutines work, a job may not delete itself
  // as once the coroutine structure has been freed, it is illegal to
  // continue its execution, even only to switch to another coroutine.
  // And if a job switches back to the scheduler before terminating its
  // destructor, the coroutine structure will not be freed. Because of
  // that, it is necessary for a job to be deleted by another one or
  // by the scheduler.
  //
  // An additional constraint comes from the fact that even when a job
  // has terminated its work, its structure may not always be deleted.
  // For example, other jobs may have kept a reference to this job and
  // expect to retrieve some information (such as a result) after the
  // job termination.
  //
  // We have several ways of dealing with those constraints and not leaking
  // memory upon a job termination:
  //
  //   1. Make sure the job spits out all the useful information concerning
  //      its state by the way of inter-job communication before asking to
  //      be deleted.
  //   2. Make sure the job keeps alive as long as there is at least one
  //      reference onto it, and that it gets deleted from another coroutine.
  //
  // We chose to implement the second solution. To achieve this, the
  // following method has been used:
  //
  //   1. All the jobs are dynamically allocated. It is forbidden to
  //      allocate a job from another job stack.
  //   2. All the references to a job use a rJob value, which is a
  //      reference counted pointer. It is stored as a field in the
  //      Job structure, hence ensuring that the job destructor will
  //      not fire as long as we do not override this field.
  //   3. At creation time, a job creates the rJob which will be used
  //      to represent itself. This rJob may be retrieved using
  //      myself_get () should anyone need to keep a reference to this
  //      job.
  //   4. In its terminate () routine, the rJob will get rid of its
  //      self reference. To avoid decreasing the reference count to 0,
  //      it will do so by handing the reference to the scheduler using
  //      take_job_reference (). This will swap the current rJob pointer
  //      with the scheduler to_kill_ pointer, which will always be 0
  //      at this stage. As a consequence, we ensure that the reference
  //      count will at least be 1.
  //   5. After returning from the job, the scheduler will set its
  //      to_kill_ pointer to 0. As a consequence, if a job has called
  //      take_job_reference (), the reference count will be decremented
  //      by 1. If nobody else has a reference on the job, its destructor
  //      will be called and its memory and the one of its associated
  //      coroutine structure will be freed. If there are other references,
  //      the job structure will not be destroyed until everyone has
  //      dropped all those references.

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

    /// Get this job name.
    const libport::Symbol& name_get () const;

    /// Throw an exception if the stack space for this job is near
    // exhaustion.
    void check_stack_space () const;

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
