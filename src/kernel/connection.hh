#ifndef KERNEL_CONNECTION_HH
# define KERNEL_CONNECTION_HH

# include <kernel/uconnection.hh>
# include <libport/asio.hh>

namespace kernel
{

  class URBI_SDK_API Connection
    : public UConnection
    , public libport::Socket
  {
  public:
    Connection();

    enum { PACKET_SIZE = 1000000000 };

    // libport::Socket virtual methods.
    virtual int onRead(const void* data, size_t length);

    virtual void onConnect();

    // kernel::UConnection virtual methods.
    virtual void close();
    virtual void onError(boost::system::error_code erc);
    virtual void endline();

    virtual size_t effective_send(const char* buffer, size_t length);
  };

}

#endif // !KERNEL_CONNECTION_HH
