/*! \file uconnection.hh
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

#ifndef UCONNECTION_HH
# define UCONNECTION_HH

# include "libport/lockable.hh"
# include "fwd.hh"
# include "utypes.hh"
# include "parser/uparser.hh"
# include "uqueue.hh"
# include "ucommandqueue.hh"

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

class UConnection: public urbi::Lockable //queue lock
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
  virtual UErrorValue send               (const ubyte *buffer, int length);
  UErrorValue         sendc              (const char *s, const char* tag = 0);
  virtual UErrorValue sendc              (const ubyte *buffer, int length);
  UErrorValue         endline            ();


  bool                isBlocked          ();
  void                block              ();
  UErrorValue         continueSend       ();
  void                flush              ();

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
  UCommandQueue&      recvQueue          ();
  void                localVariableCheck (UVariable *variable);
  void                setIP              (IPAdd ip);

  /// Error return code for the constructor..
  UErrorValue         UError;
  /// Reference to the underlying server.
  UServer             *server;
  /// The command to be executed.
  UCommand_TREE       *activeCommand;
  /// Store the position of the last command added.
  UCommand_TREE       *lastCommand;


  /// Temporarily stores bin command while binary transfer occurs.
  UCommand_ASSIGN_BINARY *binCommand;

  /// Virtual device for the connection..
  UString             *connectionTag;
  /// Virtual device for function def.
  UString             *functionTag;
  /// Class name in a class method definition.
  UString             *functionClass;
  /// Ip of the calling client.
  IPAdd               clientIP;
  /// Killall signal (empty Activecommand).
  bool                killall;
  /// Connection closing.
  bool                closing;
  /// Connection receiving (and processing) commands.
  bool                receiving;
  /// Connection in the work command.
  bool                inwork;
  /// Used by addToQueue to notify new data.
  bool                newDataAdded;
  /// True after a "return" command is met.
  bool                returnMode;
  /// False when the whole command tree has been processed.
  bool                obstructed;
  /// Call ids stack for function calls.
  std::list<UCallid*>      stack;


  /// \name Parsing.
  /// \{
public:
  /// Return the UParser we use.
  UParser& parser ();

  /// Lock access to command tree.
  urbi::Lockable treeLock;

private:
  /// The parser object.
  UParser parser_;
  /// \}

protected:

  /// Default adaptive behavior for Send/Recv..
  static const int ADAPTIVE = 100;

  virtual int         effectiveSend     (const ubyte*, int length) = 0;
  UErrorValue         error             (UErrorCode n);
  UErrorValue         warning           (UWarningCode n);
  UCommand*           processCommand    (UCommand *&command,
					 URunlevel &rl,
					 bool &mustReturn);

private:
  /// Max number of error signals used..
  static const int MAX_ERRORSIGNALS = 20;

  UQueue         sendQueue_;
  UCommandQueue  recvQueue_;

  /// Each call to effectiveSend() will send packetSize byte (or less)..
  int            packetSize_;

  /// Stores the state of the connection..
  bool           blocked_;
  /// True when the connection is receiving binary data.
  bool           receiveBinary_;

  /// Nb of bytes already received in bin mode.
  int            transferedBinary_;
  /// Adaptive behavior for the send UQueue.
  int sendAdaptive_;
  /// Adaptive behavior for the receiving UQueue.
  int recvAdaptive_;
  /// Stores error flags.
  bool           errorSignals_[MAX_ERRORSIGNALS];
  /// True when the connection is reading to send/receive data (usualy
  /// set at "true" on start).
  bool           active_;
};

//! Accessor for sendAdaptive_
inline int
UConnection::sendAdaptive()
{
  return sendAdaptive_;
}

//! Accessor for recvQueue_
inline UCommandQueue&
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

inline
UParser&
UConnection::parser ()
{
  return parser_;
}

#endif
