#include "libport/network.h"

#include "network/bsdnet/connection.hh"
#include "userver.hh"

//! LinuxConnection constructor.
/*! The constructor calls UConnection::UConnection with the appropriate
 parameters.
 The global variable ::linuxserver saves the need to pass a UServer parameter
 to the LinuxConnection constructor.
 */
Connection::Connection(int connfd)
  : UConnection	(::urbiserver,
		 Connection::MINSENDBUFFERSIZE,
		 Connection::MAXSENDBUFFERSIZE,
		 Connection::PACKETSIZE,
		 Connection::MINRECVBUFFERSIZE,
		 Connection::MAXRECVBUFFERSIZE),
    fd(connfd)
{
  // Test the error from UConnection constructor.
  if (UError != USUCCESS)
    closeConnection();
  else
    initialize();
}

//! Connection destructor.
Connection::~Connection()
{
  if (fd)
    closeConnection();
}

//! Close the connection
/*!
 */
UErrorValue
Connection::closeConnection()
{
  // Setting 'closing' to true tell the kernel not to use the
  // connection any longer
  closing = true;
#ifdef WIN32
  closesocket(fd);
  int ret = 0;//WSACleanup(); //wsastartup called only once!
#else
  int ret = close(fd);
#endif
  Network::unregisterNetworkPipe(this);

  if (ret)
    return UFAIL;
  else
  {
    fd=-1;
    return USUCCESS;
  }
}

// Try for a trick on Mac OS X
#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
#endif

void Connection::doRead()
{
  int n = ::recv(fd, (char *)read_buff, PACKETSIZE, MSG_NOSIGNAL);
  if (n<=0)
    //kill us
    closeConnection();
  else
    received(read_buff, n);

}

int Connection::effectiveSend (const ubyte *buffer, int length)
{
  int res = ::send(fd,
		   reinterpret_cast<const char *>(buffer), length,
		   MSG_NOSIGNAL);
  if (res <= 0)
  {
    //kill us
    closeConnection();
    return -1;
  }
  else
    return res; // Number of bytes actually written.
}

void Connection::doWrite()
{
  continueSend();
}

UErrorValue Connection::send(const ubyte *buffer, int length)
{
  if (sendQueueRemain()==0)
    trigger();
  return UConnection::send(buffer, length);
}
