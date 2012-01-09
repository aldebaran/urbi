/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/kernel/userver.hh>

#include <urbi/object/global.hh>
#include <object/ioservice.hh>
#include <object/server.hh>
#include <object/socket.hh>
#include <urbi/object/symbols.hh>

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

    URBI_CXX_OBJECT_INIT(IoService)
    {
#define DECLARE(Name, Cxx)                \
      bind(SYMBOL_(Name), &IoService::Cxx)

      DECLARE(pollFor,    pollFor);
      DECLARE(pollOneFor, pollOneFor);
      DECLARE(poll,       poll);
      DECLARE(makeServer, makeServer);
      DECLARE(makeSocket, makeSocket);

#undef DECLARE
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

  }
}
