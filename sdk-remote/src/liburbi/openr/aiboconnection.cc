/*! \file aiboconnection.cc
 */

#include "aiboconnection.h"
#include "CLIENT.h"
using std::min;
//! AiboConnection constructor.
/*! The constructor calls UConnection::UConnection with the appropriate
    parameters.
*/

  const int AiboConnection::MINBUFFERSIZE = 4096;
  const int AiboConnection::MAXBUFFERSIZE = 128000;
  const int AiboConnection::PACKETSIZE    = 65536;

AiboConnection::AiboConnection(antStackRef	ipstackRef,
							   OID			ID,
							   unsigned int	port,
							   const char	*host,
							   UClient		*ref) 
{
  // Attribute initialization
  ref_ = ref;
  ipstackRef_ = ipstackRef;
  ID_ = ID;
  host_ = strdup(host);
  port_ = port;
  sendBuffer_ = new UQueue(MINBUFFERSIZE,MAXBUFFERSIZE,1);
  // Connection state
  isSending = false;

  // Allocate OPENR send buffer
  
  antEnvCreateSharedBufferMsg		sendBufferMsg(PACKETSIZE);
  sendBufferMsg.Call(ipstackRef_,
					 sizeof (antEnvCreateSharedBufferMsg));
  if (sendBufferMsg.error != ANT_SUCCESS)
  	{
  	  OSYSDEBUG(("%s : antError %d\n",
				 "Can't allocate send buffer",
				 sendBufferMsg.error));
	  return ;
  	}
  antSendBuffer_ = sendBufferMsg.buffer;
  antSendBuffer_.Map();
  antSendData_ = (byte *)(antSendBuffer_.GetAddress());

  // Allocate OPENR receive buffer

  antEnvCreateSharedBufferMsg		recvBufferMsg(PACKETSIZE);
  recvBufferMsg.Call(ipstackRef_,
					 sizeof (antEnvCreateSharedBufferMsg));
  if (recvBufferMsg.error != ANT_SUCCESS)
  	{
  	  OSYSDEBUG(("%s : antError %d\n",
				 "Can't allocate receive buffer",
				 recvBufferMsg.error));
	  return ;
  	}
  antRecvBuffer_ = recvBufferMsg.buffer;
  antRecvBuffer_.Map();
  antRecvData_ = (byte *)(antRecvBuffer_.GetAddress());

  // Create IPStack endpoint

  antEnvCreateEndpointMsg	tcpCreateMsg(EndpointType_TCP,
										 PACKETSIZE * 2);
  tcpCreateMsg.Call(ipstackRef_,
					sizeof (antEnvCreateEndpointMsg));
  if (tcpCreateMsg.error != ANT_SUCCESS)
	{
	  OSYSDEBUG(("%s : antError %d\n",
				 "Can't create endpoint",
				 tcpCreateMsg.error));
	  return ;
	}
  endpoint_ = tcpCreateMsg.moduleRef;
}

//! AiboConnection destructor.

AiboConnection::~AiboConnection() {}

//! Send a "connect" request to the IPStack object.
/*! This function sends a message to the IPStack object to connect
    to the serrver.
*/

UErrorValue
AiboConnection::Connect()
{

  // Connect to the server

  TCPEndpointConnectMsg	tcpConnectMsg(endpoint_,
									  IP_ADDR_ANY,
									  IP_PORT_ANY,
									  host_,
									  port_);
  tcpConnectMsg.Call(ipstackRef_,
					 sizeof (TCPEndpointConnectMsg));
  if (tcpConnectMsg.error != TCP_SUCCESS)
	{
	  OSYSDEBUG(("%s : antError %d\n",
				 "Can't connect to server",
				 tcpConnectMsg.error));
	  return UFAIL;
	}
  Receive();
  return USUCCESS;
}

