/**
 ** \file runner/job.hh
 ** \brief Definition of runner::Job.
 */

#ifndef RUNNER_JOB_HH
# define RUNNER_JOB_HH

# include "runner/fwd.hh"
# include "runner/libcoroutine/Coro.h"

namespace runner
{

  class Job
  {
  public:
    Job (const Job&);
    explicit Job (Scheduler& scheduler);
    virtual ~Job ();

    Scheduler& scheduler_get () const;

    /// Get the underlying coroutine corresponding to this job.
    Coro* coro_get () const;

    /// Has this job terminated?
    bool terminated () const;

    /// Run the job -- called from the scheduler.
    void run ();

    /// Kill the job -- called from the scheduler.
    void terminate_now ();

    /// Register this Job on its Scheduler so that it is rescheduled next
    /// cycle. This should be called from the currently scheduled job
    /// only but must be kept visible to be callable from the primitives.
    void yield ();

  protected:

    /// Must be implemented to do something useful. If an exception is
    /// raised, it will be lost.
    virtual void work () = 0;

    /// Will be called if the job is killed prematurely or arrives at
    /// its end. It is neither necessary nor advised to call yield
    /// from this function. Any exception raised here will be lost.
    void terminate ();

  private:
    /// Scheduler in charge of this job.  Do not delete.
    Scheduler* scheduler_;

    /// Has the coroutine terminated? Set by run ().
    bool terminated_;

    /// Coro structure corresponding to this job
    Coro* self_;
  };

} // namespace runner

# include "runner/job.hxx"

#endif // !RUNNER_JOB_HH
