/*! \file uasynccommand.cc
 *******************************************************************************

 File: uasynccommand.cc\n
 Implementation of the UASyncCommand class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#include <cmath>
#include <cstdlib>
#include "libport/cstdio"
#include <list>

#include <sstream>

#include "kernel/userver.hh"
#include "kernel/utypes.hh"
#include "kernel/uconnection.hh"
#include "kernel/uasyncregister.hh"

#include "uasynccommand.hh"
#include "ucommand.hh"
#include "ueventinstance.hh"
#include "ueventmatch.hh"

// **************************************************************************
// UASyncCommand

UASyncCommand::UASyncCommand (const location& l, Type type)
  : UCommand (l, type),
    reeval_ (true)
{
}

UASyncCommand::~UASyncCommand()
{
  for (std::list<UASyncRegister*>::iterator i = regList_.begin ();
	i != regList_.end ();
	++i)
    (*i)->unregisterCmd (this);
}

void
UASyncCommand::registered_in (UASyncRegister* reg)
{
  regList_.push_back (reg);
}

void
UASyncCommand::registered_out (UASyncRegister* reg)
{
  regList_.remove (reg);
}

void
UASyncCommand::force_reeval()
{
  reeval_ = true;
}

bool
UASyncCommand::reeval()
{
  return reeval_;
}
