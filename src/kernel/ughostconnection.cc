/*
 * Copyright (C) 2005-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file kernel/ughostconnection.cc
/// \brief Implementation of the UGhostConnection class.

#include <libport/cstring>
#include <libport/windows.hh>

#include <kernel/ughostconnection.hh>
#include <urbi/kernel/userver.hh>
#include <urbi/kernel/utypes.hh>

namespace kernel
{
  // Parameters used by the constructor, and other constants.
  enum
  {
    PACKETSIZE        = 32768,
    EFFECTIVESENDSIZE = 1024,
  };

  UGhostConnection::UGhostConnection(UServer& s, bool interactive)
    : UConnection(s, PACKETSIZE)
  {
    interactive_p(interactive);
    server_.connection_add(this);
  }

  UGhostConnection::~UGhostConnection()
  {
  }

  void
  UGhostConnection::close_()
  {
    kernel::urbiserver->interactive_set(false);
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
