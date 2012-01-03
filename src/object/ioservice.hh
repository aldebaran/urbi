/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_IOSERVICE_HH
# define OBJECT_IOSERVICE_HH

# include <libport/asio.hh>

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API IoService: public CxxObject,
      public boost::asio::io_service
    {
      public:
      IoService();
      IoService(rIoService model);
      void pollFor(double duration);
      void pollOneFor(double duration);
      void poll();
      rSocket makeSocket();
      rServer makeServer();
      private:
      URBI_CXX_OBJECT(IoService, CxxObject);
    };
  }
}
#endif
