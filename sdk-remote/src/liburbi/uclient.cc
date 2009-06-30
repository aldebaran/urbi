/// \file liburbi/uclient.cc

#if !defined WIN32
# include <time.h>
# include <signal.h>
#endif

#include <boost/lambda/bind.hpp>

#include <libport/boost-error.hh>
#include <libport/format.hh>

#include <urbi/uclient.hh>

namespace urbi
{

  /*-------------------.
  | UClient::options.  |
  `-------------------*/

  UClient::options::options(bool server)
    : server_(server)
      // Unless stated otherwise, auto start.
    , start_(true)
  {
  }

  UCLIENT_OPTION_IMPL(UClient, bool, server)
  UCLIENT_OPTION_IMPL(UClient, bool, start)

  /*----------.
  | UClient.  |
  `----------*/

  UClient::UClient(const std::string& host, unsigned port,
                   size_t buflen,
                   const options& opt)
    : UAbstractClient(host, port, buflen, opt.server())
    , ping_interval_(0)
    , pong_timeout_(0)
  {
    if (opt.start())
      start();
  }

  UClient::error_type
  UClient::start()
  {
    return rc = server_ ? listen_() : connect_();
  }

  UClient::error_type
  UClient::connect_()
  {
    if (boost::system::error_code erc = connect(host_, port_))
    {
      libport::boost_error(libport::format("UClient::UClient connect(%s, %s)",
                                           host_, port_),
                           erc);
      return -1;
    }
    else
      return 0;
  }

  UClient::error_type
  UClient::listen_()
  {
    if (boost::system::error_code erc =
        listen(boost::bind(&UClient::mySocketFactory, this), host_, port_))
    {
      libport::boost_error(libport::format("UClient::UClient listen(%s, %s)",
                                           host_, port_),
                           erc);
      return -1;
    }
    else
      return 0;
  }

  UClient::~UClient()
  {
    closeUClient();
  }

  UClient::error_type
  UClient::onClose()
  {
    if (closed_)
      return 1;
    UAbstractClient::onClose();
    return 0;
  }

  UClient::error_type
  UClient::closeUClient()
  {
    close();
    onClose();
    return 0;
  }

  UClient::error_type
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
    host_ = getRemoteHost();
    port_ = getRemotePort();
  }

  void
  UClient::onError(boost::system::error_code erc)
  {
    rc = -1;
    clientError(erc.message());
    notifyCallbacks(UMessage(*this, 0, connectionTimeoutTag,
                             erc.message().c_str()));
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
