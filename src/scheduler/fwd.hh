/**
 ** \file scheduler/fwd.hh
 ** \brief Forward declarations for the namespace scheduler.
 */

#ifndef SCHEDULER_FWD_HH
# define SCHEDULER_FWD_HH

# include <list>

# include <libport/shared-ptr.hh>

namespace scheduler
{

  class Scheduler;
  class Job;
  typedef std::list<Job*> jobs_type;
  typedef libport::shared_ptr<Job> rJob;

} // namespace scheduler

#endif // !SCHEDULER_FWD_HH
