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
#include <cstdio>
#include <list>

#include <sstream>

#include "uasynccommand.hh"
#include "ucommand.hh"
#include "uconnection.hh"
#include "userver.hh"
#include "utypes.hh"
#include "ueventinstance.hh"
#include "ueventmatch.hh"

// **************************************************************************
// UASyncCommand

UASyncCommand::UASyncCommand()
{
  reeval_ = true;
}

UASyncCommand::~UASyncCommand()
{
  for  (std::list<UASyncRegister*>::iterator it = regList_.begin ();
	it != regList_.end ();
	it++)
    (*it)->unregisterCmd (this);
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
