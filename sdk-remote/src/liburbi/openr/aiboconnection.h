/*! \file aiboconnection.h
 */

#ifndef AIBOCONNECTION_H
# define AIBOCONNECTION_H

# include "entry.h"
#include "uqueue.h"
# include <OPENR/OSyslog.h>
# include <EndpointTypes.h>
# include <TCPEndpointMsg.h>
# include <IPAddress.h>
# include <ant.h>
class UClient;

/// AiboConnection implements an TCP/IP client connection using OPENR.
/*! AiboConnection uses the asynchronous message sending method of OPENR
    Once a buffer has been sent to the IPStack object, one should wait
    for the callback stating that the system is reading to send a new
    string. This asynchronous mechanism is handled by UConnection using
    its blocking feature (see UConnection::block());
*/

class	AiboConnection
{
 public:
  AiboConnection::AiboConnection(antStackRef	ipstackRef,
								 OID			ID,
								 unsigned int	port,
								 const char		*host,
								 UClient		*ref);
  ~AiboConnection();
  UErrorValue   	Connect();
  UErrorValue		Receive();
  UErrorValue		Close();

  bool			isSending;

 private:

  // Parameters used by the constructor.
  
  static const int MINBUFFERSIZE;// = 4096;
  static const int MAXBUFFERSIZE;// = 128000;
  static const int PACKETSIZE;   // = 65536;

  antStackRef		ipstackRef_;
  OID				ID_;
  antModuleRef		endpoint_;

  // OPENR Send buffer data

  antSharedBuffer	antSendBuffer_;
  byte				*antSendData_;

  // OPENR Receive buffer data

  antSharedBuffer	antRecvBuffer_;
  byte				*antRecvData_;

 protected:
  virtual UErrorValue   effectiveSend(const char *buffer, int length);
  void block() {isSending = true;}
  void unBlock() {isSending = false;}
  UClient * ref_;
  UQueue  * sendBuffer_;
  char * host_;
  int port_;
 public:
  byte				*antRecvData();
  // called when sending is possible again
  void continueSend();
  //request to send from the client
  void  send(const char * buffer, int size);
  UClient * getClient() {return ref_;}
};

#endif /* !AIBOCONNECTION_H */
