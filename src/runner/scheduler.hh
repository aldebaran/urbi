/**
 ** \file runner/scheduler.hh
 ** \brief Definition of runner::Scheduler.
 */

#ifndef RUNNER_SCHEDULER_HH
# define RUNNER_SCHEDULER_HH

# include <list>
# include <boost/utility.hpp>

# include "runner/fwd.hh"

namespace runner
{

  class Scheduler : boost::noncopyable
  {
  public:
    Scheduler ();
    ~Scheduler ();

  public:
    void work ();

    /// Memory ownership of @a job is transferred to the Scheduler.
    void add_job (Job* job);

    // Ask the scheduler do run this @a job immediately.  Memory ownership
    // of @a job is transferred to the Scheduler.
    void schedule_immediately (Job* job);

  private:
    typedef std::list<Job*> jobs;
    jobs jobs_;
  };

} // namespace runner

# include "scheduler.hxx"

#endif // !RUNNER_SCHEDULER_HH
