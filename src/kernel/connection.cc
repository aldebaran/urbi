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
    lobby_->slot_set(SYMBOL(remoteIP), object::to_urbi(getRemoteHost()));
  }

  void
  Connection::close()
  {
    libport::Socket::close();
    server_.connection_remove(this);
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
    libport::Socket::send((const void *)buffer, length);
    return length;
  }

}
