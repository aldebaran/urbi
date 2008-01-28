/**
 ** \file scheduler/fwd.hh
 ** \brief Forward declarations for the namespace scheduler.
 */

#ifndef SCHEDULER_FWD_HH
# define SCHEDULER_FWD_HH

# include <list>

namespace scheduler
{

  class Scheduler;
  class Job;
  typedef std::list<Job*> jobs;

} // namespace scheduler

#endif // !SCHEDULER_FWD_HH
