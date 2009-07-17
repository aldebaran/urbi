/// \file kernel/userver.hxx
/// \brief Inline implementation of UServer.

#ifndef KERNEL_USERVER_HXX
# define KERNEL_USERVER_HXX

# include <kernel/userver.hh>

namespace kernel
{
  inline libport::utime_t
  UServer::lastTime()
  {
    return lastTime_;
  }

  inline const sched::Scheduler&
  UServer::scheduler_get () const
  {
    return *scheduler_;
  }

  inline sched::Scheduler&
  UServer::scheduler_get ()
  {
    return *scheduler_;
  }

  inline boost::asio::io_service&
  UServer::get_io_service ()
  {
    return io_;
  }
}

#endif // !KERNEL_USERVER_HXX
