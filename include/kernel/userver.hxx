/*!
 *******************************************************************************

 Definition of the USystem class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2007.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef KERNEL_USERVER_HXX
# define KERNEL_USERVER_HXX

# include "userver.hh"

inline libport::utime_t
UServer::lastTime()
{
  return lastTime_;
}

inline void
UServer::addConnection (UConnection* connection)
{
  assert (connection);
  addConnection (*connection);
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
