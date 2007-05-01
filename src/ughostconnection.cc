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

#include "libport/cstring"

#include "utypes.hh"
#include "ughostconnection.hh"
#include "userver.hh"

//! UGhostConnection constructor.
UGhostConnection::UGhostConnection  (UServer * mainserver)
  : UConnection   (mainserver,
		   UGhostConnection::MINSENDBUFFERSIZE,
		   UGhostConnection::MAXSENDBUFFERSIZE,
		   UGhostConnection::PACKETSIZE,
		   UGhostConnection::MINRECVBUFFERSIZE,
		   UGhostConnection::MAXRECVBUFFERSIZE)
{
  ADDOBJ(UGhostConnection);
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
  closing = true;
  return USUCCESS;
}

//! Does nothing. No output for the ghosts...
int
UGhostConnection::effectiveSend(const ubyte *buffer, int length)
{
  char tmpbuf[1024];
  int real_length = length;
  if (real_length >= 1024)
    real_length = 1023;

  memcpy (static_cast<void*> (tmpbuf),
	  static_cast<const void*> (buffer),
	  real_length);
  tmpbuf[real_length] = 0;
  ::urbiserver->debug("%s", tmpbuf);

  return length;
}
