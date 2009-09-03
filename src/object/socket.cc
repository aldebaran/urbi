/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <kernel/userver.hh>

#include <object/global.hh>
#include <object/server.hh>
#include <object/socket.hh>
#include <object/symbols.hh>

namespace object
{
  Socket::Socket()
    : CxxObject()
    , libport::Socket(Socket::get_io_service())
    , server_()
    , disconnect_()
  {
    proto_add(Object::proto);
    // FIXME: call slots_create here when uobjects load order is fixed
  }

  Socket::Socket(rServer server)
    : CxxObject()
    , libport::Socket(Socket::get_io_service())
    , server_(server)
    , disconnect_()
  {
    proto_add(proto);
    init();
  }

  Socket::Socket(rSocket)
    : CxxObject()
    , libport::Socket(Socket::get_io_service())
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

#define BOUNCE(Type, From, To)                  \
  Type                                          \
  Socket::From() const                          \
  {                                             \
    if (isConnected())                          \
      return To();                              \
    else                                        \
      RAISE("unconnected socket");              \
  }
  BOUNCE(std::string, host, getRemoteHost);
  BOUNCE(unsigned short, port, getRemotePort);
#undef BOUNCE

  void
  Socket::slots_create()
  {
    CAPTURE_GLOBAL(Event);

#define EVENT(Name)                             \
    {                                           \
      rObject val = Event->call(SYMBOL(new));   \
      slot_set(Name, val);                      \
    }                                           \
/**/

    EVENT(SYMBOL(connected));
    EVENT(SYMBOL(disconnected));
    EVENT(SYMBOL(error));
    EVENT(SYMBOL(received));
#undef EVENT

  }

  void
  Socket::connect(const std::string& host, const std::string& port)
  {
    if (boost::system::error_code error = libport::Socket::connect(host, port))
      RAISE(error.message());
  }

  void
  Socket::connectSerial(const std::string& device, unsigned int baudrate)
  {
    if (boost::system::error_code error =
	libport::Socket::open_serial(device, baudrate))
      RAISE(error.message());
  }

  void
  Socket::disconnect()
  {
    close();
  }

#define EMIT(Name)                              \
  slot_get(SYMBOL(Name))->call(SYMBOL(emit))
#define EMIT1(Name, Arg)                                        \
  slot_get(SYMBOL(Name))->call(SYMBOL(emit), to_urbi(Arg))

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
    EMIT(disconnected);
    assert(disconnect_);
    disconnect_->call(SYMBOL(stop));
  }

  bool
  Socket::isConnected() const
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

  void
  Socket::poll()
  {
    boost::asio::io_service& ios = get_io_service();
    ios.reset();
    ios.poll();
  }

  URBI_CXX_OBJECT_REGISTER(Socket);

  void Socket::initialize(CxxObject::Binder<Socket>& bind)
  {
#define DECLARE(Name)                           \
    bind(SYMBOL(Name), &Socket::Name)

    DECLARE(connect);
    DECLARE(connectSerial);
    DECLARE(disconnect);
    DECLARE(host);
    DECLARE(init);
    DECLARE(isConnected);
    DECLARE(poll);
    DECLARE(port);
    DECLARE(write);
#undef DECLARE
  }

  boost::asio::io_service&
  Socket::get_io_service()
  {
    static boost::asio::io_service* ios = new boost::asio::io_service;
    return *ios;
  }

  rObject
  Socket::proto_make()
  {
    return new Socket();
  }
}
