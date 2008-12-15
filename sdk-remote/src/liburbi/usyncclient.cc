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
			   int port,
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
    , cbThread (0)
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

  void USyncClient::notifyCallbacks(const UMessage &msg)
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

  UMessage* USyncClient::waitForTag(const char* tag)
  {
    syncTag = tag;
    queueLock_.unlock();
    syncLock_--;
    //syncTag is reset by the other thread
    return msg;
  }

  UMessage* USyncClient::syncGet_(const char* format,
				  const char* mtag, const char* mmod,
				  va_list& arg)
  {
    //check there is no tag
    int p = 0;
    while (format[p] ==' ')
      ++p;
    while (isalpha(format[p]))
      ++p;
    while (format[p] == ' ')
      ++p;
    if (format[p]== ':' || format[p] == '<')
      return 0;
    //check if there is a command separator
    p = strlen(format) - 1;
    while (format[p] == ' ')
      --p;
    bool hasSep = (format[p] == ';' || format[p] == ',');
    sendBufferLock.lock();
    rc = vpack(format, arg);
    if (rc < 0)
    {
      sendBufferLock.unlock();
      return 0;
    }
    if (!hasSep)
      strcat(sendBuffer, ",");
    char tag[100];
    if (mtag && *mtag)
    {
      strcpy (tag, mtag);
      if (mmod != 0)
        strcat (tag, mmod);
    }
    else
      makeUniqueTag(tag);
    std::string cmd;
    if (kernelMajor_ > 1)
      cmd = std::string() +
      "if (!hasSlot(\"" + tag + "\")) { "
      "var lobby." +tag + " = Channel.new(\"" + tag + "\"); "
      "var lobby.__created_chan__};"
      + tag + " << ";
    else
      cmd = std::string() + tag + " << ";
    effectiveSend(cmd.c_str(), cmd.length());
    queueLock_.lock();
    rc = effectiveSend(sendBuffer, strlen(sendBuffer));
    sendBuffer[0] = 0;
    sendBufferLock.unlock();
    if (kernelMajor_ > 1)
    {
      cmd = std::string() +
        "if (hasSlot(\"__created_chan__\")) { "
        "lobby.removeSlot(\"__created_chan__\"); "
        "lobby.removeSlot(\"" + tag + "\") }; ";
      effectiveSend(cmd.c_str(), cmd.length());
    }
    if (mtag != 0)
      strcpy (tag, mtag);

    UMessage* m = waitForTag(tag);
    return m;
  }

  UMessage* USyncClient::syncGet(const char* format, ...)
  {
    UMessage* ret = 0;

    va_list arg;
    va_start(arg, format);
    ret =  syncGet_ (format, 0, 0, arg);
    va_end (arg);

    return ret;
  }

  UMessage* USyncClient::syncGetTag(const char* format, const char* mtag, const char* mmod, ...)
  {
    UMessage* ret = 0;

    va_list arg;
    va_start(arg, mmod);
    ret =  syncGet_ (format, mtag, mmod, arg);
    va_end (arg);

    return ret;
  }

  int
  USyncClient::syncGetImage(const char* camera,
			    void* buffer, size_t& buffersize,
			    int format, int transmitFormat,
			    size_t& width, size_t& height)
  {
    int f;
    if (format == IMAGE_JPEG  || transmitFormat == URBI_TRANSMIT_JPEG)
      f = 1;
    else
      f = 0;
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
    if (f == 1  &&  format != IMAGE_JPEG)
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
    val = (double)(*(m->value));
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
    val = (*(m->value));
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
    val = (double)(*(m->value));
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
    val = (double)(*(m->value));
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
    val = (double)(*(m->value));
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
  USyncClient::syncSend(const void* buffer, int length)
  {
    if (rc != 0)
      return -1;
    sendBufferLock.lock();
    int sent = 0;
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
