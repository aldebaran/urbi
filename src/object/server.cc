/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/object/global.hh>
#include <object/ioservice.hh>
#include <object/server.hh>
#include <object/socket.hh>
#include <urbi/object/symbols.hh>

namespace urbi
{
  namespace object
  {
    Server::Server()
      : libport::Socket(*object::Socket::get_default_io_service().get())
      , io_service_(object::Socket::get_default_io_service())
    {
      proto_add(Object::proto);
//    initialize();
    }

    Server::Server(rServer model)
      : libport::Socket(*object::Socket::get_default_io_service().get())
      , io_service_(object::Socket::get_default_io_service())
    {
      proto_add(model);
      initialize();
    }

    Server::Server(rIoService io_service)
      : libport::Socket(*io_service.get())
      , io_service_(io_service)
    {
      proto_add(proto);
      initialize();
    }

    URBI_CXX_OBJECT_INIT(Server)
      : libport::Socket(*object::Socket::get_default_io_service().get())
      , io_service_(object::Socket::get_default_io_service())
    {
#define DECLARE(Name, Cxx)             \
      bind(SYMBOL_(Name), &Server::Cxx)

      DECLARE(getIoService, getIoService);
      DECLARE(host,         host);
      DECLARE(listen,       listen);
      DECLARE(port,         port);
      DECLARE(sockets,      sockets);

#undef DECLARE
    }

    void
    Server::initialize()
    {
      CAPTURE_GLOBAL(Event);
      connection_ = Event->call(SYMBOL(new));
      slot_set(SYMBOL(connection), connection_);
    }

#define BOUNCE(Type, From, To)                  \
    Type                                        \
    Server::From() const                        \
    {                                           \
      if (base_)                                \
        return To();                            \
      else                                      \
        RAISE("server not listening");          \
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
      boost::system::error_code err =
        libport::Socket::listen(boost::bind(&Server::make_socket, this),
                                host, port);
      if (err)
        RAISE(err.message());
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

    rIoService Server::getIoService() const
    {
      return io_service_;
    }
  }
}
