/*! \file uclient.cpp
****************************************************************************
 * $Id: uclient.cpp,v 1.2 2005/09/21 06:45:36 nottale Exp $
 *
 * Windows implementation of the URBI interface class
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
#include <Windows.h>
#include <winsock.h>
#include <stdlib.h>
#include <stdio.h>

#include "uclient.h"





static DWORD WINAPI listenThreadStarter(void *objectPtr)
{
  UClient *connection = (UClient *) objectPtr;
  connection->listenThread();
  return 0;
}

/*! Establish the connection with the server. 
Spawn a new thread that will listen to the socket, parse the incoming URBI messages, and notify
the appropriate callbacks.
 */
UClient::UClient(const char *_host, int _port, int _buflen) 
  :UAbstractClient(_host, _port, _buflen) {
   setlocale(LC_NUMERIC,"C"); 
   InitializeCriticalSection(&listLock); 
   InitializeCriticalSection(&writeLock);
   WSADATA localWSA;
   WSAStartup(MAKEWORD(1,1),&localWSA);
    // Address resolution stage.
   
    sockaddr_in SockAddr;
	ZeroMemory(&SockAddr,sizeof(sockaddr_in));
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port= htons(port);
   
    sd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    HOSTENT *hp=gethostbyname(host);
	if (!hp) {
		SockAddr.sin_addr.s_addr=inet_addr(host);
		if (SockAddr.sin_addr.s_addr==INADDR_NONE) {        
 	       rc=-1;
           return ;  
		}    
    }
	else CopyMemory(&SockAddr.sin_addr,hp->h_addr,4);  
	if (connect(sd,(sockaddr *)&SockAddr,sizeof(sockaddr_in))==SOCKET_ERROR)
		{
        Sleep(20);
        if (connect(sd,(sockaddr *)&SockAddr,sizeof(sockaddr_in))==SOCKET_ERROR) {
		   rc=-1;
		   return ;
		   }
     }      
	
    
    //check that it really worked
    int pos=0;
    while (pos==0)
      pos = recv(sd, recvBuffer, buflen,0);
    if (pos<0) {
      rc = pos;
      printf("UClient::UClient couldn't connect: read error.\n");
      return;
    }
    else recvBufferPosition = pos;
    recvBuffer[recvBufferPosition] = 0;
    DWORD id;
    listenThreadStruct=CreateThread(NULL, 0, &listenThreadStarter,this, 0, &id);
    if (!urbi::defaultClient)
      urbi::defaultClient =  this;
}




UClient::~UClient()
{
  
  closesocket(sd);
  //must wait for listen thread to terminate
  WaitForSingleObject(listenThreadStruct, INFINITE);
  DeleteCriticalSection(&writeLock);
  DeleteCriticalSection(&listLock);

}


bool 
UClient::canSend(int size) {
  return true;
}


int 
UClient::effectiveSend(const void  * buffer, int size) {
  if (rc) return -1;
  int pos = 0;
  while (pos!=size) {
	int retval = ::send(sd, (char *) buffer + pos, size-pos,0);
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
  
  while (true) {
	
	int count = recv(sd, 
                     &recvBuffer[recvBufferPosition], 
                     buflen - recvBufferPosition - 1,0);
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
  EnterCriticalSection(&listLock);
}

void 
UClient::lockSend() {
  EnterCriticalSection(&writeLock);
}

void 
UClient::unlockList() {
  LeaveCriticalSection(&listLock);
}

void 
UClient::unlockSend() {
  LeaveCriticalSection(&writeLock);
}



void 
UClient::printf(const char * format, ...) {
  va_list arg;
  va_start(arg, format);
  vfprintf(stderr, format, arg);
  va_end(arg);
}

unsigned int UClient::getCurrentTime() {
  return GetTickCount();
} 

void urbi::execute(void) {
  while (true) Sleep(1000);
}

void urbi::exit(int code) {
  ::exit(code);
}


UClient & urbi::connect(const char  * host) {
  return *new UClient(host);
}


BOOL APIENTRY DllMain(HANDLE, DWORD, LPVOID) {
return true;
}





//int main() {}
