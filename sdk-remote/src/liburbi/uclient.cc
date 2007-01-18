/*! \file uclient.cpp
****************************************************************************
 * $Id: uclient.cpp,v 1.9 2005/09/30 17:48:00 nottale Exp $
 *
 * Linux implementation of the URBI interface class
 *
 * Copyright (C) 2004, 2006, 2007 Jean-Christophe Baillie.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**********************************************************************/

#include <cstdlib>
#include "libport/cstdio"
#include <cerrno>

#include <locale.h>
#include "urbi/uclient.hh"
#include "libport/lockable.hh"
#include "libport/thread.hh"

// In the long run, this should be part of libport, but I don't know
// where actually :( Should it be libport/sys/types.hh?  Or unistd.hh?
// What standard header should do that?

#ifdef WIN32
// On windows, file descriptors are defined as u_int (i.e., unsigned int).
# define LIBPORT_FD_SET(N, P) FD_SET(static_cast<u_int>(N), P)
#else
# define LIBPORT_FD_SET(N, P) FD_SET(N, P)
#endif

#ifdef WIN32
# include "libport/windows.hh"
# include <fcntl.h>
# include <io.h>
# include <winsock.h>
#else
# include <sys/time.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <unistd.h>
#endif

namespace urbi
{
  /*! Establish the connection with the server.
   Spawn a new thread that will listen to the socket, parse the incoming URBI
   messages, and notify the appropriate callbacks.
   */
  UClient::UClient(const char *_host, int _port, int _buflen)
    : UAbstractClient(_host, _port, _buflen)
  {
    setlocale(LC_NUMERIC, "C");
    control_fd[0] = control_fd[1] = -1;

#ifndef WIN32
    if (::pipe(control_fd) == -1)
    {
      rc = -1;
      perror("UClient::UClient failed to create pipe");
      return;
    }
#endif

    // Address resolution stage.
    struct hostent *hen;		// host-to-IP translation
    struct sockaddr_in sa;	// Internet address struct

    memset(&sa, 0, sizeof (sa));
#ifdef WIN32
    WSADATA wsaData;
    WORD wVersionRequested;
    wVersionRequested = MAKEWORD( 1, 1 );
    WSAStartup( wVersionRequested, &wsaData );
#endif
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    hen = gethostbyname(host);

    if (!hen)
    {
      // maybe it is an IP address
      sa.sin_addr.s_addr = inet_addr(host);
      if (sa.sin_addr.s_addr == INADDR_NONE)
      {
	printf("UClient::UClient cannot resolve host name.\n");
	rc = -1;
	return;
      }
    }
    else
      memcpy(&sa.sin_addr.s_addr, hen->h_addr_list[0], hen->h_length);

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
      printf("UClient::UClient socket allocation failed.\n");
      rc = -1;
      return;
    }

    // now connect to the remote server.
    rc = connect(sd, (struct sockaddr *) &sa, sizeof (sa));

    // If we attempt to connect too fast to aperios ipstack it will fail.
    if (rc)
    {
      usleep(20000);
      rc = connect(sd, (struct sockaddr *) &sa, sizeof (sa));
    }

    // Check there was no error.
    if (rc)
    {
      std::cerr << "UClient::UClient cannot connect." << std::endl;
      return;
    }

    //check that it really worked
    int pos=0;
    while (pos==0)
      pos = ::recv(sd, recvBuffer, buflen, 0);
    if (pos<0)
    {
      rc = pos;
      printf("UClient::UClient cannot connect: read error %d.\n", rc);
      return;
    }
    else
      recvBufferPosition = pos;
    recvBuffer[recvBufferPosition] = 0;
    thread = libport::startThread(this, &UClient::listenThread);
    if (!defaultClient)
      defaultClient = this;
  }

  UClient::~UClient()
  {
    close(sd);
    sd = -1;
    if (control_fd[1] != -1 )
      ::write(control_fd[1], "a", 1);
    //must wait for listen thread to terminate
    libport::joinThread(thread);
    if (control_fd[1] != -1)
      close(control_fd[1]);
    if (control_fd[0] != -1)
      close(control_fd[0]);
  }


  bool
  UClient::canSend(int)
  {
    return true;
  }


  int
  UClient::effectiveSend(const void  * buffer, int size)
  {
#if DEBUG
    char output[size+1];
    memcpy (static_cast<void*> (output), buffer, size);
    output[size]=0;
    cout << ">>>> SENT : [" << output << "]" << endl;
#endif
    if (rc)
      return -1;
    int pos = 0;
    while (pos!=size)
    {
      int retval = ::send(sd, (char *) buffer + pos, size-pos, 0);
      if (retval<0)
      {
	rc = retval;
	return rc;
      }
      pos += retval;
    }
    return 0;
  }

  void
  UClient::listenThread()
  {
    fd_set rfds;
    int maxfd=1+ (sd>control_fd[0]? sd:control_fd[0]);
    int res;
    while (true)
    {
      do {
	if (sd==-1)
	  return;
	FD_ZERO(&rfds);
	LIBPORT_FD_SET(sd, &rfds);
#ifndef WIN32
	LIBPORT_FD_SET(control_fd[0], &rfds);
#endif
	struct timeval tme;
	tme.tv_sec = 1;
	tme.tv_usec = 0;
	res = select(maxfd+1, &rfds, NULL, NULL, &tme);
	if (res < 0 && errno != EINTR)
	{
	  this->rc = -1;
#ifdef WIN32
	  res = WSAGetLastError();
#endif
	  std::cerr << "select error "<<res<<std::endl;
	  //TODO when error will be implemented, send an error msg
	  //TODO maybe try to reconnect?
	  return;
	}
	if (res == -1)
	{
	  res=0;
	  continue;
	}
#ifndef WIN32
	if (res != 0 && FD_ISSET(control_fd[0], &rfds))
	  return;
#endif
      } while (res == 0);
      int count = ::recv(sd,
			 &recvBuffer[recvBufferPosition],
			 buflen - recvBufferPosition - 1, 0);
      if (count < 0)
      {
	rc = -1;
	std::cerr << "error " << count << std::endl;
	//TODO when error will be implemented, send an error msg
	//TODO maybe try to reconnect?
	return;
      }

      recvBufferPosition += count;
      recvBuffer[recvBufferPosition] = 0;
      processRecvBuffer();
    }
  }


  void
  UClient::printf(const char * format, ...)
  {
    va_list arg;
    va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
  }

  unsigned int UClient::getCurrentTime() const
  {
  // FIXME: Put this into libport.
#ifdef WIN32
    return GetTickCount();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000+tv.tv_usec/1000;
#endif
  }

  void execute(void)
  {
#ifdef WIN32
    while (true)
      Sleep(100000);
#else
    while (true)
      sleep(100);
#endif
  }

  void exit(int code)
  {
    ::exit(code);
  }


  UClient & connect(const char* host)
  {
    return *new UClient(host);
  }

} // namespace urbi
