#include <kernel/connection.hh>

#include <kernel/userver.hh>

#include <object/lobby.hh>
#include <object/symbols.hh>

namespace kernel
{

  Connection::Connection()
    : UConnection(*kernel::urbiserver, Connection::PACKET_SIZE)
  {
    if (uerror_ != USUCCESS)
      close();
  }

  int
  Connection::onRead(const void* data, size_t length)
  {
    this->received((const char*)data, length);
    return length;
  }

  void
  Connection::onConnect()
  {
    initialize();
    try {
      lobby_->slot_set(SYMBOL(remoteIP), object::to_urbi(getRemoteHost()));
    }
    catch(std::exception& e)
    {
      // We got disconnected, do nothing, onError will be called.
    }
  }

  void
  Connection::close()
  {
    // Closing the Socket will call our onError().
    libport::Socket::close();
  }

  void
  Connection::onError(boost::system::error_code)
  {
    closing_ = true;
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
      libport::Socket::send((const void *)buffer, length);
    /// FIXME: we claim to write OK to avoid buffering useless stuff.
    return length;
  }

}
