/**
 ** \file runner/job.hxx
 ** \brief Inline implementation of runner::Job.
 */

#ifndef RUNNER_JOB_HXX
# define RUNNER_JOB_HXX

# include <cassert>
# include "runner/job.hh"
# include "runner/scheduler.hh"

#define ENABLE_DEBUG_TRACES // XXX REMOVE ME XXX
#include "libport/compiler.hh" // XXX REMOVE ME XXX

namespace runner
{

  inline
  Job::Job (Scheduler& scheduler)
    : scheduler_ (&scheduler),
      started_ (false)
  {
    assert (scheduler_);
  }

  inline
  Job::~Job ()
  {
    assert (scheduler_);
    scheduler_ = 0;
    started_ = false;
  }

  inline
  void
  Job::scheduler_set (Scheduler& scheduler)
  {
    scheduler_ = &scheduler;
    assert (scheduler_);
  }

  inline
  const Scheduler&
  Job::scheduler_get () const
  {
    assert (scheduler_);
    return *scheduler_;
  }

  inline
  Scheduler&
  Job::scheduler_get ()
  {
    assert (scheduler_);
    return *scheduler_;
  }

  inline
  void
  Job::run ()
  {
    ECHO ("job " << this << " scheduled...");
    assert (scheduler_);
    if (!started_)
    {
      started_ = true;
      start ();
    }
    work ();
  }

  inline
  void
  Job::terminate ()
  {
    assert (scheduler_);
    assert (started_);
    stop ();
    started_ = false;
  }

  inline
  void Job::yield ()
  {
    assert (scheduler_);
    scheduler_->add_job (this);
  }

  inline
  void Job::start ()
  {
  }

  inline
  void Job::stop ()
  {
  }

} // namespace runner

#endif // !RUNNER_JOB_HXX
