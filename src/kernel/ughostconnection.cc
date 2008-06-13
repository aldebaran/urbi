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

#include <libport/cstring>
#include <libport/windows.hh>

#include <kernel/utypes.hh>
#include <kernel/userver.hh>
#include <kernel/ughostconnection.hh>


// Parameters used by the constructor, and other constants.

enum
{
  MINSENDBUFFERSIZE = 4096,
  MAXSENDBUFFERSIZE = 1048576,
  PACKETSIZE        = 32768,
  MINRECVBUFFERSIZE = 4096,
  MAXRECVBUFFERSIZE = 1048576,

  EFFECTIVESENDSIZE = 1024,
};

//! UGhostConnection constructor.
UGhostConnection::UGhostConnection (UServer& s)
  : UConnection(s, PACKETSIZE)
{
  server_.connection_add(this);
}

//! UGhostConnection destructor.
UGhostConnection::~UGhostConnection()
{
  server_.connection_remove(this);
}

//! Close the connection
/*! This function does nothing. The ghost connection cannot be closed.
*/
UConnection&
UGhostConnection::close()
{
  closing_ = true;
  CONN_ERR_RET(USUCCESS);
}

//! Does nothing. No output for the ghosts...
size_t
UGhostConnection::effective_send(const char* buffer, size_t length)
{
  char buf[EFFECTIVESENDSIZE];

  for (size_t i = 0; i < length; i += EFFECTIVESENDSIZE - 1)
  {
    size_t len = std::min (length - i, size_t(EFFECTIVESENDSIZE - 1));
    memcpy (static_cast<void*> (buf), static_cast<const void*> (buffer + i),
	    len);
    buf[len] = 0;
    ::urbiserver->debug("%s", buf);
  }

  return length;
}

//! Send a "\n" through the connection
UConnection&
UGhostConnection::endline ()
{
  //FIXME: test send error
  send("\n");
  CONN_ERR_RET(USUCCESS);
}
