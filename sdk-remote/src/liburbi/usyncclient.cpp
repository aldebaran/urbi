#include <pthread.h>
#include <semaphore.h>
#include "usyncclient.h"
using std::min;
USyncClient::USyncClient(const char *_host, int _port, int _buflen) : 
  UClient(_host, _port, _buflen) {
}



struct USyncStruct
{
  UMessage *msg;
  sem_t semlock;

  //UrbiSoundCopy
  USound sound; //for format referece
  void * buffer;
  int pos;
  int size;
};


static UCallbackAction URBIUnlock(void *cbData, const UMessage &msg)
{
  USyncStruct *s = (USyncStruct *) cbData;
  s->msg = new UMessage(msg);
  sem_post(&s->semlock);
  return URBI_REMOVE;
}


int 
USyncClient::syncGetImage(const char *camera, 
                              void *buffer, int &buffersize, 
                              int format, int transmitFormat, 
                              int &width, int &height)
{
  USyncStruct s;
  sem_init(&s.semlock, 0, 0);
  int f;
  if ( (format == IMAGE_JPEG)  || (transmitFormat == URBI_TRANSMIT_JPEG))
    f = 1;
  else
    f = 0;
  send("%s.format = %d;noop;noop;", camera, f);	//XXX required to ensure format change is applied
  sendCommand(URBIUnlock, &s, "%s.val;", camera);
  sem_wait(&s.semlock);
  sem_destroy(&s.semlock);
  if (s.msg->binaryType != BINARYMESSAGE_IMAGE) {
    delete s.msg;
    return 0;
  }
  width = s.msg->image.width;
  height = s.msg->image.height;

  int osize = buffersize;
  if (f == 1  &&  format != IMAGE_JPEG ) {	//uncompress jpeg
    if (format == IMAGE_YCbCr)
      convertJPEGtoYCrCb((const byte *) s.msg->image.data, s.msg->image.size, 
                         (byte *) buffer, buffersize);
    else
      convertJPEGtoRGB((const byte *)  s.msg->image.data, s.msg->image.size,
                       (byte *) buffer, buffersize);
  }
  else if (format == IMAGE_RGB || format == IMAGE_PPM) {
    buffersize = min(s.msg->image.size, buffersize);
    if (s.msg->image.imageFormat == IMAGE_YCbCr)
      convertYCrCbtoRGB((const byte *) s.msg->image.data, buffersize, (byte *) buffer);
    else 
      memcpy(buffer, s.msg->image.data, buffersize);
    
  }
  else { //jpeg jpeg, or ycrcb ycrcb
    buffersize = min(s.msg->image.size, buffersize);
    memcpy(buffer, s.msg->image.data, buffersize);
  }
  if (format == IMAGE_PPM) {
    char p6h[20];
    sprintf(p6h, "P6\n%d %d\n255\n", width, height);
    int p6len = strlen(p6h);
    int mlen = osize > buffersize + p6len ? buffersize : osize - p6len;
    memmove((void *) (((int) buffer) + p6len), buffer, mlen);
    memcpy(buffer, p6h, p6len);
    buffersize += p6len;
  }
  delete s.msg;
  return 1;
}


int 
USyncClient::syncGetNormalizedDevice(const char *device, double &val)
{
  USyncStruct s;
  sem_init(&s.semlock, 0, 0);
  sendCommand(URBIUnlock, &s, "%s.valn;", device);
  sem_wait(&s.semlock);
  sem_destroy(&s.semlock);
  if (s.msg->type != MESSAGE_DOUBLE) {
    delete s.msg;
    return 0;
  }
  val = s.msg->doubleValue;
  return 1;
}

int 
USyncClient::syncGetDevice(const char *device, double &val)
{
  USyncStruct s;
  sem_init(&s.semlock, 0, 0);
  sendCommand(URBIUnlock, &s, "%s.val;", device);
  sem_wait(&s.semlock);
  sem_destroy(&s.semlock);
  if (s.msg->type != MESSAGE_DOUBLE) {
    delete s.msg;
    return 0;
  }
  val = s.msg->doubleValue;
  return 1;
}

int 
USyncClient::syncGetResult(const char* command, double &val) {
  USyncStruct s;
  sem_init(&s.semlock, 0, 0);
  sendCommand(URBIUnlock, &s, command);
  sem_wait(&s.semlock);
  sem_destroy(&s.semlock);
  if (s.msg->type != MESSAGE_DOUBLE) {
    delete s.msg;
    return 0;
  }
  val = s.msg->doubleValue;
  return 1;
}


int 
USyncClient::syncGetDevice(const char *device, const char * access, 
                           double &val)
{
  USyncStruct s;
  sem_init(&s.semlock, 0, 0);
  sendCommand(URBIUnlock, &s, "%s.%s;", device,access);
  sem_wait(&s.semlock);
  sem_destroy(&s.semlock);
  if (s.msg->type != MESSAGE_DOUBLE) {
    delete s.msg;
    return 0;
  }
  val = s.msg->doubleValue;
  return 1;
}

static UCallbackAction URBISoundCopy(void *cbData, const UMessage &msg) {
   USyncStruct *s = (USyncStruct *) cbData;
   if (msg.type != MESSAGE_BINARY || msg.binaryType != BINARYMESSAGE_SOUND) {
     sem_post(&s->semlock);
     return URBI_REMOVE;
   }
   s->sound = msg.sound;
   if (s->size < s->pos+msg.sound.size) s->buffer = realloc(s->buffer, msg.sound.size+s->pos);
   s->size = msg.sound.size+s->pos;
   memcpy((char*)(s->buffer)+s->pos, msg.sound.data, msg.sound.size);
   s->pos +=msg.sound.size;
   return URBI_CONTINUE;
}


int 
USyncClient::syncGetSound(const char * device, int duration, USound &sound)
{
  USyncStruct s;
  sem_init(&s.semlock, 0, 0);
  s.buffer=sound.data;
  s.pos=0;
  s.size=sound.size;
  setCallback(&URBISoundCopy, &s, "susound");
  send("loopsound: loop susound: %s.val ,   { wait(%d); stop loopsound; noop;noop; susound:ping },", device, duration);
  sem_wait(&s.semlock);
  sem_destroy(&s.semlock);
  s.sound.data = sound.data;
  s.sound.size = sound.size;
  if (! (s.sound == sound)) {
    //conversion required
    sound.data = 0;
    sound.size = 0;
    s.sound.data = (char *)s.buffer;
    s.sound.size = s.pos;
    convert(s.sound, sound);
  }
  else {
    sound.data = (char *)s.buffer;
    sound.size = s.pos;
  }
  return 0;
}

int 
USyncClient::syncSend(const void * buffer, int length) {
  if (rc !=0) return -1;
  lockSend();
  int sent = 0;
  while (sent<length) {
    int res = ::write(sd, (char *) buffer + sent, length - sent);
    if (res < 0) {
      rc = res;
      unlockSend();
      return res;
    }
    sent +=res;
  }
  unlockSend();
  return 0;
}
