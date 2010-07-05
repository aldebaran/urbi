/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_SOCKET_HH
# define OBJECT_SOCKET_HH

# include <libport/asio.hh>

# include <urbi/object/cxx-object.hh>

# include <object/ioservice.hh>
# include <object/server.hh>

namespace urbi
{
  namespace object
  {

    class URBI_SDK_API Socket: public CxxObject, public libport::Socket
    {
      public:
      Socket();
      Socket(rServer server);
      Socket(rSocket model);
      Socket(rIoService io_service);
      void connect(const std::string& host, const std::string& port);
      void connect(const std::string& host, unsigned port);
      // Default argument values is handled with overloads for bound functions.
      void connectSerial(const std::string& device, unsigned int baudrate);
      void connectSerial(const std::string& device, unsigned int baudrate,
                         bool asyncRead);
      void disconnect();
      void init();
      virtual void onConnect();
      virtual void onError(boost::system::error_code);
      virtual size_t onRead(const void* data, size_t length);
      void write(const std::string& data);
      void syncWrite(const std::string& data);
      std::string read(size_t len);
      bool isConnected() const;
      void poll();
      std::string host() const;
      unsigned short port() const;
      std::string localHost() const;
      unsigned short localPort() const;
      rIoService getIoService() const;
      static rIoService get_default_io_service();
      private:
      void slots_create();
      rServer server_;
      rObject disconnect_;
      rIoService io_service_;
      URBI_CXX_OBJECT_(Socket);
    };
  }
}

#endif
