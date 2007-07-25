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
UConnection&
Connection::closeConnection()
{
  // Setting 'closing' to true tell the kernel not to use the
  // connection any longer.
  closing = true;

  // FIXME: Akim added those two lines, but he's not too sure
  // about them: should they be before "closing = true"?
  if (fd == -1)
    // We are already closed.
    CONN_ERR_RET(USUCCESS);
#if defined(WIN32) && !defined(__MINGW32__)
  closesocket(fd);
  int ret = 0;//WSACleanup(); //wsastartup called only once!
#else
  int ret = ::close(fd);
  if (ret)
    perror ("cannot close connection fd");
#endif
  Network::unregisterNetworkPipe(this);

  if (ret)
    CONN_ERR_RET(UFAIL);
  else
  {
    fd = -1;
    CONN_ERR_RET(USUCCESS);
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

UConnection& Connection::send(const ubyte* buffer, int length)
{
  if (sendQueueRemain() == 0)
    trigger();
  return (*this) << UConnection::send(buffer, length);
}

//! Send a "\n" through the connection
UConnection& Connection::endline ()
{
  //FIXME: test send error
  (*this) <<UConnection:: send((const ubyte*)"\n", 1);
  CONN_ERR_RET(USUCCESS);
}
