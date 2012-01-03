/*
 * Copyright (C) 2007-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/kernel/userver.hxx
/// \brief Inline implementation of UServer.

#ifndef KERNEL_USERVER_HXX
# define KERNEL_USERVER_HXX

# include <urbi/kernel/userver.hh>

# include <sched/scheduler.hh>

namespace kernel
{

  /*--------------------.
  | UServer::AsyncJob.  |
  `--------------------*/

  inline
  UServer::AsyncJob::AsyncJob(object::rObject t, libport::Symbol m,
                              const object::objects_type& a)
    : target(t)
    , method(m)
    , args(a)
  {}

  inline
  UServer::AsyncJob::AsyncJob(boost::function0<void> c,
                              libport::Symbol m)
    : method(m)
    , callback(c)
  {}

  /*----------.
  | UServer.  |
  `----------*/

  inline
  void
  UServer::schedule(const AsyncJob& j)
  {
    libport::BlockLock lock(async_jobs_lock_);
    async_jobs_ << j;
    wake_up();
  }

  inline
  libport::utime_t
  UServer::lastTime()
  {
    return lastTime_;
  }

  inline
  const sched::Scheduler&
  UServer::scheduler_get () const
  {
    return *scheduler_;
  }

  inline
  sched::Scheduler&
  UServer::scheduler_get ()
  {
    return *scheduler_;
  }

  inline
  UrbiRoot&
  UServer::urbi_root_get()
  {
    return urbi_root_;
  }

  inline
  bool
  UServer::interactive_get() const
  {
    return interactive_;
  }

  inline
  void
  UServer::interactive_set(bool i)
  {
    interactive_ = i;
  }


  inline runner::Runner&
  UServer::getCurrentRunner() const
  {
    // FIXME: check that main thread is currently in handle_synchronizer_().
    return reinterpret_cast<runner::Runner&> (scheduler_->current_job());
  }

  inline runner::Runner*
  UServer::getCurrentRunnerOpt() const
  {
    sched::Job* j = scheduler_->current_job_opt();
    if (j)
      return reinterpret_cast<runner::Runner*>(j);
    return 0;
  }

  /*-------------------------.
  | Freestanding functions.  |
  `-------------------------*/

  inline
  UServer&
  server()
  {
    return *urbiserver;
  }

  inline
  runner::Runner&
  runner()
  {
    return server().getCurrentRunner();
  }

  inline
  runner::Interpreter&
  interpreter()
  {
    return reinterpret_cast<runner::Interpreter&>(runner());
  }
}

#endif // !KERNEL_USERVER_HXX
