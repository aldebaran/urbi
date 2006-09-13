
#include "usyncclient.h"
#include "lockable.h"
#include <fcntl.h>

#ifndef _MSC_VER
#include <unistd.h>
#else
#include <io.h>
#endif

using std::min;


static void *callbackThreadStarter(void *objectPtr)
{
  USyncClient *connection = (USyncClient *) objectPtr;
  connection->callbackThread();
}


USyncClient::USyncClient(const char *_host, int _port, int _buflen) : 
  UClient(_host, _port, _buflen) {
	  sem = new Semaphore();
	  queueLock = new Lockable();
	  syncLock = new Semaphore();
	  msg=0;
	  syncTag = "";
	  startThread(this, &USyncClient::callbackThread);
	  if (!urbi::defaultClient)
		  urbi::defaultClient =  this;
}

void USyncClient::callbackThread() {
	while (true) {
		(*sem)--;
		queueLock->lock();
		if (queue.empty()) {
			//we got mysteriously interrupted
			queueLock->unlock();
			continue;
		}
		UMessage *m = queue.front();
		queue.pop_front();
		queueLock->unlock();
		UAbstractClient::notifyCallbacks(*m);
		delete m;
		
	}
}

void USyncClient::notifyCallbacks(const UMessage &msg) {
	queueLock->lock();
	if (syncTag == msg.tag) {
		this->msg = new UMessage(msg);
		(*syncLock)++;
	}
	else {
		queue.push_back(new UMessage(msg));
		(*sem)++;
	}
	queueLock->unlock();
}

UMessage * USyncClient::waitForTag(const char * tag) {
	syncTag = tag;
	(*syncLock)--;
	syncTag = "";
	return msg;
}


UMessage * USyncClient::syncGet(const char * format, ...) {
	//check there is no tag
	int p=0;
	while(format[p]==' ') p++;
	while (isalpha(format[p])) p++;
	while(format[p]==' ') p++;
	if (format[p]==':') {
		std::cerr <<"FATAL: passing a taged command to syncGet:'"<<format<<"'\n";
		::exit(1);
	}
	//check if there is a command separator
	p = strlen(format)-1;
	while (format[p]==' ') p--;
	bool hasSep = (format[p]==';' || format[p]==',');
	va_list arg;
	va_start(arg, format);
	sendBufferLock.lock();
	rc = vpack(format, arg);
	va_end(arg);
	if (rc < 0) {
		sendBufferLock.unlock();
		return 0;
	}
	if (!hasSep)
		strcat(sendBuffer,",");
	char tag[70];
	makeUniqueTag(tag);
	strcat(tag,":");
	effectiveSend(tag, strlen(tag));
	tag[strlen(tag)-1]=0; //restore tag
	rc = effectiveSend(sendBuffer, strlen(sendBuffer));
	sendBuffer[0] = 0;
	sendBufferLock.unlock();
	UMessage *m = waitForTag(tag);
	return m;
}



int 
USyncClient::syncGetImage(const char *camera, 
                              void *buffer, int &buffersize, 
                              int format, int transmitFormat, 
                              int &width, int &height)
{
  int f;
  if ( (format == IMAGE_JPEG)  || (transmitFormat == URBI_TRANSMIT_JPEG))
    f = 1;
  else
    f = 0;
  send("%s.format = %d;noop;noop;", camera, f);	//XXX required to ensure format change is applied
  UMessage *m = syncGet("%s.val;",camera);
  if (m->value->binary->type != BINARY_IMAGE) {
    delete m;
    return 0;
  }
  width = m->value->binary->image.width;
  height = m->value->binary->image.height;

  int osize = buffersize;
  if (f == 1  &&  format != IMAGE_JPEG ) {	//uncompress jpeg
    if (format == IMAGE_YCbCr)
      convertJPEGtoYCrCb((const byte *) m->value->binary->image.data, m->value->binary->image.size, 
                         (byte *) buffer, buffersize);
    else
      convertJPEGtoRGB((const byte *)  m->value->binary->image.data, m->value->binary->image.size,
                       (byte *) buffer, buffersize);
  }
  else if (format == IMAGE_RGB || format == IMAGE_PPM) {
    buffersize = min(m->value->binary->image.size, buffersize);
    if (m->value->binary->image.imageFormat == IMAGE_YCbCr)
      convertYCrCbtoRGB((const byte *) m->value->binary->image.data, buffersize, (byte *) buffer);
    else 
      memcpy(buffer, m->value->binary->image.data, buffersize);
    
  }
  else { //jpeg jpeg, or ycrcb ycrcb
    buffersize = min(m->value->binary->image.size, buffersize);
    memcpy(buffer, m->value->binary->image.data, buffersize);
  }
  if (format == IMAGE_PPM) {
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
USyncClient::syncGetNormalizedDevice(const char *device, double &val)
{
 
	UMessage *m = syncGet("%s.valn;", device);
	
  if (m->type != MESSAGE_DATA || m->value->type != DATA_DOUBLE) {
    delete m;
    return 0;
  }
  val = (double)(*(m->value));
  delete m;
  return 1;
}

int 
USyncClient::syncGetDevice(const char *device, double &val)
{
		UMessage *m = syncGet("%s.val;", device);
	
  if (m->type != MESSAGE_DATA || m->value->type != DATA_DOUBLE) {
    delete m;
    return 0;
  }
  val = (double)(*(m->value));
  delete m;
  return 1;
}

int 
USyncClient::syncGetResult(const char* command, double &val) {
  	UMessage *m = syncGet("%s", command);
	
  if (m->type != MESSAGE_DATA || m->value->type != DATA_DOUBLE) {
    delete m;
    return 0;
  }
  val = (double)(*(m->value));
  delete m;
  return 1;
	
}


int 
USyncClient::syncGetDevice(const char *device, const char * access, 
                           double &val)
{
	UMessage *m = syncGet("%s.%s;", device,access);
	
  if (m->type != MESSAGE_DATA || m->value->type != DATA_DOUBLE) {
    delete m;
    return 0;
  }
  val = (double)(*(m->value));
  delete m;
  return 1;
  
  
}


int 
USyncClient::syncGetSound(const char * device, int duration, USound &sound)
{

  send("syncgetsound = BIN 0;loopsound: loop syncgetsound = syncgetsound +  %s.val ,   { wait(%d); stop loopsound; noop;noop; };", device, duration);
  UMessage * m = syncGet("syncgetsound;");
  if (m->type != MESSAGE_DATA || m->value->type != DATA_BINARY || m->value->binary->type != BINARY_SOUND) {
	  delete m;
	  return 0;
  }
  convert(m->value->binary->sound, sound);
  delete m;
  return 1;
  
}

int 
USyncClient::syncSend(const void * buffer, int length) {
  if (rc !=0) return -1;
  sendBufferLock.lock();
  int sent = 0;
  while (sent<length) {
    int res = ::write(sd, (char *) buffer + sent, length - sent);
    if (res < 0) {
      rc = res;
      sendBufferLock.unlock();
      return res;
    }
    sent +=res;
  }
  sendBufferLock.unlock();
  return 0;
}
