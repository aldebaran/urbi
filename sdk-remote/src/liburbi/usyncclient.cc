#include <libport/unistd.h>
#include <fcntl.h>

#include <libport/assert.hh>
#include <libport/compiler.hh>
#include <libport/thread.hh>
#include <libport/unistd.h>

#include <liburbi/compatibility.hh>
#include <urbi/uconversion.hh>
#include <urbi/usyncclient.hh>

namespace urbi
{

  const USyncClient::options USyncClient::options::default_options =
    USyncClient::options();

  USyncClient::options::options()
    : timeout_(0)
    , mtag_(0)
    , mmod_(0)
  {}

  USyncClient::options&
  USyncClient::options::timeout(libport::utime_t usec)
  {
    timeout_ = usec;
    return *this;
  }

  USyncClient::options&
  USyncClient::options::tag(const char* tag, const char* mod)
  {
    mtag_ = tag;
    mmod_ = mod;
    return *this;
  }

  USyncClient::USyncClient(const std::string& host,
			   unsigned port,
			   size_t buflen,
			   bool server,
                           bool startCallbackThread,
			   unsigned semListenInc)
    : UClient(host, port, buflen, server, semListenInc)
    , sem_()
    , queueLock_()
    , message_(0)
    , syncLock_()
    , syncTag()
    , default_options_()
    , stopCallbackThread_(!startCallbackThread)
    , cbThread(0)
  {
    if (error())
      return;

    if (startCallbackThread)
      cbThread = libport::startThread(this, &USyncClient::callbackThread);
    if (!defaultClient)
      defaultClient = this;

    listenSem_++;
    callbackSem_++;
  }

  USyncClient::~USyncClient()
  {
    closeUClient();

    if (cbThread)
      joinCallbackThread_();
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
    // Wait until the callback thread is actually stopped to avoid both
    // processEvents and the callbackThread running at the same time.
    stopCallbackSem_--;
  }

