/*
 * Copyright (C) 2005-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file liburbi/uclient.cc

#if !defined WIN32
# include <libport/ctime>
# include <libport/csignal>
#endif

#include <boost/lambda/bind.hpp>

#include <libport/boost-error.hh>
#include <libport/format.hh>

#include <urbi/uclient.hh>
#include <urbi/utag.hh>

#include <liburbi/compatibility.hh>

GD_CATEGORY(Urbi.Client);

namespace urbi
{

  /*-------------------.
  | UClient::options.  |
  `-------------------*/

  UClient::options::options(bool server)
    : server_(server)
      // Unless stated otherwise, auto start.
    , start_(true)
    , asynchronous_(false)
  {
  }

  UCLIENT_OPTION_IMPL(UClient, bool, server)
  UCLIENT_OPTION_IMPL(UClient, bool, start)
  UCLIENT_OPTION_IMPL(UClient, bool, asynchronous)

  /*----------.
  | UClient.  |
  `----------*/

  UClient::UClient(const std::string& host, unsigned port,
                   size_t buflen,
                   const options& opt)
    : UAbstractClient(host, port, buflen, opt.server())
    , ping_interval_(0)
    , pong_timeout_(0)
    , link_(new UClient*(this))
    , ping_sent_(libport::utime())
    , ping_sem_(0)
    , asynchronous_(opt.asynchronous())
    , synchronous_send_(false)
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
    if (boost::system::error_code erc = connect(host_, port_, false, 0,
                                                asynchronous_))
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
    *link_ = 0;
    closeUClient();
  }

  UClient::error_type
  UClient::onClose()
  {
    if (!closed_)
      UAbstractClient::onClose();
    return !!closed_;
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
    if (synchronous_send_)
      libport::Socket::syncWrite(buffer, size);
    else
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

    // Declare ping channel for kernel that requires it.  Do not try
    // to depend on kernelMajor, because it has not been computed yet.
    // And computing kernelMajor requires this code to be run.  So we
    // need to write something that both k1 and k2 will like.
    send(SYNCLINE_WRAP(
           "if (isdef(Channel))\n"
           "  var lobby.%s = Channel.new(\"%s\")|;",
           internalPongTag, internalPongTag));
    // The folowwing calls may fail if we got disconnected.
    try
    {
      host_ = getRemoteHost();
      port_ = getRemotePort();
    }
    catch (const std::exception& e)
    {
      // Ignore the error, next read attempt will trigger onError.
      GD_FINFO_DUMP("ignore std::exception: %s", e.what());
    }
    if (ping_interval_)
      sendPing(link_);
  }

  void
  UClient::onError(boost::system::error_code erc)
  {
    rc = -1;
    resetAsyncCalls_();
    clientError("!!! " + erc.message());
    notifyCallbacks(UMessage(*this, 0, CLIENTERROR_TAG,
                             "!!! " + erc.message()));
    return;
  }

  size_t
  UClient::onRead(const void* data, size_t length)
  {
    size_t capacity = recvBufSize - recvBufferPosition - 1;

    if (ping_interval_ && ping_sem_.uget(1))
    {
      pong_timeout_handler_->cancel();
      send_ping_handler_ =
        libport::asyncCall(boost::bind(&UClient::sendPing,
                                       this, link_),
                           ping_interval_ - (libport::utime() - ping_sent_));
    }
    if (capacity < length)
    {
      size_t nsz = std::max(recvBufSize*2, recvBufferPosition + length+1);
      char* nbuf = new char[nsz];
      memcpy(nbuf, recvBuffer, recvBufferPosition);
      delete[] recvBuffer;
      recvBuffer = nbuf;
      recvBufSize = nsz;
    }
    memcpy(&recvBuffer[recvBufferPosition], data, length);
    recvBufferPosition += length;
    recvBuffer[recvBufferPosition] = 0;
    processRecvBuffer();
    return length;
  }

  void
  UClient::pongTimeout(link_type l)
  {
    if (*l)
    {
      const char* err = "!!! Lost connection with server: ping timeout";
      // FIXME: Choose between two differents way to alert user program.
      clientError(err);
      notifyCallbacks(UMessage(*this, 0, connectionTimeoutTag, err));
      close();
    }
  }

  void
  UClient::sendPing(link_type l)
  {
    if (*l)
    {
      pong_timeout_handler_ =
        libport::asyncCall(boost::bind(&UClient::pongTimeout, this, link_),
                           pong_timeout_);
      send("%s << 1,", internalPongTag);
      ping_sent_ = libport::utime();
      ping_sem_++;
    }
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
  UClient::setKeepAliveCheck(unsigned ping_interval,
                             unsigned pong_timeout)
  {
    // Always interrupt previous ping handler.
    resetAsyncCalls_();
    // From milliseconds to microseconds.
    ping_interval_ = ping_interval * 1000;
    pong_timeout_  = pong_timeout * 1000;
    if (ping_interval_)
      sendPing(link_);
  }

  void
  UClient::resetAsyncCalls_()
  {
    if (pong_timeout_handler_)
    {
      pong_timeout_handler_->cancel();
      pong_timeout_handler_.reset();
    }
    if (send_ping_handler_)
    {
      send_ping_handler_->cancel();
      send_ping_handler_.reset();
    }
  }

  void
  UClient::waitForKernelVersion() const
  {
    // FIXME: use a condition.
    while (kernelMajor_ < 0 && !error())
      sleep(100000);
  }

  void
  UClient::setSynchronousSend(bool enable)
  {
    synchronous_send_ = enable;
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
    // Asynchronous deletion to let our async handlers terminate.
    client.destroy();
  }

} // namespace urbi
