/**
 ** \file runner/job.hh
 ** \brief Definition of runner::Job.
 */

#ifndef RUNNER_JOB_HH
# define RUNNER_JOB_HH

namespace runner
{

  class Scheduler; // Fwd decl.

  class Job
  {
  public:
    explicit Job (Scheduler& scheduler);
    virtual ~Job ();

    void setScheduler (Scheduler& scheduler);
    const Scheduler& getScheduler () const;
    Scheduler& getScheduler ();

    /// Start to do some work (or continue unfinished work).
    void run ();
    /// Stop this job.  If \c run is called again, the job will restart.
    void terminate ();

  protected:
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
