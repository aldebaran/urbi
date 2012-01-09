/*
 * Copyright (C) 2009-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <kernel/connection.hh>
#include <urbi/kernel/userver.hh>

#include <urbi/object/lobby.hh>
#include <urbi/object/symbols.hh>

namespace kernel
{

  Connection::Connection()
    : UConnection(*kernel::urbiserver, Connection::PACKET_SIZE)
    , Socket(kernel::urbiserver->get_io_service())
  {
    if (uerror_ != USUCCESS)
      UConnection::close();
  }

  size_t
  Connection::onRead(const void* data, size_t length)
  {
    this->received((const char*)data, length);
    return length;
  }

  void
  Connection::onConnect()
  {
    initialize();
    try
    {
      lobby_->slot_set(SYMBOL(remoteIP), object::to_urbi(getRemoteHost()));
    }
    catch(std::exception&)
    {
      // We got disconnected, do nothing, onError will be called.
    }
  }

  void
  Connection::close_()
  {
    // Closing the Socket will call our onError().
    libport::Socket::close();
  }

  void
  Connection::onError(boost::system::error_code)
  {
    if (!closing_)
      UConnection::close();
  }

  void
  Connection::endline()
  {
    //FIXME: test send error
    UConnection::send("\n");
    error_ = USUCCESS;
  }

  size_t
  Connection::effective_send(const char* buffer, size_t length)
  {
    if (!closing_)
      libport::Socket::send((const void*)buffer, length);
    /// FIXME: we claim to write OK to avoid buffering useless stuff.
    return length;
  }

}
