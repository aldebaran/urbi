/// \file kernel/userver.hxx
/// \brief Inline implementation of UServer.

#ifndef KERNEL_USERVER_HXX
# define KERNEL_USERVER_HXX

# include <kernel/userver.hh>

inline libport::utime_t
UServer::lastTime()
{
  return lastTime_;
}

inline const scheduler::Scheduler&
UServer::getScheduler () const
{
  return *scheduler_;
}

inline scheduler::Scheduler&
UServer::getScheduler ()
{
  return *scheduler_;
}

#endif // !KERNEL_USERVER_HXX
