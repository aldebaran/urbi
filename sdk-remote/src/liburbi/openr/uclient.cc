/*! \file uclient.cc
 */

#include "uclient.h"
#include "CLIENT.h"
using std::min;
extern OID		CLIENT_ID;
extern antStackRef  ipstackRef;
extern int		CLIENTtime;

UClient::UClient(const char *host, int port, int buflen)
  : UAbstractClient(host, port, buflen)
{
  debug("UCLlent::UClient\n");
  connection = new AiboConnection(ipstackRef,
								  CLIENT_ID,
								  port,
								  host,
								  this);
  debug("UCLlent::UClient created connection\n");
  connection->Connect();
  debug("UCLlent::UClient connected \n");
  if (!urbi::defaultClient)
    urbi::defaultClient =  this;
}

UClient::~UClient()
{
  connection->Close();
}

bool
UClient::canSend(int size)
{
  return true;
}

void
UClient::printf(const char *format, ...)
{
  char	*str;

  str = new char[buflen];
  va_list arg;
  va_start(arg, format);
  vsprintf(str, format, arg);
  OSYSPRINT(("%s", str));
  va_end(arg);
  delete str;
}

unsigned int
UClient::getCurrentTime() {
  struct SystemTime time;
  GetSystemTime(&time);
  return time.seconds * 1000 + time.useconds / 1000;
}

int
UClient::effectiveSend(const void *buffer, int size)
{  
  debug("UCLlent::effectiveSend\n");
  connection->send((char *)buffer, size);
}

void
UClient::lockList()
{
}

void
UClient::lockSend()
{
}

void
UClient::unlockList()
{
}

void
UClient::unlockSend()
{
}

void
urbi::execute()
{
}

void
urbi::exit(int code) 
{
  //(*(int*)0)=0;
}


UClient & 
urbi::connect(const char  * host) {
  return *new UClient(host);
}

char	*
UClient::getRecvBuffer() const
{
  return recvBuffer;
}

int
UClient::getRecvBufferPosition() const
{
  return recvBufferPosition;
}

void
UClient::setRecvBuffer(byte *buf, int length)
{
  memcpy((char *)recvBuffer + recvBufferPosition, buf, length);
}

void
UClient::setRecvBufferPosition(int pos)
{
  recvBufferPosition += pos;
  recvBuffer[recvBufferPosition] = 0;
}



void
UClient::received(const char * data, int size) {
  debug("UCLlent::received %d, buflen %d bufpos %d\n", size, buflen, recvBufferPosition);
  int pos = 0;
  while (pos<size) {
    int tocp =  min(size-pos, buflen-recvBufferPosition);
    memcpy(recvBuffer+recvBufferPosition, data+pos,tocp);
    recvBufferPosition += tocp;
    pos += tocp;
    debug("UCLlent::received: processing...\n");
    processRecvBuffer();
    debug("UCLlent::received: processing...done\n");
  }
}
