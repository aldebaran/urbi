/**
 ** \file runner/job.hh
 ** \brief Definition of runner::Job.
 */

#ifndef RUNNER_JOB_HH
# define RUNNER_JOB_HH

# include "runner/fwd.hh"

namespace runner
{

  class Job
  {
  public:
    explicit Job (Scheduler& scheduler);
    virtual ~Job ();

    void scheduler_set (Scheduler& scheduler);
    const Scheduler& scheduler_get () const;
    Scheduler& scheduler_get ();

    /// Start to do some work (or continue unfinished work).
    void run ();
    /// Stop this job.  If \c run is called again, the job will restart.
    void terminate ();

  protected:
    /// Register this Job on its Scheduler so that it is rescheduled next
    /// cycle (that is, its \c run method will be invoked).
    void yield ();

    /// Can optionally be overridden to do something when (re)starting a new
    /// job.
    virtual void start ();
    /// Must be implemented to do something useful.
    virtual void work () = 0;
    /// Can optionally be overridden to do something when stopping a job.
    virtual void stop ();

  private:
    /// Scheduler in charge of this job.  Do not delete.
    Scheduler* scheduler_;
    bool started_;
  };

} // namespace runner

# include "runner/job.hxx"

#endif // !RUNNER_JOB_HH
