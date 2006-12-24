/*! \file uasyncregister.cc
 *******************************************************************************

 File: uasyncregister.cc\n
 Implementation of the UASyncRegister class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */
#include "libport/cstdio"
#include <sstream>

#include "uasyncregister.hh"
#include "uasynccommand.hh"

//FIXME: used only for debugging purposes
#include "userver.hh"
#include "utypes.hh"

// **************************************************************************
// UASyncRegister

UASyncRegister::UASyncRegister ()
{
}

UASyncRegister::~UASyncRegister()
{
  for  (std::list<UASyncCommand*>::iterator it = register_.begin ();
	it != register_.end ();
	it++)
    (*it)->registered_out (this);
}

void
UASyncRegister::registerCmd(UASyncCommand* cmd)
{
  register_.push_back(cmd);
  cmd->registered_in (this);
}

void
UASyncRegister::unregisterCmd(UASyncCommand* cmd)
{
  register_.remove(cmd);
}

void
UASyncRegister::updateRegisteredCmd ()
{
  for  (std::list<UASyncCommand*>::iterator it = register_.begin ();
	it != register_.end ();
	it++)
    (*it)->force_reeval ();
}
