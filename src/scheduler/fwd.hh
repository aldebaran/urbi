/**
 ** \file scheduler/fwd.hh
 ** \brief Forward declarations for the namespace scheduler.
 */

#ifndef SCHEDULER_FWD_HH
# define SCHEDULER_FWD_HH

# include <list>

# include <libport/shared-ptr.hh>

# include "kernel/exception.hh"

namespace scheduler
{

  class Scheduler;
  class Job;
  typedef std::list<Job*> jobs_type;
  typedef libport::shared_ptr<Job> rJob;
  class Tag;
  typedef libport::shared_ptr<Tag> rTag;

  // This exception is above other scheduler-related exceptions such
  // as BlockedException. This allows catching more specific exceptions
  // first, then handling scheduler-related exceptions in a general
  // way.
  struct SchedulerException : public kernel::exception
  {
    COMPLETE_EXCEPTION (SchedulerException);
  };

} // namespace scheduler

#endif // !SCHEDULER_FWD_HH
