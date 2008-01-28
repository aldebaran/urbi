#include <iostream>
#include <boost/foreach.hpp>

#include "libport/compiler.hh"     // For ECHO
#include "job.hh"

namespace scheduler
{
  void
  Job::run ()
  {
    ECHO ("In Job::run for " << this);
    yield_front ();
    try {
      work ();
      terminate_now ();
    }
    catch (...)
    {
      // Exception is lost, as written in the header file. However, be
      // nice and signal it.
      std::cerr << "Exception caught in job " << this << ",loosing it\n";
    }
    yield ();

    // We should never go there as the scheduler will have terminated us.
    assert (false);
  }

  void
  Job::terminate_now ()
  {
    terminate ();
    terminated_ = true;
    BOOST_FOREACH (Job* job, to_wake_up_)
      scheduler_->resume_job (job);
  }

  void
  Job::yield_until_terminated (Job& other)
  {
    if (!other.terminated ())
    {
      other.to_wake_up_.push_back (this);
      scheduler_->resume_scheduler_suspend (this);
    }
  }
}
