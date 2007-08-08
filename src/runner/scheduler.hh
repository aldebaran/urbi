/**
 ** \file runner/scheduler.hh
 ** \brief Definition of runner::Scheduler.
 */

#ifndef RUNNER_SCHEDULER_HH
# define RUNNER_SCHEDULER_HH

# include <boost/utility.hpp>

namespace runner
{

  class Job; // Fwd decl.

  class Scheduler : boost::noncopyable
  {
  public:
    Scheduler ();
    ~Scheduler ();

  public:
    void work ();

    /// Memory ownership of @a job is transferred to the Scheduler.
    void add_job (Job* job);

  private:
    std::list<Job*> jobs_;
  };

} // namespace runner

# include "scheduler.hxx"

#endif // !RUNNER_SCHEDULER_HH
