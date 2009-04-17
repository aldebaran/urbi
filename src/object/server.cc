#include <object/global.hh>
#include <object/server.hh>
#include <object/symbols.hh>

namespace object
{
  Server::Server()
  {
    proto_add(Object::proto);
//    initialize();
  }

  Server::Server(rServer model)
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
    libport::Socket::listen(boost::bind(&Server::make_socket, this), host, port);
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
    bind(SYMBOL(listen), &Server::listen);
    bind(SYMBOL(sockets), &Server::sockets);
  }

  rObject
  Server::proto_make()
  {
    return new Server();
  }

}
