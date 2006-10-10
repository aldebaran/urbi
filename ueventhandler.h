/*! \file ueventhandler.h
 *******************************************************************************

 File: ueventhandler.h\n
 Definition of the UEventHandler class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UEVENTHANDLER_H_DEFINED
#define UEVENTHANDLER_H_DEFINED

#include <list>

#include "fwd.hh"
#include "ustring.h"


// *****************************************************************************
//! Contains an event handler definition
class UEventHandler
{
public:

  UEventHandler(UString *device);
  ~UEventHandler();

	UCommand_EMIT* cmd;
  UString*       name;
};

#endif
