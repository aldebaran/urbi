/*
 * Copyright (C) 2005-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/unistd.h>
#include <libport/fcntl.h>

#include <libport/cassert>
#include <libport/compiler.hh>
#include <libport/debug.hh>
#include <libport/thread.hh>
#include <libport/unistd.h>

#include <liburbi/compatibility.hh>
#include <urbi/uconversion.hh>
#include <urbi/umessage.hh>
#include <urbi/usyncclient.hh>

GD_CATEGORY(Urbi.Client.Sync);

namespace urbi
{

  USyncClient::options::options()
    : super_type::options()
    , startCallbackThread_(true)
    , connectCallback_(0)
  {}

  const USyncClient::send_options USyncClient::send_options::default_options =
    USyncClient::send_options();

  USyncClient::send_options::send_options()
    : timeout_(0)
    , mtag_(0)
    , mmod_(0)
  {}

  USyncClient::send_options&
  USyncClient::send_options::timeout(libport::utime_t usec)
  {
    timeout_ = usec;
    return *this;
  }

  USyncClient::send_options&
  USyncClient::send_options::tag(const char* tag, const char* mod)
  {
    mtag_ = tag;
    mmod_ = mod;
    return *this;
  }

  UCLIENT_OPTION_IMPL(USyncClient, bool, startCallbackThread);
  UCLIENT_OPTION_IMPL(USyncClient, USyncClient::connect_callback_type,
                      connectCallback);

  USyncClient::USyncClient(const std::string& host,
			   unsigned port,
			   size_t buflen,
                           const options& opts)
    // Be cautious not to start the UClient part of this USyncClient
    // before the completion of "this".  If you do (e.g., you forget
    // to pass "start(false)" to UClient), then the UClient, as a
    // libport::Socket, is likely to receive messages that will be
    // bounced to USyncClient::onRead, as expected for virtual
    // functions.  Except that "this" is not a valid USyncClient, as
    // its members we not initialized.
    //
    // Therefore, start handling connection only in the body of the
    // ctor.
    : UClient(host, port, buflen,
              UClient::options()
                .server(opts.server())
                .asynchronous(opts.asynchronous())
                .start(false))
    , sem_()
    , queueLock_()
    , message_(0)
    , syncLock_()
    , syncTag()
    , default_options_()
    , stopCallbackThread_(!opts.startCallbackThread())
    , cbThread(0)
    , connectCallback_(opts.connectCallback())
    , synchronous_(false)
  {
    // Do not start if connectCallback_ is set, we were constructed by a
    // listening socket.
    if (!connectCallback_)
      start();
    if (error())
      return;

    if (opts.startCallbackThread())
      cbThread = libport::startThread(this, &USyncClient::callbackThread);
    if (!defaultClient)
      defaultClient = this;

    callbackSem_++;
  }

  USyncClient::~USyncClient()
  {
    // Notify of destruction
    wasDestroyed();
    // Close the socket
    close();
    // Wait for our message handler thread to terminate
    if (cbThread)
      joinCallbackThread_();
    // Wait for all asio async handlers to terminate
    waitForDestructionPermission();
  }

  void USyncClient::callbackThread()
  {
    callbackSem_--;

    while (true)
    {
      sem_--;
      if (stopCallbackThread_)
      {
	// The call to stopCallbackThread is
	// waiting on stopCallbackSem_.
	stopCallbackSem_++;
	return;
      }
      queueLock_.lock();
      if (queue.empty())
      {
	// Only explanation: processEvents from another thread stole our
	// message.
	sem_++; // Give back the token we took without popping a message.
	queueLock_.unlock();
	continue;
      }
      UMessage *m = queue.front();
      queue.pop_front();
      queueLock_.unlock();
      UAbstractClient::notifyCallbacks(*m);
      delete m;
    }
  }

  void USyncClient::stopCallbackThread()
  {
    if (stopCallbackThread_)
      return;
    stopCallbackThread_ = true;
    sem_++;
    // Unlock any pending syncGet.
    syncLock_++;
    // Wait until the callback thread is actually stopped to avoid both
    // processEvents and the callbackThread running at the same time.
    stopCallbackSem_--;
  }

  bool USyncClient::processEvents(libport::utime_t timeout)
  {
    bool res = false;
    libport::utime_t startTime = libport::utime();
    do // Always check at least once.
    {
      queueLock_.lock();
      if (queue.empty())
      {
	queueLock_.unlock();
	return res;
      }
      res = true;
      UMessage *m = queue.front();
      queue.pop_front();
      sem_--; // Will not block since queue was not empty.
      queueLock_.unlock();
      UAbstractClient::notifyCallbacks(*m);
      delete m;
    } while (timeout < 0 || libport::utime() - startTime <= timeout);
    return res;
  }

  int USyncClient::joinCallbackThread_()
  {
    stopCallbackThread();
    if (cbThread)
    {
      PTHREAD_RUN(pthread_join, cbThread, 0);
      cbThread = 0;
    }
    return 0;
  }

  void
  USyncClient::notifyCallbacks(const UMessage& msg)
  {
    queueLock_.lock();
    // If waiting for a tag, pass it to the user.
    if (!syncTag.empty() && syncTag == msg.tag)
    {
      message_ = new UMessage(msg);
      syncTag.clear();
      if (waitingFromPollThread_)
        libport::get_io_service().stop();
      else
        syncLock_++;
    }
    else if (synchronous_)
      UClient::notifyCallbacks(msg);
    else
    {
      queue.push_back(new UMessage(msg));
      sem_++;
    }
    queueLock_.unlock();
  }

  UMessage*
  USyncClient::waitForTag(const std::string& tag, libport::utime_t useconds)
  {
    if (message_ || !syncTag.empty())
      throw std::runtime_error("Another waitForTag is already in progress");
    syncTag = tag;
    message_ = 0;
    waitingFromPollThread_ = libport::isPollThread();
    // Reset before releasing the lock, as other thread may call io.stop()
    if (waitingFromPollThread_)
      libport::get_io_service().reset();
    queueLock_.unlock();

    // syncTag is reset by the other thread.
    if (!waitingFromPollThread_)
      syncLock_.uget(useconds);
    else if (useconds)
      libport::pollFor(useconds);
    else
      libport::get_io_service().run();

    UMessage *res = message_;
    if (!res)
      GD_ERROR("Timed out");
    else if (res->type == MESSAGE_ERROR)
      GD_FERROR("Received error message: %s", *res);
    message_ = 0;
    syncTag.clear();
    return res;
  }

  namespace
  {
    /// Check that \a cp looks like "foo < " or "foo :".
    /// I venture this is an attempt to see if there is a "tag" or a
    /// channel.
    static
    bool
    has_tag(const char* cp)
    {
      while (*cp == ' ')
        ++cp;
      while (isalpha(*cp))
        ++cp;
      while (*cp == ' ')
        ++cp;
      return *cp == ':' || *cp == '<';
    }

    /// Return the concatenation of t1 and t2, make it unique
    /// if they are empty.
    static
    std::string
    make_tag(UAbstractClient& cl, const USyncClient::send_options& opt)
    {
      std::string res;
      if (opt.mtag_)
      {
        res = opt.mtag_;
        if (opt.mmod_)
          res += opt.mmod_;
      }
      else
        res = cl.fresh();
      return res;
    }
  }

  USyncClient::error_type
  USyncClient::onClose()
  {
    if (closed_)
      return 1;

    UClient::onClose();

    stopCallbackThread_ = true;
    callbackSem_++;
    sem_++;
    return 0;
  }

  UMessage*
  USyncClient::syncGet_(const char* format, va_list& arg,
			const USyncClient::send_options& options)
  {
    const USyncClient::send_options& opt_used = getOptions(options);
    if (has_tag(format))
      return 0;
    sendBufferLock.lock();
    std::string tag = make_tag(*this, opt_used);
    pack("%s", compatibility::evaluate_in_channel_open
         (tag, kernelMajor()).c_str());
    rc = vpack(format, arg);
    if (rc < 0)
    {
      sendBufferLock.unlock();
      return 0;
    }
    pack("%s", compatibility::evaluate_in_channel_close
         (tag, kernelMajor()).c_str());
    queueLock_.lock();
    rc = effective_send(sendBuffer);
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    return waitForTag(opt_used.mtag_ ? opt_used.mtag_ : tag,
                      opt_used.timeout_);
  }

  UMessage*
  USyncClient::syncGet(const char* format, ...)
  {
    va_list arg;
    va_start(arg, format);
    UMessage* res = syncGet_(format, arg);
    va_end(arg);
    return res;
  }

  UMessage*
  USyncClient::syncGet(const std::string& msg)
  {
    // Yes, this is studid, as it will copy uselessly.  But that's the
    // only safe way to do it.  The interface should be redesigned,
    // without buffers actually.
    return syncGet("%s", msg.c_str());
  }

  UMessage*
  USyncClient::syncGet(libport::utime_t useconds,
                       const char* format, ...)
  {
    va_list arg;
    va_start(arg, format);
    UMessage* res = syncGet_(format, arg, send_options().timeout(useconds));
    va_end(arg);
    return res;
  }

  UMessage*
  USyncClient::syncGetTag(const char* format,
                          const char* mtag, const char* mmod, ...)
  {
    va_list arg;
    va_start(arg, mmod);
    UMessage* res = syncGet_(format, arg, send_options().tag(mtag, mmod));
    va_end(arg);
    return res;
  }

  UMessage*
  USyncClient::syncGetTag(libport::utime_t useconds,
                          const char* format,
                          const char* mtag,
                          const char* mmod, ...)
  {
    va_list arg;
    va_start(arg, mmod);
    UMessage* res = syncGet_(format, arg,
			     send_options().tag(mtag, mmod).timeout(useconds));
    va_end(arg);
    return res;
  }

  int
  USyncClient::syncGetImage(const char* camera,
			    void* buffer, size_t& buffersize,
			    int format, int transmitFormat,
			    size_t& width, size_t& height,
			    libport::utime_t useconds)
  {
    int f = format == IMAGE_JPEG || transmitFormat == URBI_TRANSMIT_JPEG;
    if (kernelMajor_ < 2)
    // FIXME: required to ensure format change is applied
      send("%s.format = %d;\n"
           "noop;\n"
           "noop;\n", camera, f);
    else
      send(SYNCLINE_WRAP("%s.format = %d|;", camera, f));
    UMessage *m = syncGet(useconds, "%s.val", camera);
    if (!m
        || m->type != MESSAGE_DATA
        || m->value->type != DATA_BINARY
        || m->value->binary->type != BINARY_IMAGE)
    {
      delete m;
      return 0;
    }
    width = m->value->binary->image.width;
    height = m->value->binary->image.height;

    size_t osize = buffersize;
    if (f == 1 && format != IMAGE_JPEG)
    {
      size_t w, h;
      //uncompress jpeg
      if (format == IMAGE_YCbCr)
	convertJPEGtoYCrCb((const byte*) m->value->binary->image.data,
			   m->value->binary->image.size, (byte**) &buffer,
			   buffersize, w, h);
      else
	convertJPEGtoRGB((const byte*) m->value->binary->image.data,
			 m->value->binary->image.size, (byte**) &buffer,
			 buffersize, w, h);
    }
    else if (format == IMAGE_RGB || format == IMAGE_PPM)
    {
      buffersize = std::min(m->value->binary->image.size,
			    static_cast<size_t> (buffersize));
      if (m->value->binary->image.imageFormat == IMAGE_YCbCr)
	convertYCbCrtoRGB((const byte*) m->value->binary->image.data,
			  buffersize, (byte*) buffer);
      else
	memcpy(buffer, m->value->binary->image.data, buffersize);

    }
    else
    {
      //jpeg jpeg, or ycrcb ycrcb
      buffersize = std::min(m->value->binary->image.size,
			    static_cast<size_t>(buffersize));
      memcpy(buffer, m->value->binary->image.data, buffersize);
    }
    if (format == IMAGE_PPM)
    {
      char p6h[20];
      sprintf(p6h, "P6\n%zu %zu\n255\n", width, height);
      size_t p6len = strlen(p6h);
      size_t mlen = osize > buffersize + p6len ? buffersize : osize - p6len;
      memmove((void *) (((long) buffer) + p6len), buffer, mlen);
      memcpy(buffer, p6h, p6len);
      buffersize += p6len;
    }
    delete m;
    return 1;
  }

  int
  USyncClient::syncGetNormalizedDevice(const char* device, ufloat& val,
				       libport::utime_t useconds)
  {
    return getValue(syncGet(useconds, "%s.valn;", device), val);
  }

  int
  USyncClient::syncGetValue(const char* valName, UValue& val,
			    libport::utime_t useconds)
  {
    return syncGetValue(0, valName, val, useconds);
  }

  int
  USyncClient::syncGetValue(const char* tag, const char* valName, UValue& val,
			    libport::utime_t useconds)
  {
    return getValue(syncGetTag(useconds, "%s;", tag, 0, valName), val);
  }

  int
  USyncClient::syncGetDevice(const char* device, ufloat& val,
			     libport::utime_t useconds)
  {
    return getValue(syncGet(useconds, "%s.val;", device), val);
  }

  int
  USyncClient::syncGetResult(const char* command, ufloat& val,
			     libport::utime_t useconds)
  {
    return getValue(syncGet(useconds, "%s", command), val);
  }


  int
  USyncClient::syncGetDevice(const char* device, const char* access,
			     ufloat& val, libport::utime_t useconds)
  {
    return getValue(syncGet(useconds, "%s.%s;", device, access), val);
  }


  int
  USyncClient::syncGetSound(const char* device, int duration, USound& sound,
			    libport::utime_t useconds)
  {
    if (kernelMajor_ < 2)
      send("syncgetsound = BIN 0;\n"
           "loopsound: loop syncgetsound = syncgetsound + %s.val,\n"
           "{\n"
           "  sleep(%d);\n"
           "  stop loopsound;\n"
           "  noop;\n"
           "  noop;\n"
           "};\n",
           device, duration);
    else
      send(SYNCLINE_WRAP(
             "syncgetsound = BIN 0;\n"
             "loopsound: loop syncgetsound = syncgetsound + %s.val,\n"
             "{\n"
             "  sleep(%d);\n"
             "  loopsound.stop;\n"
             "};", device, duration));

    UMessage* m = syncGet(useconds, "%s", "syncgetsound;");
    if (!m
        || m->type != MESSAGE_DATA
        || m->value->type != DATA_BINARY
	|| m->value->binary->type != BINARY_SOUND)
    {
      delete m;
      return 0;
    }
    convert(m->value->binary->sound, sound);
    delete m;
    return 1;
  }

  int
  USyncClient::syncSend(const void* buffer, size_t length)
  {
    if (rc != 0)
      return -1;
    sendBufferLock.lock();
    int res = effective_send(buffer, length);
    sendBufferLock.unlock();
    return res;
  }

  void
  USyncClient::waitForKernelVersion(bool hasProcessingThread)
  {
    // Do not call kernelMajor() which precisely requires kernelMajor_
    // to be defined.
    while (kernelMajor_ < 0 && !error())
    {
      // Process events if we are the processing thread or if there is none.
      if (!hasProcessingThread || cbThread == pthread_self())
        processEvents();
      // Process the sockets if we are the asio worker thread.
      libport::Socket::sleep(100000);
    }
  }

  void
  USyncClient::onConnect()
  {
    UClient::onConnect();
    if (connectCallback_)
      connectCallback_(this);
    connectCallback_ = 0;
  }

  void
  USyncClient::setDefaultOptions(const USyncClient::send_options& opt)
  {
    default_options_ = opt;
  }

  const USyncClient::send_options&
  USyncClient::getOptions(const USyncClient::send_options& opt) const
  {
    return (&opt == &USyncClient::send_options::default_options) ?
      default_options_ : opt;
  }

  static void destroySocket(libport::Socket* s)
  {
    s->destroy();
  }

  libport::Socket*
  USyncClient::onAccept(connect_callback_type connectCallback,
                        size_t buflen,
                        bool startCallbackThread)
  {
    return new USyncClient("", 0, buflen,
                           USyncClient::options()
                           .startCallbackThread(startCallbackThread)
                           .connectCallback(connectCallback));
  }

  boost::shared_ptr<libport::Finally>
  USyncClient::listen(const std::string& host, const std::string& port,
                      boost::system::error_code& erc,
                      connect_callback_type connectCallback,
                      size_t buflen,
                      bool startCallbackThread)
  {
    libport::Socket* s = new libport::Socket;
    erc = s->listen(boost::bind(&onAccept, connectCallback, buflen,
                                startCallbackThread),
                    host, port);
    if (erc)
      return boost::shared_ptr<libport::Finally>((libport::Finally*)0);
    boost::shared_ptr<libport::Finally> res(new libport::Finally);
    (*res) << boost::bind(&destroySocket, s);
    return res;
  }

  bool
  USyncClient::isCallbackThread() const
  {
    return pthread_self() == cbThread;
  }

  void
  USyncClient::setSynchronous(bool enable)
  {
    synchronous_ = enable;
  }

  void
  USyncClient::lockQueue()
  {
    queueLock_.lock();
  }
} // namespace urbi
