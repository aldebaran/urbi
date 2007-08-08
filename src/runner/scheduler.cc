/**
 ** \file runner/scheduler.cc
 ** \brief Implementation of runner::Scheduler.
 */

#define ENABLE_DEBUG_TRACES

#include <cassert>

#include <boost/foreach.hpp>

#include "libport/compiler.hh"

#include "runner/scheduler.hh"
#include "runner/job.hh"

namespace runner
{

  void
  Scheduler::work ()
  {
    ECHO (jobs_.size () << " jobs pending");
    BOOST_FOREACH (Job* job, jobs_)
    {
      assert (job);
      ECHO ("scheduling " << job);
      job->run ();
    }
  }

} // namespace runner
