/*
 * Copyright (C) 2009-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef KERNEL_CONNECTION_HH
# define KERNEL_CONNECTION_HH

# include <urbi/kernel/uconnection.hh>
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
    virtual size_t onRead(const void* data, size_t length);

    virtual void onConnect();

    // kernel::UConnection virtual methods.
    virtual void onError(boost::system::error_code erc);
    virtual void endline();

    virtual size_t effective_send(const char* buffer, size_t length);
  protected:
    virtual void close_();
  };

}

#endif // !KERNEL_CONNECTION_HH
