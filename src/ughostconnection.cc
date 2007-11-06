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

#include "kernel/utypes.hh"
#include "kernel/userver.hh"

#include "ughostconnection.hh"


// Parameters used by the constructor.

enum
{
  MINSENDBUFFERSIZE = 4096,
  MAXSENDBUFFERSIZE = 1048576,
  PACKETSIZE        = 32768,
  MINRECVBUFFERSIZE = 4096,
  MAXRECVBUFFERSIZE = 1048576,
};

//! UGhostConnection constructor.
UGhostConnection::UGhostConnection  (UServer* mainserver)
  : UConnection   (mainserver,
		   MINSENDBUFFERSIZE,
		   MAXSENDBUFFERSIZE,
		   PACKETSIZE,
		   MINRECVBUFFERSIZE,
		   MAXRECVBUFFERSIZE)
{
  ::urbiserver->connectionList.push_front (this);
}

//! UGhostConnection destructor.
UGhostConnection::~UGhostConnection()
{
  ::urbiserver->connectionList.remove (this);
}

//! Close the connection
/*! This function does nothing. The ghost connection cannot be closed.
*/
UConnection&
UGhostConnection::closeConnection()
{
  closing = true;
  CONN_ERR_RET(USUCCESS);
}

//! Does nothing. No output for the ghosts...
int
UGhostConnection::effectiveSend(const ubyte *buffer, int length)
{
  char buf[1024];
  int len = std::min (length, static_cast<int>(sizeof buf) - 1);

  memcpy (static_cast<void*> (buf), static_cast<const void*> (buffer),
	  len);
  buf[len] = 0;
  ::urbiserver->debug("%s", buf);

  return length;
}

//! Send a "\n" through the connection
UConnection&
UGhostConnection::endline ()
{
  //FIXME: test send error
  (*this) << send((const ubyte*)"\n", 1);
  CONN_ERR_RET(USUCCESS);
}
