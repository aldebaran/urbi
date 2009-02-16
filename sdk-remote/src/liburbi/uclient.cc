/*! \file uclient.cc
****************************************************************************
 *
 * Implementation of the URBI interface class
 *
 * Copyright (C) 2004, 2006, 2007, 2008, 2009 Gostai S.A.S.  All rights reserved.
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
#include <cerrno>

#include <locale.h>

#include <libport/windows.hh>
#include <libport/unistd.h>

#include <libport/sys/time.h>
#if !defined WIN32
# include <time.h>
# include <signal.h>
#endif

#include <libport/cstdio>
#include <libport/sys/select.h>
#include <libport/arpa/inet.h>
#include <libport/netdb.h>
#include <libport/errors.hh>
#include <libport/lockable.hh>
#include <libport/thread.hh>
#include <libport/utime.hh>

#include <urbi/uclient.hh>
#include <urbi/utag.hh>

namespace urbi
{
  /*! Establish the connection with the server.
   Spawn a new thread that will listen to the socket, parse the incoming URBI
   messages, and notify the appropriate callbacks.
   */
  UClient::UClient(const std::string& host, unsigned port,
                   size_t buflen, bool server,
                   int semListenInc)
    : UAbstractClient(host, port, buflen, server)
    , thread(0)
    , pingInterval(0)
    , semListenInc_ (semListenInc)
  {
    sd = -1;
    int pos = 0;

    setlocale(LC_NUMERIC, "C");
    control_fd[0] = control_fd[1] = -1;

#ifndef WIN32
    if (::pipe(control_fd) == -1)
    {
      rc = -1;
      libport::perror("UClient::UClient failed to create pipe");
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
    wVersionRequested = MAKEWORD(1, 1);
    WSAStartup(wVersionRequested, &wsaData);
#endif
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    // host-to-IP translation
    struct hostent* hen = gethostbyname(host_.c_str());
    if (!hen)
    {
      // maybe it is an IP address
      sa.sin_addr.s_addr = inet_addr(host_.c_str());
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
      rc = -1;
      libport::perror("UClient::UClient socket");
      return;
    }

    if (!server_)
    {
      // Connect on given host and port
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
	rc = -1;
	libport::perror("UClient::UClient connect");
	return;
      }

      // Check that it really worked.
      while (!pos)
	pos = ::recv(sd, recvBuffer, buflen, 0);
      if (pos < 0)
      {
	rc = -1;
	libport::perror("UClient::UClient recv");
	return;
      }

    }
    else
    {
      // Allow to rebind on the same port shortly after having used it.
      {
	int one = 1;
	rc = libport::setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,
                                 &one, sizeof one);
	if (rc)
	{
	  rc = -1;
	  libport::perror("UClient::UClient cannot use setsockopt");
	  return;
	}
      }
      // Bind socket
      rc = bind (sd, (struct sockaddr *) &sa, sizeof sa);
      if (rc)
      {
	rc = -1;
	libport::perror("UClient::UClient cannot bind");
	return;
      }
      // Activate listen/passive mode, do not allow queued connections
      rc = listen (sd, 0);
      if (rc)
      {
	rc = -1;
	libport::perror("UClient::UClient cannot listen");
	return;
      }

      // Create a thread waiting for incoming connection.
      // This must not be blocking in case of a remote server.
      // FIXME: block if normal remote ?
      init_ = false;
      thread = libport::startThread(this, &UClient::acceptThread);
    }

    recvBufferPosition = pos;
    recvBuffer[recvBufferPosition] = 0;

    // Do not create thread if one is already waiting for incoming connection
    if (!thread)
    {
      thread = libport::startThread(this, &UClient::listenThread);
      // Notify the base class that connection is established.
      onConnection();
    }

    if (!defaultClient)
      defaultClient = this;

    listenSem_++;
    acceptSem_++;

  }

  UClient::~UClient()
  {
    if (sd >= 0)
      closeUClient ();
  }

  int
  UClient::closeUClient ()
  {
    if (sd >= 0 && libport::closeSocket(sd) == -1)
      libport::perror ("cannot close sd");

    sd = -1;
    if (control_fd[1] != -1
        && ::write(control_fd[1], "a", 1) == -1)
      libport::perror ("cannot write to control_fd[1]");

    // If the connection has failed while building the client, the
    // thread is not created.
    if (thread)
      // Must wait for listen thread to terminate.
      libport::joinThread(thread);

    if (control_fd[1] != -1
        && close(control_fd[1]) == -1)
      libport::perror ("cannot close controlfd[1]");
    if (control_fd[0] != -1
        && close(control_fd[0]) == -1)
      libport::perror ("cannot close controlfd[0]");

    return 0;
  }

  int
  UClient::effectiveSend(const void* buffer, size_t size)
  {
#if DEBUG
    char output[size+1];
    memcpy (static_cast<void*> (output), buffer, size);
    output[size]=0;
    std::cerr << ">>>> SENT : [" << output << "]" << std::endl;
#endif
    if (rc)
      return -1;
    size_t pos = 0;
    while (pos != size)
    {
      int retval = ::send(sd, (char *) buffer + pos, size-pos, 0);
      if (retval< 0)
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
  UClient::acceptThread()
  {
    // Wait for it...
    acceptSem_--;

    // Accept one connection
    struct sockaddr_in saClient;
    socklen_t addrlenClient;
    int acceptFD = 0;

    acceptFD = accept (sd, (struct sockaddr *) &saClient, &addrlenClient);
    if (acceptFD < 0)
    {
      libport::perror("UClient::UClient cannot accept");
      rc = -1;
      return;
    }

    // Store client connection info
    host_ = inet_ntoa(saClient.sin_addr);
    port_ = saClient.sin_port;

    // Do not listen anymore.
    close(sd);
    // Redirect send/receive on accepted connection.
    sd = acceptFD;

    // FIXME: leaking ?
    thread = libport::startThread(this, &UClient::listenThread);

    init_ = true;
    onConnection();

    // Stop this thread, the listen one is the real thing.
    return;
  }

  void
  UClient::listenThread()
  {
    // Wait for it...
    for (int i = semListenInc_; i > 0; --i)
      listenSem_--;

    int maxfd = 1 + std::max(sd, control_fd[0]);
    waitingPong = false;
    // Declare ping channel for kernel that requires it.
    send("if (isdef(Channel)) var lobby.%s = Channel.new(\"%s\") | {};",
	internalPongTag.c_str(), internalPongTag.c_str());
    while (true)
    {
      if (sd == -1)
        return;

      fd_set rfds;
      FD_ZERO(&rfds);
      LIBPORT_FD_SET(sd, &rfds);

      fd_set efds;
      FD_ZERO(&efds);
      LIBPORT_FD_SET(sd, &efds);

#ifndef WIN32
      LIBPORT_FD_SET(control_fd[0], &rfds);
#endif

      int selectReturn;
      if (pingInterval)
      {
        const unsigned delay = waitingPong ? pongTimeout : pingInterval;
        struct timeval timeout = { delay / 1000, (delay % 1000) * 1000};
        selectReturn = ::select(maxfd + 1, &rfds, NULL, &efds, &timeout);
      }
      else
      {
        selectReturn = ::select(maxfd + 1, &rfds, NULL, &efds, NULL);
      }

      if (sd < 0)
	return;

      // Treat error
      if (selectReturn < 0 && errno != EINTR)
      {
        rc = -1;
        clientError("Connection error : ", errno);
        notifyCallbacks(UMessage(*this, 0, connectionTimeoutTag.c_str(),
                                 "!!! Connection error", std::list<BinaryData>() ));
        return;
      }

      if (selectReturn < 0)  // ::select catch a signal (errno == EINTR)
        continue;
      // timeout
      else if (selectReturn == 0)
      {
        if (waitingPong) // Timeout while waiting PONG
        {
          rc = -1;
          // FIXME: Choose between two differents way to alert user program
          clientError("Lost connection with server");
          notifyCallbacks(UMessage(*this, 0, connectionTimeoutTag.c_str(),
                                   "!!! Lost connection with server",
                                   std::list<BinaryData>() ));
          return;
        }
        else // Timeout : Ping_interval
        {
          send("%s << 1,", internalPongTag.c_str());
          waitingPong = true;
        }
      }
      else
      {
        // We receive data, at least the "1" value sent through the pong tag
        // channel so we are no longer waiting for a pong.
        waitingPong = false;
        int count = ::recv(sd, &recvBuffer[recvBufferPosition],
                           buflen - recvBufferPosition - 1, 0);

        if (count <= 0)
        {
          std::string errorMsg;
          int         errorCode = 0;

          if (count < 0)
          {
#ifdef WIN32
            errorCode = WSAGetLastError();
#else
            errorCode = errno;
#endif
            errorMsg = "!!! Connection error";
          }
          else // count == 0  => Connection close
          {
            errorMsg = "!!! Connection closed";
          }

          rc = -1;
          clientError(errorMsg.c_str(), errorCode);
          notifyCallbacks(UMessage(*this, 0, connectionTimeoutTag.c_str(),
                                   errorMsg.c_str(), std::list<BinaryData>() ));
          return;
        }

        recvBufferPosition += count;
        recvBuffer[recvBufferPosition] = 0;
        processRecvBuffer();
      }
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


  UClient& connect(const std::string& host)
  {
    return *new UClient(host);
  }

  void disconnect(UClient &client)
  {
    delete &client;
  }

  void
  UClient::setKeepAliveCheck(const unsigned pingInterval,
                             const unsigned pongTimeout)
  {
    this->pingInterval = pingInterval;
    this->pongTimeout  = pongTimeout;
  }

} // namespace urbi