  bool USyncClient::processEvents(libport::utime_t timeout)
  {
    bool res = false;
    libport::utime_t startTime = libport::utime();
    while (timeout < 0 || libport::utime() - startTime <= timeout)
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
    }
    return res;
  }

  int USyncClient::joinCallbackThread_()
  {
    stopCallbackThread();
    if (cbThread)
    {
      pthread_join(cbThread, 0);
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
      syncLock_++;
    }
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
    syncTag = tag;
    queueLock_.unlock();
    // syncTag is reset by the other thread.
    UMessage *res = syncLock_.uget(useconds) ? message_ : 0;
    message_ = 0;
    return res;
  }

  namespace
  {
    /// Check that \a cp looks like "foo <" or "foo :".
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

    /// Whether \a cp ends with , or ; (skipping trailing spaces).
    static
    bool
    has_terminator(const char* cp)
    {
      int p = static_cast<int>(strlen(cp)) - 1;
      while (0 <= p && cp[p] == ' ')
        --p;
      return 0 <= p && (cp[p] == ';' || cp[p] == ',');
    }

    /// Return the concatenation of t1 and t2, make it unique
    /// if they are empty.
    static
    std::string
    make_tag(UAbstractClient& cl, const USyncClient::options& opt)
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

  UMessage*
  USyncClient::syncGet_(const char* format, va_list& arg,
			const USyncClient::options& options)
  {
    const USyncClient::options& opt_used = getOptions(options);
    if (has_tag(format))
      return 0;
    sendBufferLock.lock();
    rc = vpack(format, arg);
    if (rc < 0)
    {
      sendBufferLock.unlock();
      return 0;
    }
    if (!has_terminator(format))
      strcat(sendBuffer, ",\n");
    std::string tag = make_tag(*this, opt_used);
    effective_send(compatibility::evaluate_in_channel_open(tag));
    queueLock_.lock();
    rc = effective_send(sendBuffer);
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    effective_send(compatibility::evaluate_in_channel_close(tag));
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
    UMessage* res = syncGet_(format, arg, options().timeout(useconds));
    va_end(arg);
    return res;
  }

  UMessage*
  USyncClient::syncGetTag(const char* format,
                          const char* mtag, const char* mmod, ...)
  {
    va_list arg;
    va_start(arg, mmod);
    UMessage* res = syncGet_(format, arg, options().tag(mtag, mmod));
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
			     options().tag(mtag, mmod).timeout(useconds));
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
    int f = format == IMAGE_JPEG  || transmitFormat == URBI_TRANSMIT_JPEG;
    //XXX required to ensure format change is applied
    send("%s.format = %d;\n"
         "noop;\n"
         "noop;\n", camera, f);
    UMessage *m = syncGet(useconds, "%s.val;\n", camera);
    if (!m)
      return 0;
    if (m->value->binary->type != BINARY_IMAGE)
    {
      delete m;
      return 0;
    }
    width = m->value->binary->image.width;
    height = m->value->binary->image.height;

    size_t osize = buffersize;
    if (f == 1  && format != IMAGE_JPEG)
    {
      //uncompress jpeg
      if (format == IMAGE_YCbCr)
	convertJPEGtoYCrCb((const byte*) m->value->binary->image.data,
			   m->value->binary->image.size, (byte*) buffer,
			   buffersize);
      else
	convertJPEGtoRGB((const byte*) m->value->binary->image.data,
			 m->value->binary->image.size, (byte*) buffer,
			 buffersize);
    }
    else if (format == IMAGE_RGB || format == IMAGE_PPM)
    {
      buffersize = std::min(m->value->binary->image.size,
			    static_cast<size_t> (buffersize));
      if (m->value->binary->image.imageFormat == IMAGE_YCbCr)
	convertYCrCbtoRGB((const byte*) m->value->binary->image.data,
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
  USyncClient::syncGetNormalizedDevice(const char* device, double& val,
				       libport::utime_t useconds)
  {
    return getValue(syncGet(useconds, "%s.valn;\n", device), val);
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
    return getValue(syncGetTag(useconds, "%s;\n", tag, 0, valName), val);
  }

  int
  USyncClient::syncGetDevice(const char* device, double& val,
			     libport::utime_t useconds)
  {
    return getValue(syncGet(useconds, "%s.val;\n", device), val);
  }

  int
  USyncClient::syncGetResult(const char* command, double& val,
			     libport::utime_t useconds)
  {
    return getValue(syncGet(useconds, "%s", command), val);
  }


  int
  USyncClient::syncGetDevice(const char* device, const char* access,
			     double& val, libport::utime_t useconds)
  {
    return getValue(syncGet(useconds, "%s.%s;", device, access), val);
  }


  int
  USyncClient::syncGetSound(const char* device, int duration, USound& sound,
			    libport::utime_t useconds)
  {
    send("syncgetsound = BIN 0;\n"
	 "loopsound: loop syncgetsound = syncgetsound +  %s.val,\n"
	 " {\n"
	 "   sleep(%d);\n"
	 "   stop loopsound;\n"
	 "   noop;\n"
	 "   noop;\n"
	 " };\n", device, duration);
    UMessage* m = syncGet(useconds, "%s", "syncgetsound;");
    if (!m)
      return 0;
    if (m->type != MESSAGE_DATA
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
    size_t sent = 0;
    while (sent < length)
    {
      int res = ::write(sd, (char*) buffer + sent, length - sent);
      if (res < 0)
      {
	rc = res;
	sendBufferLock.unlock();
	return res;
      }
      sent += res;
    }
    sendBufferLock.unlock();
    return 0;
  }

  void
  USyncClient::waitForKernelVersion(bool hasProcessingThread)
  {
    // Do not call kernelMajor() which precisely requires kernelMajor_
    // to be defined.
    while (kernelMajor_ < 0 && !error())
    {
      if (!hasProcessingThread)
        processEvents();
      usleep(100000);
    }
  }

  void
  USyncClient::setDefaultOptions(const USyncClient::options& opt)
  {
    default_options_ = opt;
  }

  const USyncClient::options&
  USyncClient::getOptions(const USyncClient::options& opt) const
  {
    return (&opt == &USyncClient::options::default_options) ?
      default_options_ : opt;
  }

} // namespace urbi
