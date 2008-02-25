/*! \file uclient.cc
****************************************************************************
 *
 * Linux implementation of the URBI interface class
 *
 * Copyright (C) 2004, 2006, 2007, 2008 Jean-Christophe Baillie.  All rights reserved.
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
#else
# include "libport/windows.hh"
#endif

#include "urbi/uclient.hh"
#include "urbi/utag.hh"
#include "libport/network.h"
#include "libport/lockable.hh"
#include "libport/thread.hh"
#include "libport/utime.hh"
#include "libport/stdio.hh"
#include "libport/string.hh"

namespace urbi
{
  /*! Establish the connection with the server.
   Spawn a new thread that will listen to the socket, parse the incoming URBI
   messages, and notify the appropriate callbacks.
   */
  UClient::UClient(const char *_host, int _port, int _buflen)
    : UAbstractClient(_host, _port, _buflen),
      thread(0),
      pingInterval(0)
  {
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
      libport::perror("UClient::UClient socket");
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
      libport::perror("UClient::UClient connect");
      return;
    }

    //check that it really worked
    int pos=0;
    while (pos==0)
      pos = ::recv(sd, recvBuffer, buflen, 0);
    if (pos<0)
    {
      libport::perror("UClient::UClient recv");
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
    if (libport::closeSocket(sd) == -1)
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
    std::cerr << ">>>> SENT : [" << output << "]" << std::endl;
#endif
    if (rc)
      return -1;
    int pos = 0;
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


  UCallbackAction
  UClient::pongCallback(const UMessage &)
  {
    waitingPong = false;
    return URBI_CONTINUE;
  }


  void
  UClient::listenThread()
  {
    const char* pongTag = "__URBI_INTERNAL_PONG";

    int maxfd;

    maxfd = 1 + std::max(sd, control_fd[0]);
    waitingPong = false;

    setCallback(callback(*this, &UClient::pongCallback), pongTag);

    while (true)
    {
      if (sd == -1)
        return;

      fd_set rfds;
      fd_set efds;

      FD_ZERO(&rfds);
      FD_ZERO(&efds);
      LIBPORT_FD_SET(sd, &rfds);
      LIBPORT_FD_SET(sd, &efds);
#ifndef WIN32
      LIBPORT_FD_SET(control_fd[0], &rfds);
#endif

      int selectReturn;
      if (pingInterval != 0)
      {
        const unsigned delay = waitingPong ? pongTimeout : pingInterval;
        struct timeval timeout = { delay / 1000, (delay % 1000) * 1000};
        selectReturn = ::select(maxfd + 1, &rfds, NULL, &efds, &timeout);
      }
      else
      {
        selectReturn = ::select(maxfd + 1, &rfds, NULL, &efds, NULL);
      }

      // Treat error
      if (selectReturn < 0 && errno != EINTR)
      {
        int errorCode = selectReturn;
#ifdef WIN32
        errorCode = WSAGetLastError();
#endif

        rc = -1;
        clientError("Connection error : ", errorCode);
        notifyCallbacks(UMessage(*this, 0, connectionTimeoutTag.c_str(),
                                 "!!! Connection error", std::list<BinaryData>() ));
        return;
      }
      if (selectReturn < 0)  // ::select catch a signal (errno == EINTR)
        continue;

      // timeout
      if (selectReturn == 0)
      {
        if (waitingPong) // Timeout while waiting PONG
        {
          rc = -1;
          // FIXME: Choose between two differents way to alert user program
          clientError("Lost connection with server", 0);
          notifyCallbacks(UMessage(*this, 0, connectionTimeoutTag.c_str(),
                                   "!!! Lost connection with server",
                                   std::list<BinaryData>() ));
          return;
        }
        else // Timeout : Ping_interval
        {
          send("%s << 1,", pongTag);
          waitingPong = true;
        }
      }

      if (selectReturn > 0)
      {
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


  UClient& connect(const char* host)
  {
    return *new UClient(host);
  }

  void disconnect(UClient &client)
  {
    delete &client;
  }

  void UClient::setKeepAliveCheck(const unsigned    pingInterval,
                                  const unsigned    pongTimeout)
  {
    this->pingInterval = pingInterval;
    this->pongTimeout  = pongTimeout;
  }

} // namespace urbi
