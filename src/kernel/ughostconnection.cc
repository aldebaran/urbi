/*! \file kernel/ughostconnection.cc
 *******************************************************************************

 File: kernel/ughostconnection.cc\n
 Implementation of the UGhostConnection class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 Copyright (c) 2004-2009, Jean-Christophe Baillie.

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

namespace kernel
{
  // Parameters used by the constructor, and other constants.
  enum
  {
    PACKETSIZE        = 32768,
    EFFECTIVESENDSIZE = 1024,
  };

  UGhostConnection::UGhostConnection(UServer& s)
    : UConnection(s, PACKETSIZE)
  {
    server_.connection_add(this);
  }

  UGhostConnection::~UGhostConnection()
  {
  }

  void
  UGhostConnection::close()
  {
    closing_ = true;
    error_ = USUCCESS;
  }

  size_t
  UGhostConnection::effective_send(const char* buffer, size_t length)
  {
    char buf[EFFECTIVESENDSIZE];

    for (size_t i = 0; i < length; i += EFFECTIVESENDSIZE - 1)
    {
      size_t len = std::min(length - i, size_t(EFFECTIVESENDSIZE - 1));
      memcpy(static_cast<void*>(buf), static_cast<const void*>(buffer + i),
             len);
      buf[len] = 0;
      urbiserver->display(buf);
    }

    return length;
  }

  void
  UGhostConnection::endline()
  {
    //FIXME: test send error
    send("\n");
    error_ = USUCCESS;
  }
}
