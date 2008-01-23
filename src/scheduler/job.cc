#include <iostream>
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
      terminate ();
    }
    catch (...)
    {
      // Exception is lost, as written in the header file. However, be
      // nice and signal it.
      std::cerr << "Exception caught in job " << this << ",loosing it\n";
    }
    terminated_ = true;
    yield ();

    // We should never go there as the scheduler will have terminated us.
    assert (false);
  }
}
