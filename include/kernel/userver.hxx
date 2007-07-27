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

#ifndef USERVER_HXX
# define USERVER_HXX

# include "userver.hh"

inline ufloat
UServer::getFrequency ()
{
  return frequency_;
}

inline ufloat
UServer::lastTime()
{
  return lastTime_;
}

inline bool
UServer::isRunningSystemCommands () const
{
  return systemcommands;
}

inline void
UServer::setSystemCommand (bool val)
{
  systemcommands = val;
}

inline const HMtagtab&
UServer::getTagTab () const
{
  return tagtab;
}

inline HMtagtab&
UServer::getTagTab ()
{
  return tagtab;
}

inline const HMgrouptab&
UServer::getGroupTab () const
{
  return grouptab;
}

inline HMgrouptab&
UServer::getGroupTab ()
{
  return grouptab;
}

inline const HMfunctiontab&
UServer::getFunctionTab () const
{
  return functiontab;
}

inline HMfunctiontab&
UServer::getFunctionTab ()
{
  return functiontab;
}

inline const HMobjtab&
UServer::getObjTab () const
{
  return objtab;
}

inline HMobjtab&
UServer::getObjTab ()
{
  return objtab;
}

inline const HMbindertab&
UServer::getFunctionBinderTab () const
{
  return functionbindertab;
}

inline HMbindertab&
UServer::getFunctionBinderTab ()
{
  return functionbindertab;
}

inline const HMaliastab&
UServer::getObjAliasTab () const
{
  return objaliastab;
}

inline HMaliastab&
UServer::getObjAliasTab ()
{
  return objaliastab;
}

inline bool
UServer::isDefChecking () const
{
  return defcheck;
}

inline void
UServer::setDefCheck (bool val)
{
  defcheck = val;
}

inline const HMobjWaiting&
UServer::getObjWaitTab () const
{
  return objWaittab;
}

inline HMobjWaiting&
UServer::getObjWaitTab ()
{
  return objWaittab;
}

inline const HMaliastab&
UServer::getAliasTab () const
{
  return aliastab;
}

inline HMaliastab&
UServer::getAliasTab ()
{
  return aliastab;
}

inline void
UServer::hasSomethingToDelete ()
{
  somethingToDelete = true;
}

inline const HMvariabletab&
UServer::getVariableTab () const
{
  return variabletab;
}

inline HMvariabletab&
UServer::getVariableTab ()
{
  return variabletab;
}

inline const HMfunctiontab&
UServer::getFunctionDefTab () const
{
  return functiondeftab;
}

inline HMfunctiontab&
UServer::getFunctionDefTab ()
{
  return functiondeftab;
}

inline const HMbindertab&
UServer::getEventBinderTab () const
{
  return eventbindertab;
}

inline HMbindertab&
UServer::getEventBinderTab ()
{
  return eventbindertab;
}

inline const runner::Scheduler&
UServer::getScheduler () const
{
  return *scheduler_;
}

inline runner::Scheduler&
UServer::getScheduler ()
{
  return *scheduler_;
}

inline
int unique ()
{
  static int cnt = 10000;
  return ++cnt;
}

inline
std::string unique (const std::string& prefix)
{
  std::ostringstream o;
  o << prefix << unique();
  return o.str();
}

#endif
