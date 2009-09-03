/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <object/global.hh>
#include <object/server.hh>
#include <object/symbols.hh>

namespace object
{
  Server::Server()
  : libport::Socket(object::Socket::get_io_service())
  {
    proto_add(Object::proto);
//    initialize();
  }

  Server::Server(rServer model)
  : libport::Socket(object::Socket::get_io_service())
  {
    proto_add(model);
    initialize();
  }

  void
  Server::initialize()
  {
    CAPTURE_GLOBAL(Event);
    connection_ = Event->call(SYMBOL(new));
    slot_set(SYMBOL(connection), connection_);
  }

#define BOUNCE(Type, From, To)                  \
  Type                                          \
  Server::From() const                          \
  {                                             \
    if (base_)                                  \
      return To();                              \
    else                                        \
      RAISE("server not listening");            \
  }
  BOUNCE(std::string, host, getLocalHost);
  BOUNCE(unsigned short, port, getLocalPort);
#undef BOUNCE

  libport::Socket*
  Server::make_socket()
  {
    rSocket sock = new object::Socket(this);
    sockets_.push_back(sock);
    return sock.get();
  }

  void
  Server::listen(const std::string& host, const std::string& port)
  {
    libport::Socket::listen(boost::bind(&Server::make_socket, this),
                            host, port);
  }

  void
  Server::socket_ready(rSocket socket)
  {
    connection_->call(SYMBOL(syncEmit), socket);
  }

  const Server::sockets_type&
  Server::sockets() const
  {
    return sockets_;
  }

  URBI_CXX_OBJECT_REGISTER(Server);

  void Server::initialize(CxxObject::Binder<Server>& bind)
  {
    bind(SYMBOL(host),    &Server::host);
    bind(SYMBOL(listen),  &Server::listen);
    bind(SYMBOL(port),    &Server::port);
    bind(SYMBOL(sockets), &Server::sockets);
  }

  rObject
  Server::proto_make()
  {
    return new Server();
  }

}
