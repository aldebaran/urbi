#include <object/global.hh>
#include <object/server.hh>
#include <object/socket.hh>
#include <object/symbols.hh>

namespace object
{

  Socket::Socket()
    : CxxObject()
    , libport::Socket()
    , server_()
    , disconnect_()
  {
    proto_add(Object::proto);
    // FIXME: call slots_create here when uobjects load order is fixed
  }

  Socket::Socket(rServer server)
    : CxxObject()
    , libport::Socket()
    , server_(server)
    , disconnect_()
  {
    proto_add(proto);
    init();
  }

  Socket::Socket(rSocket)
    : CxxObject()
    , libport::Socket()
    , server_()
    , disconnect_()
  {
    proto_add(proto);
    // FIXME: call slots_create here when uobjects load order is fixed
  }

  void
  Socket::init()
  {
    slots_create();
  }

  std::string
  Socket::host()
  {
    return getRemoteHost();
  }

  void
  Socket::slots_create()
  {
    CAPTURE_GLOBAL(Event);

#define EVENT(Name)                                                     \
    {                                                                   \
      rObject val = Event->call(SYMBOL(new));                           \
      slot_set(SYMBOL(Name), val);                                      \
    }                                                                   \

    EVENT(connected);
    EVENT(disconnected);
    EVENT(error);
    EVENT(received);
#undef event

  }

  void
  Socket::connect(const std::string& host, const std::string& port)
  {
    if (boost::system::error_code error = libport::Socket::connect(host, port))
      runner::raise_primitive_error(error.message());
  }

#define EMIT(Name)                              \
  slot_get(SYMBOL(Name))->call(SYMBOL(emit));
#define EMIT1(Name, Arg)                                        \
  slot_get(SYMBOL(Name))->call(SYMBOL(emit), to_urbi(Arg));

  void
  Socket::onConnect()
  {
    disconnect_ = slot_get(SYMBOL(connected))->call(SYMBOL(trigger));
    if (server_)
      server_->socket_ready(this);
  }

  void
  Socket::onError(boost::system::error_code err)
  {
    EMIT1(error, err.message());
//     if (!isConnected())
    assert(disconnect_);
    disconnect_->call(SYMBOL(stop));
  }

  bool
  Socket::isConnected()
  {
    return libport::Socket::isConnected();
  }

  int
  Socket::onRead(const void* data, size_t length)
  {
    std::string str(reinterpret_cast<const char*>(data), length);
    EMIT1(received, str);
    return length;
  }

  void
  Socket::write(const std::string& data)
  {
    assert(isConnected());
    libport::Socket::send(data);
  }

  static boost::asio::io_service* ios = 0;
  void
  Socket::poll()
  {
    ios->reset();
    ios->poll();
  }

  URBI_CXX_OBJECT_REGISTER(Socket);

  void Socket::initialize(CxxObject::Binder<Socket>& bind)
  {
    bind(SYMBOL(connect), &Socket::connect);
    bind(SYMBOL(host),    &Socket::host);
    bind(SYMBOL(init),    &Socket::init);
    bind(SYMBOL(isConnected), &Socket::isConnected);
    bind(SYMBOL(poll),    &Socket::poll);
    bind(SYMBOL(write),   &Socket::write);
  }

  rObject
  Socket::proto_make()
  {
    ios = &libport::get_io_service(false);
    return new Socket();
  }
}
