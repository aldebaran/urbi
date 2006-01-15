/*! \file uclient.cpp
****************************************************************************
 * $Id: uclient.cpp,v 1.9 2005/09/30 17:48:00 nottale Exp $
 *
 * Linux implementation of the URBI interface class
 *
 * Copyright (C) 2004 Jean-Christophe Baillie.  All rights reserved.
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
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <locale.h>
#include "uclient.h"

using namespace std;


static void *listenThreadStarter(void *objectPtr)
{
  UClient *connection = (UClient *) objectPtr;
  connection->listenThread();
}

/*! Establish the connection with the server. 
Spawn a new thread that will listen to the socket, parse the incoming URBI messages, and notify
the appropriate callbacks.
 */
UClient::UClient(const char *_host, int _port, int _buflen) 
  :UAbstractClient(_host, _port, _buflen) {
    setlocale(LC_NUMERIC,"C");
    control_fd[0] = control_fd[1] = -1;
    if (::pipe(control_fd) == -1) {
      rc = -1;
      perror("UClient::UClient failed to create pipe");
      return;
    }
    pthread_mutexattr_t ma;
    pthread_condattr_t ca;
    pthread_mutexattr_init(&ma);
    
    pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&writeLock, &ma);
    //MUST BE RECURSIVE if a callback calls notifycallback 
    //pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_FAST_NP);
    pthread_mutex_init(&listLock, &ma);
    listenThreadStruct = NULL;
    
    // Address resolution stage.
    struct hostent *hen;		// host-to-IP translation
    struct sockaddr_in sa;	// Internet address struct
    
   
    memset(&sa, 0, sizeof(sa));
    
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    
    hen = gethostbyname(host);
    
    if (!hen) {	//maybe it is an IP address
      sa.sin_addr.s_addr = inet_addr(host);
      if (sa.sin_addr.s_addr == INADDR_NONE) {
        printf("UClient::UClient couldn't resolve host name.\n");
        rc = -1;
        return;
      }
    }
    
    else
      memcpy(&sa.sin_addr.s_addr, hen->h_addr_list[0], hen->h_length);
    
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0) {
      printf("UClient::UClient socket allocation failed.\n");
      rc = -1;
      return;
    }
    
    // now connect to the remote server. 
    rc = connect(sd, (struct sockaddr *) &sa, sizeof(sa));
    
    // If we attempt to connect too fast to aperios ipstack it will fail.
    if (rc) {
      usleep(20000);
	rc = connect(sd, (struct sockaddr *) &sa, sizeof(sa));
    }
    
    // Check there was no error.
    if (rc) {
      printf("UClient::UClient couldn't connect.\n");
      return;
    }
    
    if (rc) return;
    
    //check that it really worked
    int pos=0;
    while (pos==0)
      pos = read(sd, recvBuffer, buflen);
    if (pos<0) {
      rc = pos;
      printf("UClient::UClient couldn't connect: read error.\n");
      return;
    }
    else recvBufferPosition = pos;
    recvBuffer[recvBufferPosition] = 0;
    listenThreadStruct = new pthread_t();
    pthread_create(listenThreadStruct, NULL, listenThreadStarter, this);
    if (!urbi::defaultClient)
      urbi::defaultClient =  this;
  }




UClient::~UClient()
{
  
  close(sd);
  if (control_fd[1] != -1 ) ::write(control_fd[1],"a",1);
  //must wait for listen thread to terminate
  if (listenThreadStruct)
	pthread_join(*listenThreadStruct, NULL);
  pthread_mutex_destroy(&writeLock);
  pthread_mutex_destroy(&listLock);
  if (control_fd[1] != -1 ) close(control_fd[1]);
  if (control_fd[0] != -1 ) close(control_fd[0]);
}


bool 
UClient::canSend(int size) {
  return true;
}


int 
UClient::effectiveSend(const void  * buffer, int size) {
#if DEBUG
char output[size+1];
memcpy((void*)output, buffer, size);
output[size]=0;
cout << ">>>> SENT : [" << output << "]" << endl;
#endif
  if (rc) return -1;
  int pos = 0;
  while (pos!=size) {
    int retval = ::write(sd, (char *) buffer + pos, size-pos);
	if (retval<0) {
	  rc = retval;
	  return rc;
	}
	pos += retval;
  }
  return 0;
}

void 
UClient::listenThread() {
  fd_set rfds;
  int maxfd=1+ (sd>control_fd[0]? sd:control_fd[0]);
  int res;
  while (true) {
	do {
	  FD_ZERO(&rfds);
	  FD_SET(sd, &rfds);
	  FD_SET(control_fd[0], &rfds);
	  res = select(maxfd+1, &rfds, NULL, NULL, NULL);
	  if ( (res == -1) && (errno != EINTR)) {
		this->rc = -1;
		//TODO when error will be implemented, send an error msg
		//TODO maybe try to reconnect?
		return;
	  }
	  if (res == -1) { res=0;continue;}
	  if ( (res != 0) && (FD_ISSET(control_fd[0], &rfds)) ) return;
	}
	while (res == 0);	
	int count = read(sd, 
                     &recvBuffer[recvBufferPosition], 
                     buflen - recvBufferPosition - 1);
	if (count == -1) {
	  rc = -1;
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
UClient::lockList() {
  pthread_mutex_lock(&listLock);
}

void 
UClient::lockSend() {
  pthread_mutex_lock(&writeLock);
}

void 
UClient::unlockList() {
  pthread_mutex_unlock(&listLock);
}

void 
UClient::unlockSend() {
  pthread_mutex_unlock(&writeLock);
}



void 
UClient::printf(const char * format, ...) {
  va_list arg;
  va_start(arg, format);
  vfprintf(stderr, format, arg);
  va_end(arg);
}

unsigned int UClient::getCurrentTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1000+tv.tv_usec/1000;
} 

void urbi::execute(void) {
  while (true) sleep(100);
}

void urbi::exit(int code) {
  ::exit(code);
}


UClient & urbi::connect(const char  * host) {
  return *new UClient(host);
}
