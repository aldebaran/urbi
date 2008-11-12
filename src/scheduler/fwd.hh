/**
 ** \file scheduler/fwd.hh
 ** \brief Forward declarations for the namespace scheduler.
 */

#ifndef SCHEDULER_FWD_HH
# define SCHEDULER_FWD_HH

# include <list>

# include <libport/intrusive-ptr.hh>

# include <scheduler/exception.hh>

namespace scheduler
{

  class Scheduler;
  class Job;
  typedef libport::intrusive_ptr<Job> rJob;
  typedef std::list<rJob> jobs_type;
  class Tag;
  typedef libport::intrusive_ptr<Tag> rTag;

  // This exception is above other scheduler-related exceptions such
  // as BlockedException. This allows catching more specific exceptions
  // first, then handling scheduler-related exceptions in a general
  // way.
  struct SchedulerException : public exception
  {
    COMPLETE_EXCEPTION(SchedulerException);
  };

} // namespace scheduler

#endif // !SCHEDULER_FWD_HH
