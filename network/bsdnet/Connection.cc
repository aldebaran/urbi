#ifdef WIN32
#define GROUP __GROUP
#include <winsock2.h>
#undef GROUP
#define YYTOKENTYPE
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif
#include "Connection.h"

extern UServer * THESERVER;
/**
 * \file linuxconnection.cpp
 * \brief UConnection specialization for Linux
 * @author Anthony Truchet from a previous work by Arnaud Sarthou
 */



//! LinuxConnection constructor.
/*! The constructor calls UConnection::UConnection with the appropriate
    parameters. 
    The global variable ::linuxserver saves the need to pass a UServer parameter 
    to the LinuxConnection constructor. 

    UError can have the following values:
    -  USUCCESS: success
    -  UFAIL   : UConnection or memory allocation failed
*/ 
Connection::Connection(int connfd) :
                UConnection   ( (UServer*) THESERVER,
                                 Connection::MINSENDBUFFERSIZE,
                                 Connection::MAXSENDBUFFERSIZE,
                                 Connection::PACKETSIZE, 
                                 Connection::MINRECVBUFFERSIZE,
                                 Connection::MAXRECVBUFFERSIZE), 
                fd(connfd) {
	if (UError != USUCCESS) {// Test the error from UConnection constructor.
	  //baad
	  closeConnection();
	}
	else {

		initialize();
	}
}

//! Connection destructor.
Connection::~Connection() 
{
	if (fd!=0)
		closeConnection();
}

//! Close the connection
/*! 
*/
UErrorValue
Connection::closeConnection() {
	int ret;
	// Setting 'closing' to true tell the kernel not to use the connection any longer
	closing=true;
#ifdef WIN32
    closesocket(fd);
    ret = 0;//WSACleanup(); //wsastartup called only once!
#else
    ret = close(fd);
#endif
    Network::unregisterNetworkPipe(this);
    if (ret!=0) {
    	return UFAIL;
    }
    else {
    	fd=-1;
    	//THESERVER.removeConnection(this);
    	return USUCCESS;
    }
    
}

// Try for a trick on Mac OS X
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

void Connection::doRead(){

	int n = ::recv(fd, (char *)read_buff, PACKETSIZE, MSG_NOSIGNAL);
	if(n<=0){
	  //kill us
	    closeConnection();
	}
	else
	  received(read_buff, n);
	
}

int Connection::effectiveSend (const ubyte *buffer, int length){

	int ret = ::send(fd, (char *)buffer, length, MSG_NOSIGNAL);
	if(ret<=0){
	  //kill us
	  closeConnection();
	  return -1;
	}
	else
	  return ret; // Number of bytes actually written.
}

void Connection::doWrite(){
	continueSend();
}
