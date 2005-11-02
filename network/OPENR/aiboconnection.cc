/*! \file aiboconnection.cc
 *******************************************************************************

 File: aiboconnection.cc\n
 Implementation of the AiboConnection class.

 This file is part of 
 %URBI Server Aibo, version __rsaiboversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://urbi.sourceforge.net

 **************************************************************************** */

#include <OPENR/core_macro.h>
#include <OPENR/OSyslog.h>
#include <OPENR/OPENRAPI.h>
#include <ant.h>
#include <EndpointTypes.h>
#include <TCPEndpointMsg.h>
#include <string.h>

#include <userver.h>

#include "URBI.h"
#include "entry.h"
#include "aiboserver.h"
#include "aiboconnection.h"

//! AiboConnection constructor.
/*! The constructor calls UConnection::UConnection with the appropriate
    parameters. 
    The global variable ::aiboserver saves the need to pass a UServer parameter 
    to the AiboConnection constructor. 

    UError can have the following values:
    -  USUCCESS: success
    -  UFAIL   : UConnection or OPENR memory allocation failed
*/ 
AiboConnection::AiboConnection() :
                UConnection   ( ::urbiserver,
                                 AiboConnection::MINSENDBUFFERSIZE,
                                 AiboConnection::MAXSENDBUFFERSIZE,
                                 AiboConnection::PACKETSIZE, 
                                 AiboConnection::MINRECVBUFFERSIZE,
                                 AiboConnection::MAXRECVBUFFERSIZE)
{
  ADDOBJ(AiboConnection);

  if (UError != USUCCESS) return;// Test the error from UConnection constructor.
	
  // Initialize connection specific parameters

  cameraResolution      = ofbkimageLAYER_H;
  cameraFormat          = 1; // JPEG
  cameraJpegfactor      = 80;
  cameraReconstruct     = false;

  // Allocate OPENR send buffer
  antEnvCreateSharedBufferMsg sendBufferMsg(AiboConnection::PACKETSIZE);

  sendBufferMsg.Call(::ipstackRef, sizeof(sendBufferMsg));
  if (sendBufferMsg.error != ANT_SUCCESS) {
    
    server->error(::DISPLAY_FORMAT1, (int)this,
                  "AiboConnection::AiboConnection",
                  "Can't allocate send buffer",
                  sendBufferMsg.error);
    UError = UFAIL;
    return;
  }
  ADDMEM(AiboConnection::PACKETSIZE); // Estimation of the size of the mysterious
                                      // OPENR send Buffer...

  antSendBuffer_ = sendBufferMsg.buffer;
  antSendBuffer_.Map();
  antSendData_ = (byte*)(antSendBuffer_.GetAddress());

  // Allocate OPENR receive buffer
  antEnvCreateSharedBufferMsg recvBufferMsg(AiboConnection::PACKETSIZE);

  recvBufferMsg.Call(::ipstackRef, sizeof(recvBufferMsg));
  if (recvBufferMsg.error != ANT_SUCCESS) {
    
    server->error(::DISPLAY_FORMAT1, (int)this,
                  "AiboConnection::AiboConnection",
                  "Can't allocate receive buffer",
                  recvBufferMsg.error);
    UError = UFAIL;
    return;
  }
  
  ADDMEM(AiboConnection::PACKETSIZE); // Estimation of the size of the mysterious
                                      // OPENR recv Buffer...
  antRecvBuffer_ = recvBufferMsg.buffer;
  antRecvBuffer_.Map();
  antRecvData_ = (byte*)(antRecvBuffer_.GetAddress());
  
  // Connection state
  isListening          = false;
  isClosing            = false;
  recoverFromIsolation = false;

  // Create IPStack endpoint
  antEnvCreateEndpointMsg tcpCreateMsg(EndpointType_TCP,
				       AiboConnection::PACKETSIZE*2); // why *2?

  tcpCreateMsg.Call(::ipstackRef, sizeof(tcpCreateMsg));

  if (tcpCreateMsg.error != ANT_SUCCESS)  {

    server->error(::DISPLAY_FORMAT1, (int)this,
                  "AiboConnection::AiboConnection",
                  "Can't create endpoint",
                  tcpCreateMsg.error);

    UError = UFAIL;
    return;
  }

  ADDMEM(AiboConnection::PACKETSIZE*2); // Estimation of the size of the mysterious
                                        // OPENR send Buffer...
  endpoint_ = tcpCreateMsg.moduleRef;
  
  disactivate(); // disactivated until listen returns...
  // Start to listen on the connection
  UError = oListen();
}

//! AiboConnection destructor.
AiboConnection::~AiboConnection() 
{
  FREEOBJ(AiboConnection);
}

