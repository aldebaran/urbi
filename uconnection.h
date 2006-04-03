/*! \file uconnection.h
 *******************************************************************************

 File: uconnection.h\n
 Definition of the UConnection class.

 This file is part of 
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#ifndef UCONNECTION_H_DEFINED
#define UCONNECTION_H_DEFINED

#include "utypes.h"
#include "userver.h"
#include "uqueue.h"
#include "ucallid.h"
#include "ucommandqueue.h"
#include "uvariable.h"
#include "parser/bison/location.hh"  //FIXME remove this to abstract parser
#include "lockable.h"				     // from connection

class UParser;

/// Pure virtual class for a client connection.
/*! UConnection is holding the message queue in and out. No assumption is made 
    here on the kind of underlying connection (TCP, IPC, OPENR, ...).

    The sending mechanism is asynchronous. Each time the send() function is 
    called, it will pile the data in the internal buffer and try to send what is
    available in the internal buffer by calling continueSend(), except if the
    connection is blocked (see block()).

    The programmer has to call continueSend() each time the system is ready to 
    send something, to make sure the internal buffer is progressively emptied.

    If the connection is not ready for sending, the programmer must call the
    block() function in order to prevent future send() to call continueSend(). 
    Using this feature, it is always possible to use send(), but the data will
    simply be piled on the internal buffer and sent later, when the connection
    is unblocked.

    A call to continueSend() automatically unblocks the connection.

    The received() function must be called each time data has been received on 
    the connection.

    The effectiveSend() function should be overloaded to define the way the 
    system is actually sending data through the real connection.
 */

class UConnection: public Lockable //queue lock
{
  friend class UServer;
  
public:
	
  UConnection  (UServer *userver, 
                int minSendBufferSize,   
                int maxSendBufferSize,
                int packetSize,
                int minRecvBufferSize,
                int maxRecvBufferSize);

  virtual ~UConnection ();

  void                initialize         ();
  virtual UErrorValue closeConnection    () = 0;
		
  UErrorValue         sendPrefix         (const char* tag = 0);
  UErrorValue         send               (const char *s, const char* tag = 0);
  UErrorValue         send               (const ubyte *buffer, int length);
  UErrorValue         endline            ();

  
  bool                isBlocked          ();
  void                block              ();
  UErrorValue         continueSend       ();

  UErrorValue         received           (const char *s);
  UErrorValue         received           (const ubyte *buffer, int length);

  int                 sendAdaptive       ();
  int                 receiveAdaptive    ();

  void                setSendAdaptive    (int sendAdaptive);
  void                setReceiveAdaptive (int receiveAdaptive);

  void                errorSignal        (UErrorCode n);
  void                errorCheck         (UErrorCode n);

  void                activate           ();
  void                disactivate        ();
  bool                isActive           ();                              
  void                execute            (UCommand_TREE* &execCommand);
  void                append             (UCommand_TREE *command);
  int                 availableSendQueue ();
  int                 sendQueueRemain    ();
  UCommandQueue*      recvQueue          ();  
  void                localVariableCheck (UVariable *variable);  
  void                setIP              (IPAdd ip);

  UErrorValue         UError;    ///< error return code for the constructor.    
  UServer             *server;   ///< reference to the underlying server 
  UCommand_TREE       *activeCommand; ///< the command to be executed
  UCommand_TREE       *lastCommand; ///< store the position of the last
                                    ///< command added. 

  UCommand_ASSIGN_BINARY *binCommand; ///< temporarily stores bin command while
                                      ///< binary transfer occurs.
  UString             *connectionTag; ///< virtual device for the connection.
  UString             *functionTag;   ///< virtual device for function def
  UString             *functionClass; ///< class name in a class method definition
  IPAdd               clientIP;      ///< IP of the calling client
  bool                killall;       ///< killall signal (empty Activecommand)
  bool                closing;       ///< connection closing
  bool                receiving;     ///< connection receiving (and processing) commands
  bool                inwork;        ///< connection in the work command
  bool                newDataAdded;  ///< used by addToQueue to notify new data
  bool                returnMode;    ///< true after a "return" command is met
  bool                obstructed;    ///< false when the whole command tree has been processed
  list<UCallid*>      stack;         ///< call ids stack for function calls  

  
  yy::location        lastloc;       ///< last location after parsing

  Lockable            treeLock;      ///< Lock access to command tree
protected:

  static const int ADAPTIVE = 100; ///< Default adaptive behavior for Send/Recv.

  virtual int         effectiveSend     (const ubyte *buffer, int length) = 0;
  UErrorValue         error             (UErrorCode n);
  UErrorValue         warning           (UWarningCode n);
  UCommand*           processCommand    (UCommand *&command,
                                         URunlevel &rl,
                                         bool &mustReturn);

private:
  static const int MAX_ERRORSIGNALS = 20; ///< Max number of error signals used.

  UQueue         *sendQueue_;
  UCommandQueue  *recvQueue_;

  int            packetSize_;    ///< Each call to effectiveSend() will send 
                                 ///< packetSize byte (or less).

  bool           blocked_;       ///< Stores the state of the connection.
  bool           receiveBinary_; ///< True when the connection is receiving
                                 ///< binary data.
  int            transferedBinary_; ///< nb of bytes already received in bin mode
  int            sendAdaptive_;  ///< Adaptive behavior for the send UQueue.
  int            recvAdaptive_;  ///< Adaptive behavior for the send UQueue.
  bool           errorSignals_[MAX_ERRORSIGNALS];///< stores error flags
  bool           active_;        ///< true when the connection is reading to
                                 ///< send/receive data (usualy set at "true"
                                 ///< on start).
};

//! Accessor for sendAdaptive_
inline int
UConnection::sendAdaptive() 
{
  return sendAdaptive_;
}

//! Accessor for recvQueue_
inline UCommandQueue*
UConnection::recvQueue() 
{
  return recvQueue_;
}

//! Accessor for receiveAdaptive_
inline int
UConnection::receiveAdaptive() 
{  
return recvAdaptive_;
}

//! Sets sendAdaptive_
inline void                
UConnection::setSendAdaptive (int sendAdaptive) 
{
  sendAdaptive_ = sendAdaptive;
}

//! Sets receiveAdaptive_
inline void                
UConnection::setReceiveAdaptive (int receiveAdaptive) 
{
  recvAdaptive_ = receiveAdaptive;
}

#endif
