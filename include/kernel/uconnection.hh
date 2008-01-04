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

#ifndef KERNEL_UCONNECTION_HH
# define KERNEL_UCONNECTION_HH

# include <cstdarg>
# include <iomanip>

# include "libport/config.h"
# if ! defined LIBPORT_URBI_ENV_AIBO
#  include <boost/thread.hpp>
# endif
# include <boost/shared_ptr.hpp>

# include "kernel/fwd.hh"
# include "kernel/utypes.hh"
# include "kernel/ucomplaints.hh"

# define ERR_SET(Val) (error_ = Val)
# define CONN_ERR_RET(Val) do			\
  {						\
    ERR_SET(Val);				\
    return *this;				\
  } while (0)

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

class UConnection
{
  friend class UServer;

public:

  /// UConnection constructor.
  /*! This constructor must be called by any sub class.

    Important note on memory management: you should call
    ADDOBJ(nameofsubclass) in the constructor of your UConnection sub
    class to maintain a valid memory occupation
    evaluation. Symmetricaly, you must call FREEOBJ(nameofsubclass) in
    the UConnection destructor. "nameofsubclass" is the name of the
    sub class used (which will be evaluated with a sizeof
    operator). ADDOBJ and FREEOBJ macros are in utypes.h.

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
  UConnection  (UServer *userver,
		int minSendBufferSize,
		int maxSendBufferSize,
		int packetSize,
		int minRecvBufferSize,
		int maxRecvBufferSize);

  virtual ~UConnection ();

  UConnection&         initialize         ();

protected:
  //! UConnection close. Must be redefined by the robot-specific sub class.
  /*! The implementation of this function must set 'closing' to true, to
    tell the UConnection to stop sending data.
  */
  virtual UConnection& closeConnection    () = 0;

#if 1
public:
  static UConnection& block (UConnection& c);
  static UConnection& endl (UConnection& c);
  static UConnection& flush (UConnection& c);
  static UConnection& continueSend (UConnection& c);
  static UConnection& activate (UConnection& c);
  static UConnection& disactivate (UConnection& c);
  static UConnection& close (UConnection& c);

  UConnection& operator<< (UConnection& m (UConnection&));

  /*--------------.
  | Send, sendc.  |
  `--------------*/

  /// Unified struct for sending messages
  struct _Send
  {
    const ubyte* _tag; int _taglen;
    ubyte* _buf; int _buflen;
    bool _flush;
  };

  static inline _Send sendf (const std::string& tag,
			     const char* format, va_list args)
  {
    char buf[1024];
    vsnprintf(buf, sizeof buf - 1, format, args);
    return send (buf, tag.c_str());
  }

  /// Invoke the previous sendf.
  static inline _Send sendf (const std::string& tag,
			     const char* format, ...)
  {
    va_list args;
    va_start(args, format);
    const _Send tmp = sendf (tag, format, args);
    va_end(args);
    return tmp;
  }

  //! Send a string through the connection.
  /*! A tag is automatically added to output the message string and the
    resulting string is sent via send(const ubyte*,int).
    \param s the string to send
    \param tag the tag of the message. Default is "notag"
  */
  static inline _Send send (const char* s, const char* tag)
  {
    return send ((const ubyte*) s, s != 0 ? strlen (s) : 0,
		 (const ubyte*) tag);
  }

  static inline _Send sendc (const char* s, const char* tag)
  {
    return sendc ((const ubyte*) s, s != 0 ? strlen (s) : 0,
		  (const ubyte*) tag);
  }

  static inline _Send sendc (const ubyte* buf, int len,
			     const ubyte* tag = 0)
  {
    return send (buf, len, tag, false);
  }

  static inline
  _Send
  send (const ubyte* buf, int buflen,
	const ubyte* tag = 0, bool flush = true)
  {
    _Send msg;
    msg._tag = tag;
    msg._taglen = 0;
    if (buf)
    {
      msg._buf = new ubyte [buflen];
      memcpy(msg._buf, buf, buflen);
    }
    else
      msg._buf = 0;
    msg._buflen = buflen;
    msg._flush = flush;
    return msg;
  }

  static inline
  _Send
  send (const std::string& s)
  {
    return send (reinterpret_cast<const ubyte*>(s.c_str()), s.length(),
		 0, false);
  }

  UConnection& operator<< (_Send msg);

  /*---------.
  | Prefix.  |
  `---------*/

  struct _Prefix { const char* _tag; };
  static inline _Prefix prefix (const char * tag)
  {
    _Prefix pref;
    pref._tag = tag;
    return pref;
  }
  UConnection& operator<< (_Prefix pref);

  struct _ErrorSignal { UErrorCode _n; };
  static inline _ErrorSignal errorSignal (UErrorCode n)
  {
    _ErrorSignal err;
    err._n = n;
    return err;
  }
  UConnection& operator<< (_ErrorSignal pref);

  struct _ErrorCheck { UErrorCode _n; };
  static inline _ErrorCheck errorCheck (UErrorCode n)
  {
    _ErrorCheck err;
    err._n = n;
    return err;
  }
  UConnection& operator<< (_ErrorCheck pref);

  struct _Activate { bool _st; };
  static inline _Activate setActivate (bool st)
  {
    _Activate act;
    act._st = st;
    return act;
  }
  UConnection& operator<< (_Activate act);

