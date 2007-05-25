#include "libport/network.h"

#include "kernel/userver.hh"

#include "network/bsdnet/connection.hh"

// Mac OSX does not have MSG_NOSIGNAL, used by send and recv to ask
// for events to become errno rather than signals.  But it supports
// the socket option SO_NOSIGPIPE.
#ifndef MSG_NOSIGNAL
# define MSG_NOSIGNAL 0
#endif


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
  if (uerror_ != USUCCESS)
    closeConnection();
  else
    initialize();
}

//! Connection destructor.
Connection::~Connection()
{
  if (fd != -1)
    closeConnection();
}

std::ostream&
Connection::print (std::ostream& o) const
{
  return o
    << "Connection "
    << "{ controlFd = " << controlFd
    << ", fd = " << fd
    << " }";

}

//! Close the connection
/*!
 */
UErrorValue
Connection::closeConnection()
{
  // Setting 'closing' to true tell the kernel not to use the
  // connection any longer.
  closing = true;

  // FIXME: Akim added those two lines, but he's not too sure
  // about them: should they be before "closing = true"?
  if (fd == -1)
    // We are already closed.
    return USUCCESS;
#if defined(WIN32) && !defined(__MINGW32__)
  closesocket(fd);
  int ret = 0;//WSACleanup(); //wsastartup called only once!
#else
  int ret = close(fd);
  if (ret)
    perror ("cannot close connection fd");
#endif
  Network::unregisterNetworkPipe(this);

  if (ret)
    return UFAIL;
  else
  {
    fd = -1;
    return USUCCESS;
  }
}

void Connection::doRead()
{
  int n = ::recv(fd, (char*)read_buff, PACKETSIZE, MSG_NOSIGNAL);
  if (n == -1)
  {
    perror ("cannot recv");
    closeConnection();
  }
  else
    received(read_buff, n);
}

int Connection::effectiveSend (const ubyte* buffer, int length)
{
  int res = ::send(fd,
		   reinterpret_cast<const char *>(buffer), length,
		   MSG_NOSIGNAL);
  if (res == -1)
  {
    perror ("cannot send");
    closeConnection();
  }

  return res;
}

void Connection::doWrite()
{
  continueSend();
}

UErrorValue Connection::send(const ubyte* buffer, int length)
{
  if (sendQueueRemain() == 0)
    trigger();
  return UConnection::send(buffer, length);
}
