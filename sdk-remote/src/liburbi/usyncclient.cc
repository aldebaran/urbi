#include <libport/unistd.h>
#include <fcntl.h>

#include <libport/assert.hh>
#include <libport/compiler.hh>
#include <libport/thread.hh>

#include <urbi/uconversion.hh>
#include <urbi/usyncclient.hh>

namespace urbi
{
  USyncClient::USyncClient(const std::string& host,
			   unsigned port,
			   size_t buflen,
			   bool server,
                           bool startCallbackThread,
			   int semListenInc)
    : UClient(host, port, buflen, server, semListenInc)
    , sem_()
    , queueLock_()
    , msg(0)
    , syncLock_()
    , syncTag()
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

  USyncClient::~USyncClient ()
  {
    closeUClient ();

    if (cbThread)
      joinCallbackThread_ ();
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

  bool USyncClient::processEvents(const libport::utime_t timeout)
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

  int USyncClient::joinCallbackThread_ ()
  {
    stopCallbackThread ();
    if (cbThread)
    {
      libport::joinThread(cbThread);
      cbThread = 0;
    }
    return 0;
  }

  void
  USyncClient::notifyCallbacks(const UMessage& msg)
  {
    queueLock_.lock();
    if (!syncTag.empty() && syncTag == msg.tag)
    {
      this->msg = new UMessage(msg);
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
  USyncClient::waitForTag(const std::string& tag)
  {
    syncTag = tag;
    queueLock_.unlock();
    syncLock_--;
    // syncTag is reset by the other thread.
    return msg;
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
    make_tag(UAbstractClient&cl, const char*t1, const char* t2)
    {
      std::string res;
      if (t1)
      {
        res += t1;
        if (t2)
          res += t2;
      }
      else
      {
        char buf[100];
        cl.makeUniqueTag(buf);
        res += buf;
      }
      return res;
    }
  }

  UMessage*
  USyncClient::syncGet_(const char* format,
                        const char* mtag, const char* mmod,
                        va_list& arg)
  {
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
    std::string tag = make_tag(*this, mtag, mmod);
    std::string cmd;
    if (kernelMajor_ > 1)
      // Really create the channel for this tag, as the user is
      // probably using this tag in the code.
      cmd += ("if (!hasSlot(\"" + tag + "\"))\n"
              "{\n"
              "  var this." + tag + " = Channel.new(\"" + tag + "\")|\n"
              "  var this.__created_chan__ = true |\n"
              "}|\n");
    cmd += tag + " << ";
    effective_send(cmd);
    queueLock_.lock();
    rc = effective_send(sendBuffer);
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    if (kernelMajor_ > 1)
    {
      cmd = ("if (hasSlot(\"__created_chan__\"))\n"
             "{\n"
             "  removeSlot(\"__created_chan__\")|\n"
             "  removeSlot(\"" + tag + "\")|\n"
             "};\n");
      effective_send(cmd);
    }

    if (mtag)
      tag = mtag;
    return waitForTag(tag);
  }

  UMessage* USyncClient::syncGet(const char* format, ...)
  {
    va_list arg;
    va_start(arg, format);
    UMessage* res = syncGet_(format, 0, 0, arg);
    va_end(arg);
    return res;
  }

  UMessage* USyncClient::syncGetTag(const char* format,
                                    const char* mtag, const char* mmod, ...)
  {
    va_list arg;
    va_start(arg, mmod);
    UMessage* res = syncGet_(format, mtag, mmod, arg);
    va_end (arg);
    return res;
  }

  int
  USyncClient::syncGetImage(const char* camera,
			    void* buffer, size_t& buffersize,
			    int format, int transmitFormat,
			    size_t& width, size_t& height)
  {
    int f = format == IMAGE_JPEG  || transmitFormat == URBI_TRANSMIT_JPEG;
    //XXX required to ensure format change is applied
    send("%s.format = %d; noop; noop;", camera, f);
    UMessage *m = syncGet("%s.val;", camera);
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
			    static_cast<size_t> (buffersize));
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
  USyncClient::syncGetNormalizedDevice(const char* device, double& val)
  {
    UMessage *m = syncGet("%s.valn;", device);

    if (m->type != MESSAGE_DATA || m->value->type != DATA_DOUBLE)
    {
      delete m;
      return 0;
    }
    val = (double)*m->value;
    delete m;
    return 1;
  }

  int
  USyncClient::syncGetValue(const char* valName, UValue& val)
  {
    return syncGetValue(0, valName, val);
  }

  int
  USyncClient::syncGetValue(const char* tag, const char* valName, UValue& val)
  {
    UMessage *m = syncGetTag("%s;", tag, 0, valName);

    if (m->type != MESSAGE_DATA)
    {
      delete m;
      return 0;
    }
    val = *m->value;
    delete m;
    return 1;
  }

  int
  USyncClient::syncGetDevice(const char* device, double& val)
  {
    UMessage *m = syncGet("%s.val;", device);

    if (m->type != MESSAGE_DATA || m->value->type != DATA_DOUBLE)
    {
      delete m;
      return 0;
    }
    val = (double)*m->value;
    delete m;
    return 1;
  }

  int
  USyncClient::syncGetResult(const char* command, double& val)
  {
    UMessage *m = syncGet("%s", command);

    if (m->type != MESSAGE_DATA || m->value->type != DATA_DOUBLE)
    {
      delete m;
      return 0;
    }
    val = (double)*m->value;
    delete m;
    return 1;
  }


  int
  USyncClient::syncGetDevice(const char* device, const char* access,
			     double& val)
  {
    UMessage *m = syncGet("%s.%s;", device, access);

    if (m->type != MESSAGE_DATA || m->value->type != DATA_DOUBLE)
    {
      delete m;
      return 0;
    }
    val = (double)*m->value;
    delete m;
    return 1;
  }


  int
  USyncClient::syncGetSound(const char* device, int duration, USound& sound)
  {
    send("syncgetsound = BIN 0;"
	 " loopsound: loop syncgetsound = syncgetsound +  %s.val,"
	 " { "
	 "   sleep(%d);"
	 "   stop loopsound;"
	 "   noop;"
	 "   noop;"
	 " };", device, duration);
    UMessage* m = syncGet("syncgetsound;");
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
} // namespace urbi