  struct _SendAdaptative { int _val; };
  static inline _SendAdaptative sendAdaptative (int val)
  {
    _SendAdaptative adap;
    adap._val = val;
    return adap;
  }
  UConnection& operator<< (_SendAdaptative adap);

  struct _RecvAdaptative { int _val; };
  static inline _RecvAdaptative receiveAdaptative (int val)
  {
    _RecvAdaptative adap;
    adap._val = val;
    return adap;
  }
  UConnection& operator<< (_RecvAdaptative adap);

  struct _MsgCode { UMsgType _t; int _n; };
  static inline _MsgCode msg (UMsgType t, int n)
  {
    _MsgCode msg;
    msg._t = t;
    msg._n = n;
    return msg;
  }
  UConnection& operator<< (_MsgCode msg);
  UConnection& operator<< (UErrorCode id);
  UConnection& operator<< (UWarningCode id);

  struct _Execute {
    _Execute (UCommand_TREE*& cmd) : _val (cmd) {}
    UCommand_TREE*& _val;
  };
  static inline _Execute mexecute (UCommand_TREE*& val)
  {
    _Execute cmd (val);
    return cmd;
  }
  UConnection& operator<< (_Execute cmd);

  struct _Append { UCommand_TREE* _val; };
  static inline _Append mappend (UCommand_TREE* val)
  {
    _Append cmd;
    cmd._val = val;
    return cmd;
  }
  UConnection& operator<< (_Append cmd);

  struct _Received { const ubyte* _val; int _len; };
  static inline _Received received (const ubyte* val, int len)
  {
    _Received cmd;
    cmd._val = val;
    cmd._len = len;
    return cmd;
  }
  static inline _Received received (const char* val)
  {
    return received((const ubyte*) val,
		     ((val != 0) ? strlen (val) : 0));
  }
  UConnection& operator<< (_Received cmd);

#endif // 1

protected:
  std::string		mkPrefix	   (const ubyte* tag) const;

  virtual UConnection&	sendc_             (const ubyte *buffer, int length);
  virtual UConnection&	endline            () = 0;

public:
  bool                isBlocked          ();
  UConnection&        block              ();
  UConnection&        continueSend       ();
  UConnection&        flush              ();

protected:
  UConnection&        received_           (const char *s);

  /// \brief Handle an incoming buffer of data.
  ///
  /// Must be called each time a buffer of data is received by the connection.
  /// \param buffer the incoming buffer
  /// \param length the length of the buffer
  /// \return UFAIL       buffer overflow
  /// \return UMEMORYFAIL critical memory overflow
  /// \return USUCCESS    otherwise
  UConnection&        received_          (const ubyte *buffer, int length);

public:
  int                 sendAdaptive       ();
  int                 receiveAdaptive    ();
  UConnection&        setSendAdaptive    (int sendAdaptive);
  UConnection&        setReceiveAdaptive (int receiveAdaptive);

  UConnection&        errorSignal_set    (UErrorCode n);
  UConnection&        errorCheckAndSend  (UErrorCode n);

  UConnection&        activate           ();
  UConnection&        disactivate        ();
  bool                isActive           ();

protected:
  UConnection&        execute            (UCommand_TREE* &execCommand);
  UConnection&        append             (UCommand_TREE *command);

public:
  int                 availableSendQueue ();
  int                 sendQueueRemain    ();

  UCommandQueue&      recvQueue          ();
  UQueue& send_queue();
  UConnection&        localVariableCheck (UVariable *variable);


public:
  //! UConnection IP associated
  /*! The robot specific part should call the function when the
    connection is active and transmit the IP address of the client,
    as a long int.  */
  UConnection& setIP (IPAdd ip);

public:
  /// Error return code for the constructor.
  UErrorValue         uerror_;
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
  UParser& parser (); // OK : accessor

# if ! defined LIBPORT_URBI_ENV_AIBO
  /// Lock access to command tree.
  boost::try_mutex treeMutex;
# endif

private:
  /// Our parser.  A pointer to stop dependencies.
  UParser* parser_;
  /// \}

protected:

  /// Default adaptive behavior for Send/Recv..
  enum { ADAPTIVE = 100 };

  virtual int         effectiveSend     (const ubyte*, int length) = 0;
  UConnection&        send_error        (UErrorCode n);
  UConnection&        send_warning      (UWarningCode n);
  UCommand*           processCommand    (UCommand *&command,
					 URunlevel &rl,
					 bool &mustReturn);

public:
  UErrorValue         error             () const;

protected:
  /// Store error on commands
  UErrorValue	 error_;

private:
  /// Max number of error signals used..
  enum { MAX_ERRORSIGNALS = 20 };

  /// A pointer to stop dependencies.
  UQueue* sendQueue_;

  /// A pointer to stop dependencies.
  UCommandQueue* recvQueue_;

  /// Each call to effectiveSend() will send packetSize byte (or less)..
  int            packetSize_;

  /// Stores the state of the connection.
  bool blocked_;

  /// Whether the connection is receiving binary data.
  bool receiveBinary_;

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

# if ! defined LIBPORT_URBI_ENV_AIBO
  boost::mutex mutex_;
# endif
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

//! Accessor for error_
inline UErrorValue
UConnection::error() const
{
  return error_;
}


inline
UParser&
UConnection::parser ()
{
  return *parser_;
}

#endif // !KERNEL_UCONNECTION_HH
