/// \file liburbi/uclient.cc

#if !defined WIN32
# include <time.h>
# include <signal.h>
#endif

#include <libport/boost-error.hh>

#include <urbi/uclient.hh>
//#include <urbi/utag.hh>

namespace urbi
{
  /*! Establish the connection with the server.
   */
  UClient::UClient(const std::string& host, unsigned port,
                   size_t buflen, bool server)
    : UAbstractClient(host, port, buflen, server)
    , ping_interval_(0)
    , pong_timeout_(0)
  {
    boost::system::error_code erc;

    if (!server_)
    {
      if ((erc = connect(host, port)))
        libport::boost_error("UClient::UClient connect", erc);
    }
    else if ((erc = listen(boost::bind(&urbi::UClient::mySocketFactory, this),
                           host, port)))
    {
      libport::boost_error("UClient::UClient cannot listen", erc);
      return;
    }
  }

  UClient::~UClient()
  {
  }

  int
  UClient::effectiveSend(const void* buffer, size_t size)
  {
    if (rc)
      return -1;
    libport::Socket::write(buffer, size);
    return 0;
  }

  libport::Socket*
  UClient::mySocketFactory()
  {
    return this;
  }

  void
  UClient::onConnect()
  {
    init_ = true;
    onConnection();

   waitingPong = false;

    // Declare ping channel for kernel that requires it.  Do not try
    // to depend on kernelMajor, because it has not been computed yet.
    // And computing kernelMajor requires this code to be run.  So we
    // need to write something that both k1 and k2 will like.
    send("if (isdef(Channel))\n"
         "  var lobby.%s = Channel.new(\"%s\") | {};",
         internalPongTag, internalPongTag);
  }

  void
  UClient::onError(boost::system::error_code erc)
  {
    rc = -1;
    clientError(erc.message());
    notifyCallbacks(UMessage(*this, 0, connectionTimeoutTag, erc.message().c_str()));
    return;
  }

  int
  UClient::onRead(const void* data, size_t length)
  {
    size_t capacity = buflen - recvBufferPosition - 1;
    size_t eat = std::min(capacity, length);

    memcpy(&recvBuffer[recvBufferPosition], data, eat);
    recvBufferPosition += eat;
    recvBuffer[recvBufferPosition] = 0;
    processRecvBuffer();
    return eat;
  }

  void
  UClient::printf(const char * format, ...)
  {
    va_list arg;
    va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
  }

  unsigned int UClient::getCurrentTime() const
  {
  // FIXME: Put this into libport.
#ifdef WIN32
    return GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000+tv.tv_usec/1000;
#endif
  }

  void
  UClient::setKeepAliveCheck(unsigned pingInterval,
                             unsigned pongTimeout)
  {
    // From milliseconds to microseconds.
    ping_interval_ = pingInterval * 1000;
    pong_timeout_  = pongTimeout * 1000;
  }


/*-----------------------.
| Standalone functions.  |
`-----------------------*/

  void execute()
  {
    while (true)
      sleep(100);
  }

  void exit(int code)
  {
    ::exit(code);
  }

  UClient&
  connect(const std::string& host)
  {
    return *new UClient(host);
  }

  void disconnect(UClient &client)
  {
    delete &client;
  }

} // namespace urbi
