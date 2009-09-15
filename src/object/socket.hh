/*
 * Copyright (C) 2009, Gostai S.A.S.
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

# include <object/cxx-object.hh>

namespace object
{
  class URBI_SDK_API Socket: public CxxObject, public libport::Socket
  {
  public:
    Socket();
    Socket(rServer server);
    Socket(rSocket model);
    void connect(const std::string& host, const std::string& port);
    void connectSerial(const std::string& device, unsigned int baudrate);
    void disconnect();
    void init();
    virtual void onConnect();
    virtual void onError(boost::system::error_code);
    virtual size_t onRead(const void* data, size_t length);
    void write(const std::string& data);
    bool isConnected() const;
    static void poll();
    std::string host() const;
    unsigned short port() const;
    static boost::asio::io_service& get_io_service();
  private:
    void slots_create();
    rServer server_;
    rObject disconnect_;
    URBI_CXX_OBJECT(Socket);
  };
}

#endif