//! Send a "listen" request to the IPStack object.
/*! This function sends a message to the IPStack object to start listening 
    for incoming connections.

    As soon as a new connection is detected, the URBI::TCPListenCont() callback
    will be trigerred.
*/
UErrorValue
AiboConnection::oListen()
{
  // Send the "Listen" request message
  TCPEndpointListenMsg listenMsg(endpoint_,
				 IP_ADDR_ANY, 
                                 AiboServer::TCP_PORT);

  listenMsg.continuation = (void*)this; // Used to recover in the callback.

  listenMsg.Send(::ipstackRef, ::URBI_OID,
		 Extra_Entry[entryTCPListenCont], sizeof(listenMsg));
  
  server->echo(::DISPLAY_FORMAT, (int)this,
               "AiboConnection::oListen",  
               "Connection starts to listen...");
  isListening = true;
  return USUCCESS;
}

//! Send a buffer to IPStack, so that it will be send through the connection
/*! This function sends a message to the IPStack object, requesting to send
    a buffer of data. The connection is blocked.

    As soon as the connection is ready to send another packet, the 
    URBI::TCPSendCont() callback will be trigerred and the connection will be
    unblocked by calling UServer::continueSend().
*/
int
AiboConnection::effectiveSend(const ubyte *buffer, int length)
{    
  // Transfert to IPStack shared buffer 
  // NB: in fact ubyte == byte, it's just because one come from the URBI kernel
  // and the other one goes in the OPENR API.
  memcpy(antSendData_, (byte*)buffer, length);
  
  // Send message to IPSTack
  TCPEndpointSendMsg sendMsg(endpoint_, antSendData_, length);  
  sendMsg.continuation = (void*)this;
    
  sendMsg.Send(::ipstackRef, ::URBI_OID,
               Extra_Entry[entryTCPSendCont],
               sizeof(TCPEndpointSendMsg));
   
  // Block sending until the connection is ready again (callback SendCont)
  block();
  
  server->echoKey(">>>>>",::DISPLAY_FORMAT, (int)this,
               "AiboConnection::effectiveSend",  
               "Sending is done...");
  return length; // with aibo: what you request is what you get...
}

//! Send a "receive" request to the IPStack object.
/*! This function sends a message to the IPStack object, requesting to start
    receiving data.

    As soon as a new data has arrived, the URBI::TCPReceiveCont() callback
    will be trigerred.
*/
UErrorValue
AiboConnection::oReceive()
{
  // Send the "Receive" request message
  TCPEndpointReceiveMsg receiveMsg(endpoint_,
				   antRecvData_,
				   1, AiboConnection::PACKETSIZE);
 
  receiveMsg.continuation = (void*)this;

  receiveMsg.Send(::ipstackRef, ::URBI_OID,
		  Extra_Entry[entryTCPReceiveCont], sizeof(receiveMsg));

  server->echo(::DISPLAY_FORMAT, (int)this,
               "AiboConnection::oReceive",  
               "Connection waits to receive...");
  return USUCCESS;
}

//! Close the connection
/*! This function call the robot-specific oClose() function.
*/
UErrorValue
AiboConnection::closeConnection()
{
  return (oClose());
}


//! Send a "close" request to the IPStack object.
/*! This function sends a message to the IPStack object, requesting to close the
    connection.

    As soon as the IPStack internal closing is done, the URBI::TCPCloseCont() 
    callback will be trigerred.
*/
UErrorValue
AiboConnection::oClose()
{
  closing = true; // kernel notification. Mandatory.

  // Free the shared buffers
  antRecvBuffer_.UnMap();
  antEnvDestroySharedBufferMsg receiveBufferMsg(antRecvBuffer_);
  receiveBufferMsg.Call(::ipstackRef,sizeof(antEnvDestroySharedBufferMsg));
  FREEMEM(AiboConnection::PACKETSIZE); 

  antSendBuffer_.UnMap();
  antEnvDestroySharedBufferMsg sendBufferMsg(antSendBuffer_);
  sendBufferMsg.Call(::ipstackRef,sizeof(antEnvDestroySharedBufferMsg));
  FREEMEM(AiboConnection::PACKETSIZE); 
 
  // Send the "Close" request message
  TCPEndpointCloseMsg closeMsg(endpoint_);
  closeMsg.continuation = (void*)this;
  
  closeMsg.Send(::ipstackRef, ::URBI_OID,
		Extra_Entry[entryTCPCloseCont], sizeof(closeMsg));

  server->echo(::DISPLAY_FORMAT, (int)this,
               "AiboConnection::oClose",  
               "Connection waits to close...");
  isClosing = true;

  return USUCCESS;
}
