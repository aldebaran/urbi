/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <kernel/userver.hh>

#include <urbi/object/global.hh>
#include <object/ioservice.hh>
#include <object/server.hh>
#include <object/socket.hh>
#include <object/symbols.hh>

namespace urbi
{
  namespace object
  {
        IoService::IoService()
      : CxxObject()
    {
      proto_add(Object::proto);
      // FIXME: call slots_create here when uobjects load order is fixed
    }

    IoService::IoService(rIoService)
      : CxxObject()
    {
      proto_add(proto);
    }


    void IoService::initialize(CxxObject::Binder<IoService>& bind)
    {
      bind(SYMBOL(pollFor), &IoService::pollFor);
      bind(SYMBOL(pollOneFor), &IoService::pollOneFor);
      bind(SYMBOL(poll), &IoService::poll);
      bind(SYMBOL(makeServer), &IoService::makeServer);
      bind(SYMBOL(makeSocket), &IoService::makeSocket);
    }

    void IoService::pollFor(double d)
    {
      libport::pollFor(static_cast<useconds_t>(d*1000000.0), false, *this);
    }

    void IoService::pollOneFor(double d)
    {
      libport::pollFor(static_cast<useconds_t>(d*1000000.0), true, *this);
    }

    void IoService::poll()
    {
      boost::asio::io_service::reset();
      boost::asio::io_service::poll();
    }

    rSocket IoService::makeSocket()
    {
      return rSocket(new Socket(this));
    }

    rServer IoService::makeServer()
    {
      return rServer(new Server(this));
    }

    URBI_CXX_OBJECT_REGISTER(IoService)
    {
    }
  }
}
