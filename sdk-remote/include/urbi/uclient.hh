/*
 * Copyright (C) 2004, 2006-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/uclient.hh

#ifndef URBI_UCLIENT_HH
# define URBI_UCLIENT_HH

# include <libport/asio.hh>
# include <libport/pthread.h>
# include <libport/semaphore.hh>
# include <libport/utime.hh>

# include <urbi/uabstractclient.hh>

namespace urbi
{
  ///Linux implementation of UAbstractClient.
  /*! This implementation uses a shared thread between all the instances to
   * handle Socket operations, and call the registered callbacks in that thread.
  */
  class URBI_SDK_API UClient
    : public UAbstractClient
    , public libport::Socket
  {
  public:
    /// Construction options.
    struct URBI_SDK_API options
    {
      /// Backward compatibility with the previous UClient::UClient
      /// interface.  Don't make it "explicit" so that we can call
      /// "UClient(host, port)" and have the expected "server ==
      /// false".
      ///
      /// start defaults to true, for backward compatibility too.
      options(bool server = false);
# define UCLIENT_OPTION(Type, Name)             \
    public:                                     \
      options& Name(Type b);                    \
      Type Name() const;                        \
    private:                                    \
      Type Name ## _;

      /// Whether in server mode.
      UCLIENT_OPTION(bool, server);
      /// Whether the socket autostarts.
      UCLIENT_OPTION(bool, start);
      /// Wheteher the connection is established asynchronously.
      UCLIENT_OPTION(bool, asynchronous);
    };

# define UCLIENT_OPTION_IMPL(Class, Type, Name) \
  Class::options&                               \
  Class::options::Name(Type v)                  \
  {                                             \
    Name ## _ = v;                              \
    return *this;                               \
  }                                             \
                                                \
  Type                                          \
  Class::options::Name() const                  \
  {                                             \
    return Name ## _;                           \
  }

    /*! Create a new client and tries to connect to the server.
     * Will block until the connection is established or timeouts.
     * \param host    IP address or name of the robot to connect to.
     * \param port    TCP port to connect to.
     * \param buflen  size of send and receive buffers.
     * \param opt  options: whether server, whether autostart.
     */
    UClient(const std::string& host = default_host(),
            unsigned port = URBI_PORT,
	    size_t buflen = URBI_BUFLEN,
            // FIXME: See http://llvm.org/bugs/show_bug.cgi?id=8692,
            // revert this in the future.
            // FIXME: ndmefyl: changed from options::options(false) -
            // wtf a direct call to a ctor - which doesn't work - at
            // least with 4.5.1 - to option(false). If this is related
            // to the LLVM bug, please check you're not breaking gcc
            // compilation while fixing.
	    const UClient::options& opt = options(false));

    virtual ~UClient();

    virtual void waitForKernelVersion() const;

    /// Bounce to listen or connect, depending whether server mode.
    error_type start();
  protected:
    virtual error_type onClose();

  public:
    int closeUClient ();

    ATTRIBUTE_PRINTF(2, 3)
    virtual void printf(const char* format, ...);

    virtual unsigned int getCurrentTime() const;

    /// Activate KeepAlive functionality.
    /// \param pingInterval  is in milliseconds.
    /// \param pongTimeout   is in milliseconds.
    virtual void setKeepAliveCheck(unsigned pingInterval,
                                   unsigned pongTimeout);

    using UAbstractClient::send;

    /// Use synchronous or asynchronous send.
    void setSynchronousSend(bool enable);
  protected:
    virtual int effectiveSend(const void* buffer, size_t size);

    libport::Socket* mySocketFactory();

    virtual void onConnect();

    virtual void onError(boost::system::error_code erc);

    virtual size_t onRead(const void*, size_t length);

  protected:
    /// Delay (in microseconds) without activity to check if the
    /// connection is yet available.
    libport::utime_t ping_interval_;

    /// Delay (in microseconds) of timeout to wait 'PONG'.
    libport::utime_t pong_timeout_;

    typedef boost::shared_ptr<UClient*> link_type;
    link_type link_;

    /// Send timeout error.
    void pongTimeout(link_type l);

    void sendPing(link_type l);

    libport::utime_t ping_sent_;

    libport::AsyncCallHandler pong_timeout_handler_;
    libport::AsyncCallHandler send_ping_handler_;

    libport::Semaphore ping_sem_;

  private:
    /// Wrapper around Socket::connect.
    /// Client mode.
    error_type connect_();
    /// Wrapper around Socket::listen.
    /// Server mode.
    error_type listen_();
    /// Reset all asynchronous calls.
    void resetAsyncCalls_();
    /// Make an asynchronous connect.
    bool asynchronous_;
    /// Use synchronous/asynchronous send.
    bool synchronous_send_;
  };

} // namespace urbi
#endif