//! Send a buffer to IPStack, so that it will be send through the connection
/*! This function sends a message to the IPStack object, requesting to send
    a buffer of data. The connection is blocked.

    As soon as the connection is ready to send another packet, the 
    CLIENT::TCPSendCont() callback will be trigerred and the connection will be
    unblocked by calling UServer::continueSend().
*/

UErrorValue
AiboConnection::effectiveSend(const char *buffer, int len)
{
  // Transfert to IPStack shared buffer
  memcpy(antSendData_, buffer, len);
  
  // Send message to IPSTack
  TCPEndpointSendMsg	tcpSendMsg(endpoint_,
								   antSendData_,
								   len);
  tcpSendMsg.continuation = (void *)this;
  tcpSendMsg.Send(ipstackRef_,
				  ID_,
				  Extra_Entry[entryTCPSendCont],
				  sizeof (TCPEndpointSendMsg));

  // Block sending until the connection is ready again (callback SendCont)
  block();

  return USUCCESS;
}

//! Send a "receive" request to the IPStack object.
/*! This function sends a message to the IPStack object, requesting to start
    receiving data.

    As soon as a new data has arrived, the CLIENT::TCPReceiveCont() callback
    will be trigerred.
*/

UErrorValue
AiboConnection::Receive()
{
  // Send the "Receive" request message

  TCPEndpointReceiveMsg	tcpRecvMsg(endpoint_,
								   antRecvData_,
								   1,
								   AiboConnection::PACKETSIZE);
  tcpRecvMsg.continuation = (void *)this;
  tcpRecvMsg.Send(ipstackRef_,
				  ID_,
				  Extra_Entry[entryTCPReceiveCont],
				  sizeof (TCPEndpointReceiveMsg));

  return USUCCESS;
}

//! Send a "close" request to the IPStack object.
/*! This function sends a message to the IPStack object, requesting to close the
    connection.

    As soon as the IPStack internal closing is done, the CLIENT::TCPCloseCont() 
    callback will be trigerred.
*/

UErrorValue
AiboConnection::Close()
{
  // Free the shared buffers
  antSendBuffer_.UnMap();
  antEnvDestroySharedBufferMsg	sendBufferMsg(antSendBuffer_);
  sendBufferMsg.Call(ipstackRef_,
					 sizeof (antEnvDestroySharedBufferMsg));

  antRecvBuffer_.UnMap();
  antEnvDestroySharedBufferMsg	recvBufferMsg(antRecvBuffer_);
  recvBufferMsg.Call(ipstackRef_,
					 sizeof (antEnvDestroySharedBufferMsg));

  // Send the "Close" request message
  TCPEndpointCloseMsg		tcpCloseMsg(endpoint_);
  tcpCloseMsg.continuation = (void *)this;
  tcpCloseMsg.Send(ipstackRef_,
				   ID_,
				   Extra_Entry[entryTCPCloseCont],
				   sizeof (TCPEndpointCloseMsg));
  return USUCCESS;
}

//! Accessor for antReceiveData_ used by CLIENT::ReceiveCont()

byte*
AiboConnection::antRecvData()
{
  return antRecvData_;
}

void 
AiboConnection::continueSend() {
  if (sendBuffer_->dataSize() != 0) {
    debug("unqueuing %d bytes\n", min(PACKETSIZE, sendBuffer_->dataSize()));
    int tosend = min(PACKETSIZE, sendBuffer_->dataSize());
    effectiveSend((char *)sendBuffer_->pop(tosend), tosend);
  }
  else {
    debug("nothing to unqueue\n");
    isSending = false;
  }
}


void 
AiboConnection::send(const char * data, int size) {
  if (!isSending) {
    debug("sending  %d bytes\n",min(size, PACKETSIZE));
    effectiveSend(data, min(size, PACKETSIZE));
    if (size > PACKETSIZE) {
      debug("queuing %d bytes\n", size);
      sendBuffer_->push((byte *)(data+PACKETSIZE), size - PACKETSIZE);
    }
  }
  else {
    debug("queuing %d bytes\n", size);
    sendBuffer_->push((byte *)data, size);
  }
}

