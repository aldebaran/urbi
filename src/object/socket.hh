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
    void disconnect();
    void init();
    virtual void onConnect();
    virtual void onError(boost::system::error_code);
    virtual int  onRead(const void* data, size_t length);
    void write(const std::string& data);
    bool isConnected();
    static void poll();
    std::string host();

  private:
    void slots_create();
    rServer server_;
    rObject disconnect_;

    URBI_CXX_OBJECT(Socket);
  };
}

#endif
