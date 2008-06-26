#ifndef _MSC_VER
# include <unistd.h>
#else
# include <io.h>
#endif
#include <fcntl.h>

#include "libport/thread.hh"
#include "libport/assert.hh"

#include "urbi/usyncclient.hh"
#include "urbi/uconversion.hh"

/* "min" shouldn't be defined as a preprocessor macro. On windows, it is. So
 * using std::min leads to a parse error because CPP expands it as
 *   std::((a) < (b) ? (a) : (b))
 * However, this should only occur on windows with the STL of MS VC++.
 */
#ifdef min
# ifdef WIN32
#  undef min
# else
#  error "min is defined as a CPP macro and we're not on WIN32. Report this!"
# endif /* !WIN32 */
#endif /* !min */

namespace urbi
{
  USyncClient::USyncClient(const char *_host, int _port, int _buflen)
    : UClient(_host, _port, _buflen),
      sem_(),
      queueLock_(),
      msg(0),
      syncLock_(),
      syncTag (""),
      stopCallbackThread_(false)
  {
    if (error())
      return;
    libport::startThread(this, &USyncClient::callbackThread);
    if (!defaultClient)
      defaultClient = this;
  }

  void USyncClient::callbackThread()
  {
    while (true)
    {
      sem_--;
      if (stopCallbackThread_)
	return;
      queueLock_.lock();
      if (queue.empty())
      {
	//we got mysteriously interrupted
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
    stopCallbackThread_ = true;
  }
  void USyncClient::processEvents(const libport::utime_t timeout)
  {
    libport::utime_t startTime = libport::utime();
    while (timeout < 0 || libport::utime() - startTime <= timeout)
    {
      queueLock_.lock();
      if (queue.empty())
      {
	queueLock_.unlock();
	return;
      }
      UMessage *m = queue.front();
      queue.pop_front();
      queueLock_.unlock();
      UAbstractClient::notifyCallbacks(*m);
      delete m;
    }

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
    passert(format[p], format[p]!= ':' && format[p] != '<');
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
    const char * separator = "<<";
    char tag[100];
    if (mtag != 0)
    {
      strcpy (tag, mtag);
      if (mmod != 0)
	strcat (tag, mmod);
    }
    else
      makeUniqueTag(tag);
    effectiveSend(tag, strlen(tag));
    effectiveSend(separator, strlen(separator));
    queueLock_.lock();
    rc = effectiveSend(sendBuffer, strlen(sendBuffer));
    sendBuffer[0] = 0;
    sendBufferLock.unlock();

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
			    void* buffer, int& buffersize,
			    int format, int transmitFormat,
			    int& width, int& height)
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

    int osize = buffersize;
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
      sprintf(p6h, "P6\n%d %d\n255\n", width, height);
      int p6len = strlen(p6h);
      int mlen = osize > buffersize + p6len ? buffersize : osize - p6len;
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
    UMessage *m = syncGet("%s;", valName);

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
