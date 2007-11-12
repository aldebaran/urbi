/*! \file uclient.cc
****************************************************************************
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

#include "libport/unistd.h"

#ifndef WIN32
# include <sys/time.h>
# include <time.h>
# include <signal.h>
#endif

#include "urbi/uclient.hh"
#include "libport/network.h"
#include "libport/lockable.hh"
#include "libport/thread.hh"

namespace urbi
{
  /*! Establish the connection with the server.
   Spawn a new thread that will listen to the socket, parse the incoming URBI
   messages, and notify the appropriate callbacks.
   */
  UClient::UClient(const char *_host, int _port, int _buflen)
    : UAbstractClient(_host, _port, _buflen),
      thread(0)
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
    //block sigpipe
    signal(SIGPIPE, SIG_IGN);
#endif

    // Address resolution stage.
    struct sockaddr_in sa;	// Internet address struct
    memset(&sa, 0, sizeof sa);
#ifdef WIN32
    WSADATA wsaData;
    WORD wVersionRequested;
    wVersionRequested = MAKEWORD( 1, 1 );
    WSAStartup( wVersionRequested, &wsaData );
#endif
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    // host-to-IP translation
    struct hostent* hen = gethostbyname(host);
    if (!hen)
    {
      // maybe it is an IP address
      sa.sin_addr.s_addr = inet_addr(host);
      if (sa.sin_addr.s_addr == INADDR_NONE)
      {
	std::cerr << "UClient::UClient cannot resolve host name." << std::endl;
	rc = -1;
	return;
      }
    }
    else
      memcpy(&sa.sin_addr.s_addr, hen->h_addr_list[0], hen->h_length);

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
      perror("UClient::UClient socket");
      rc = -1;
      return;
    }

    // now connect to the remote server.
    rc = connect(sd, (struct sockaddr *) &sa, sizeof sa);

    // If we attempt to connect too fast to aperios ipstack it will fail.
    if (rc)
    {
      usleep(20000);
      rc = connect(sd, (struct sockaddr *) &sa, sizeof sa);
    }

    // Check there was no error.
    if (rc)
    {
      perror("UClient::UClient connect");
      return;
    }

    //check that it really worked
    int pos=0;
    while (pos==0)
      pos = ::recv(sd, recvBuffer, buflen, 0);
    if (pos<0)
    {
      perror("UClient::UClient recv");
      rc = pos;
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
    if (close(sd) == -1)
      perror ("cannot close sd");
    sd = -1;
    if (control_fd[1] != -1
	&& ::write(control_fd[1], "a", 1) == -1)
      perror ("cannot write to control_fd[1]");

    // If the connection has failed while building the client, the
    // thread is not created.
    if (thread)
      // Must wait for listen thread to terminate.
      libport::joinThread(thread);

    if (control_fd[1] != -1
	&& close(control_fd[1]) == -1)
      perror ("cannot close controlfd[1]");
    if (control_fd[0] != -1
	&& close(control_fd[0]) == -1)
      perror ("cannot close controlfd[0]");
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
	clientError("send error", rc);
	return rc;
      }
      pos += retval;
    }
    return 0;
  }

  void
  UClient::listenThread()
  {
    fd_set rfds, efds;
    int maxfd=1+ (sd>control_fd[0]? sd:control_fd[0]);
    int res;
    while (true)
    {
      do {
	if (sd==-1)
	  return;
	FD_ZERO(&rfds);
	FD_ZERO(&efds);
	LIBPORT_FD_SET(sd, &rfds);
	LIBPORT_FD_SET(sd, &efds);
#ifndef WIN32
	LIBPORT_FD_SET(control_fd[0], &rfds);
#endif
	struct timeval tme;
	tme.tv_sec = 1;
	tme.tv_usec = 0;
	res = select(maxfd+1, &rfds, NULL, &efds, &tme);
	if (res < 0 && errno != EINTR)
	{
	  rc = -1;
#ifdef WIN32
	  res = WSAGetLastError();
#endif
	  clientError("select error", res);
	  std::cerr << "select error "<<res<<std::endl;
	  //TODO when error will be implemented, send an error msg
	  //TODO maybe try to reconnect?
	  return;
	}
	if (res == -1)
	{
	  res = 0;
	  continue;
	}
#ifndef WIN32
	if (res != 0 && FD_ISSET(control_fd[0], &rfds))
	  return;
#endif
      } while (!res);
      int count = ::recv(sd,
			 &recvBuffer[recvBufferPosition],
			 buflen - recvBufferPosition - 1, 0);
      //TODO when error will be implemented, send an error msg
      //TODO maybe try to reconnect?
      if (count < 0)
      {
	perror ("recv error");
	clientError("recv error, errno =", count);
	rc = -1;
	return;
      }
      else if (!count)
      {
	std::cerr << "recv error: connection closed" << std::endl;
	clientError("recv error: connection closed");
	rc = -1;
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

  void execute()
  {
    while (true)
      sleep(100);
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
