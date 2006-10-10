/*! \file ughostconnection.cc
 *******************************************************************************

 File: ughostconnection.cc\n
 Implementation of the UGhostConnection class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#include <string.h>

#include "utypes.h"
#include "ughostconnection.h"

class UServer;

//! UGhostConnection constructor.
UGhostConnection::UGhostConnection  ( UServer * mainserver) :
  UConnection   ( mainserver,
		  UGhostConnection::MINSENDBUFFERSIZE,
		  UGhostConnection::MAXSENDBUFFERSIZE,
		  UGhostConnection::PACKETSIZE,
		  UGhostConnection::MINRECVBUFFERSIZE,
		  UGhostConnection::MAXRECVBUFFERSIZE )
{
  ADDOBJ(UGhostConnection);

  if (UError != USUCCESS) return;// Test the error from UConnection constructor.
}

//! UGhostConnection destructor.
UGhostConnection::~UGhostConnection()
{
  FREEOBJ(UGhostConnection);
}

//! Close the connection
/*! This function does nothing. The ghost connection cannot be closed.
*/
UErrorValue
UGhostConnection::closeConnection()
{
  return (USUCCESS);
}

//! Does nothing. No output for the ghosts...
int
UGhostConnection::effectiveSend(const ubyte *buffer, int length)
{
  char tmpbuf[1024];
  int real_length = length;
  if (real_length>=1024)
    real_length = 1023;

  memcpy((void*)tmpbuf,(void*)buffer,real_length);
  tmpbuf[real_length]=0;

  ::urbiserver->debug(tmpbuf);

  return length;
}
