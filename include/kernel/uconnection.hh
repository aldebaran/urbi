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

#include <cstdarg>

# include "libport/lockable.hh"

# include "kernel/fwd.hh"
# include "kernel/utypes.hh"
# include "kernel/ucomplaints.hh"

# include "ast/fwd.hh"
# include "object/fwd.hh"

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

class UConnection: public libport::Lockable //queue lock
{
  friend class UServer;

public:

  /// UConnection constructor.
  /*! This constructor must be called by any sub class.

    \param userver is the server to which the connection belongs

    \param minSendBufferSize UConnection uses an adaptive dynamic
    queue (UQueue) to buffer sent data. This parameter sets the
    minimal and initial size of the queue. Suggested value is 4096.

    \param maxSendBufferSize The internal sending UQueue size can grow
    up to maxSendBufferSize.  A good strategy is to have here twice
    the size of the biggest data that could enter the queue, plus a
    small 10% overhead. Typically, the size of the biggest image times
    two + 10%.  Zero means "illimited" and is not advised if one wants
    to control the connection's size.

    \param packetSize is the maximal size of a packet sent in one shot
    via a call to effectiveSend(). This should be the biggest as
    possible to help emptying the sending queue as fast as
    possible. Check the capacity of your connection to know this
    limit.

    \param minRecvBufferSize UConnection uses an adaptive dynamic
    queue (UQueue) to buffer received data. This parameter sets the
    minimal and initial size of the queue. Suggested value is 4096.

    \param maxRecvBufferSize The internal receiving UQueue size can
    grow up to maxRecvBufferSize.A good strategy is to have here twice
    the size of the biggest data that could enter the queue, plus a
    small 10% overhead. Zero means "illimited". Note that binary data
    are handled on specific buffers and are not part of the receiving
    queue. The "biggest size" here means the "biggest ascii
    command". For URBI commands, a good choice is 65536.

    Note that all UQueues are, by default, adaptive queues (see the
    UQueue documentation for more details). This default kernel
    behavior can be modified by changing the UConnection::ADAPTIVE
    constant or, individually by using the setSendAdaptive(int) and
    setReceiveAdaptive(int) method.

    When exiting, UError can have the following values: - USUCCESS:
    success - UFAIL : memory allocation failed.

    \sa UQueue */
  UConnection (UServer* userver,
               int minSendBufferSize,
               int maxSendBufferSize,
               int packetSize,
               int minRecvBufferSize,
               int maxRecvBufferSize);

  virtual ~UConnection ();

  void initialize ();
  virtual UErrorValue closeConnection () = 0;

  UErrorValue sendPrefix (const char* tag = 0);
  UErrorValue send (const char *s, const char* tag = 0);
  virtual UErrorValue send (const ubyte *buffer, int length);

  UErrorValue sendf (const std::string& tag, const char* format, va_list args);
  UErrorValue sendf (const std::string& tag, const char* format, ...);

  UErrorValue sendc (const char *s, const char* tag = 0);
  virtual UErrorValue sendc (const ubyte *buffer, int length);
  UErrorValue endline ();


  bool isBlocked ();
  void block ();
  UErrorValue continueSend ();
  void flush ();

  UErrorValue received (const char *s);

  /// \brief Handle an incoming buffer of data.
  ///
  /// Must be called each time a buffer of data is received by the connection.
  /// \param buffer the incoming buffer
  /// \param length the length of the buffer
  /// \return UFAIL       buffer overflow
  /// \return UMEMORYFAIL critical memory overflow
  /// \return USUCCESS    otherwise
  UErrorValue received (const ubyte *buffer, int length);

  int sendAdaptive ();
  int receiveAdaptive ();

  void setSendAdaptive (int sendAdaptive);
  void setReceiveAdaptive (int receiveAdaptive);

  void errorSignal (UErrorCode n);
  void errorCheck (UErrorCode n);

  void activate ();
  void disactivate ();
  bool isActive ();

  /// Execute the pending commands (if any).
  void execute ();

  int availableSendQueue ();
  int sendQueueRemain ();

  UCommandQueue&recvQueue ();
  UQueue& send_queue ();

  void localVariableCheck (UVariable* variable);


  //! UConnection IP associated
  /*! The robot specific part should call the function when the
   connection is active and transmit the IP address of the client,
   as a long int.  */
  void setIP (IPAdd ip);

  bool has_pending_command () const;
  void drop_pending_commands ();

  /// Notify the connection that a new result is available.  This will
  /// typically print the result on the console or send it through the
  /// network.
  void new_result (object::rObject result);

protected:
  /// Error return code for the constructor.
  UErrorValue uerror_;

public:
  /// Reference to the underlying server.
  UServer* server;

private:
  /// The command to be executed (root of the AST).
  ast::BinaryExp* active_command_;

public:
  /// Temporarily stores bin command while binary transfer occurs.
  UCommand_ASSIGN_BINARY* binCommand;

  /// Virtual device for the connection..
  UString* connectionTag;
  /// Virtual device for function def.
  UString* functionTag;
  /// Class name in a class method definition.
  UString* functionClass;
  /// Ip of the calling client.
  IPAdd clientIP;
  /// Killall signal (empty Activecommand).
  bool killall;
  /// Connection closing.
  bool closing;
  /// Connection receiving (and processing) commands.
  bool receiving;
  /// Connection in the work command.
  bool inwork;
  /// Used by addToQueue to notify new data.
  bool newDataAdded;
  /// True after a "return" command is met.
  bool returnMode;
  /// False when the whole command tree has been processed.
  bool obstructed;
  /// Call ids stack for function calls.
  std::list<UCallid*> stack;


  /// \name Parsing.
  /// \{
public:
  /// Return the UParser we use.
  UParser& parser ();

  /// Lock access to command tree.
  libport::Lockable treeLock;

private:
  /// Our parser.  A pointer to stop dependencies.
  UParser* parser_;
  /// \}

protected:

  /// Default adaptive behavior for Send/Recv..
  enum { ADAPTIVE = 100 };

  virtual int effectiveSend (const ubyte*, int length) = 0;
  UErrorValue error (UErrorCode n);
  UErrorValue warning (UWarningCode n);

private:
  /// Max number of error signals used..
  enum { MAX_ERRORSIGNALS = 20 };

  /// A pointer to stop dependencies.
  UQueue* sendQueue_;

  /// A pointer to stop dependencies.
  UCommandQueue* recvQueue_;

  /// Each call to effectiveSend() will send packetSize byte (or less)..
  int packetSize_;

  /// Stores the state of the connection.
  bool blocked_;

  /// Whether the connection is receiving binary data.
  bool receiveBinary_;

  /// Nb of bytes already received in bin mode.
  int transferedBinary_;
  /// Adaptive behavior for the send UQueue.
  int sendAdaptive_;
  /// Adaptive behavior for the receiving UQueue.
  int recvAdaptive_;
  /// Stores error flags.
  bool errorSignals_[MAX_ERRORSIGNALS];
  /// True when the connection is reading to send/receive data (usualy
  /// set at "true" on start).
  bool active_;

  /// The Context into which the code is evaluated.
  /// This is this connection, wrapped into an Urbi object.
  object::rContext context_;
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
  return *recvQueue_;
}

//! Accessor for sendQueue_
inline UQueue&
UConnection::send_queue()
{
  return *sendQueue_;
}

//! Accessor for receiveAdaptive_
inline int
UConnection::receiveAdaptive()
{
  return recvAdaptive_;
}

inline
UParser&
UConnection::parser ()
{
  return *parser_;
}

#endif
