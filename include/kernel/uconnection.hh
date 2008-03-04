/*! \file kernel/uconnection.hh
 *******************************************************************************

 File: uconnection.hh\n
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

# include "ast/fwd.hh"
# include "object/fwd.hh"

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
    available in the internal buffer by calling continue_send(), except if the
    connection is blocked (see block()).

    The programmer has to call continue_send() each time the system is ready to
    send something, to make sure the internal buffer is progressively emptied.

    If the connection is not ready for sending, the programmer must call the
    block() function in order to prevent future send() to call continue_send().
    Using this feature, it is always possible to use send(), but the data will
    simply be piled on the internal buffer and sent later, when the connection
    is unblocked.

    A call to continue_send() automatically unblocks the connection.

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

    \param userver is the server to which the connection belongs

    \param packetSize is the maximal size of a packet sent in one shot
    via a call to effectiveSend(). This should be the biggest as
    possible to help emptying the sending queue as fast as
    possible. Check the capacity of your connection to know this
    limit.

    When exiting, UError can have the following values: - USUCCESS:
    success - UFAIL : memory allocation failed.

    \sa UQueue */
  UConnection (UServer* userver, size_t packetSize);
  virtual ~UConnection ();

  //! Initializes the connection, by sending the standard header for URBI
  /*! This function must be called once the connection is operational and
    able to send data. It is a requirement for URBI compliance to send
    the header at start, so this function must be called.
  */
  UConnection& initialize ();

  /*-----------------------.
  | Send/receive functions |
  \-----------------------*/

  /// The "base" high-level send function. Calls the send_queue() function.
  UConnection&
  send (const char* buf, const char* tag = 0, bool flush = true);

  /// Send Object \a o on \a tag, possibly prefixed by \a p (e.g., "*** ").
  UConnection&
  send (object::rObject result, const char* tag = 0, const char* p = 0);

  //! Send an error message based on the error number.
  /*! This command sends an error message through the connection, and to the
    server output system, according to the error number n.
    \param n the error number.
   */
  UConnection& send (UErrorCode n);

  //! Send a warning message based on the warning number.
  /*! This command sends an warning message through the connection, and to
    the server output system, according to the warning number n.

    \param n the warning number. Use the UWarningCode enum. Can be:
    - 0 : Memory overflow warning

    \param complement is a complement string added at the end
    of the warning message.
  */
  UConnection& send (UWarningCode n);

  //! Send at most packetSize bytes in the connection, calling effectiveSend()
  /*! Must be called when the system tells that the connection is ready to
    accept new data for sending, in order to carry on the processing of the
    sending queue stored in the internal buffer.
    Each call to continue_send sends packetSize bytes (at most) through the real
    connection until the internal buffer is empty.
    \return
    - USUCCESS: successful
    - UFAIL   : effectiveSend() failed or not enough memory
   */
  UConnection& continue_send ();

  /// Notify the connection that a new result is available.  This will
  /// typically print the result on the console or send it through the
  /// network.
  void new_result (object::rObject result);

  /// Handle an incoming string.
  /*! Must be called each time a string is received by the connection.
   \param s the incoming string
   \return UFAIL buffer overflow
   \return UMEMORYFAIL critical memory overflow
   \return USUCCESS otherwise
   */
  UConnection& received (const char *s);

  /// \brief Handle an incoming buffer of data.
  ///
  /// Must be called each time a buffer of data is received by the connection.
  /// \param buffer the incoming buffer
  /// \param length the length of the buffer
  /// \return UFAIL       buffer overflow
  /// \return UMEMORYFAIL critical memory overflow
  /// \return USUCCESS    otherwise
  UConnection& received (const ubyte *buffer, int length);

  /// A generic << operator, to easily send every kind of data through the
  ///connection.
  template <typename T>
  UConnection& operator<<(const T&);

  /// A << operator which call endline() when receiving std::endl.
  UConnection& operator<<(std::ostream& (*pf)(std::ostream&));

  //! UConnection close. Must be redefined by the robot-specific sub class.
  /*! The implementation of this function must set 'closing_' to true, to
    tell the UConnection to stop sending data.
  */
  virtual UConnection& close () = 0;

  /// Abstract end of line.
  virtual UConnection&	endline() = 0;

  void flush ();

  /*----------.
  | Accessors |
  \----------*/

  UErrorValue error_get () const;
  UParser& parser_get ();
  UServer& server_get() const;
  object::rLobby lobby_get();

  //! UConnection IP associated
  /*! The robot specific part should call the function when the
    connection is active and transmit the IP address of the client,
    as a long int.  */
  IPAdd& client_ip_get ();

  bool send_queue_empty () const;

  UQueue& recv_queue_get ();
  UQueue& send_queue_get ();

  bool& active_get ();
  bool& blocked_get ();
  bool& closing_get ();

  bool& new_data_added_get();

  /*------------------.
  | Utility functions |
  \------------------*/

  bool has_pending_command () const;
  void drop_pending_commands ();

  /// Build a prefix [01234567:tag].
  std::string make_prefix (const char* tag) const;

  void error_signal_set (UErrorCode n);
  void error_check_and_send (UErrorCode n);

protected:
  enum { ADAPTIVE = 100 };

  //! Send a buffer through the connection without flushing it.
  /*! The function piles the buffer in the sending queue and calls
   continue_send() if the connection is not blocked (blocked means that
   the connection is not ready to send data). The server will try to send
   the data in the sending queue each time the "work" function is called
   and if the connection is not blocked.

   It is the job of the programmer to let the kernel know when
   the connection is blocked or not, using the "block()" function to block it
   or by calling continue_send() directly to unblock it.

  \param buffer the buffer to send
  \param length the length of the buffer
  \return
  - USUCCESS: successful. The message is in the queue.
  - UFAIL   : could not send the buffer, not enough memory in the
  send queue.
  \sa send(const char*)
  */
  virtual UConnection&	send_queue(const ubyte *buffer, int length);

  //! Sends a buffer through the real connection (redefined in the sub class)
  /*! Must be defined to implement the effective code that sends a buffer through
    the connection.

    ATTENTION: The buffer received is a short lived buffer. There is no
    warranty whatsoever that it will survive once the function returns. You must
    make a copy of it if your sending method requires to work asynchronously on
    the buffer, after the function has returned.

    \return the number of bytes effectively sent. -1 means that there was an error.
   */
  virtual int effective_send (const ubyte*, int length) = 0;

  UConnection& execute ();

public:
  /// Error return code for the constructor.
  UErrorValue uerror_;

  /// Virtual device for the connection..
  std::string connection_tag_;

  /// Ip of the calling client.
  IPAdd client_ip_;

  /// Connection closing.
  bool closing_;

  /// Connection receiving (and processing) commands.
  bool receiving_;

  /// Used by addToQueue to notify new data.
  bool new_data_added_;

# if ! defined LIBPORT_URBI_ENV_AIBO
  /// Lock access to command tree.
  boost::try_mutex tree_mutex_;
# endif

protected:
  /// Store error on commands
  UErrorValue error_;

private:
  /// Max number of error signals used..
  enum { MAX_ERRORSIGNALS = 20 };

  /// A pointer to stop dependencies.
  UQueue* send_queue_;

  /// A pointer to stop dependencies.
  UQueue* recv_queue_;

  /// Each call to effectiveSend() will send packetSize byte (or less)..
  int packet_size_;

  /// Stores the state of the connection.
  bool blocked_;

  /// Stores error flags.
  bool error_signals_[MAX_ERRORSIGNALS];

  /// True when the connection is reading to send/receive data (usualy
  /// set at "true" on start).
  bool active_;

  /// The Lobby into which the code is evaluated.
  object::rLobby lobby_;

  /// Our parser.  A pointer to stop dependencies.
  UParser* parser_;

  /// The commands to be executed.
  ast::Nary* active_command_;

  /// Reference to the underlying server.
  UServer* server_;

# if ! defined LIBPORT_URBI_ENV_AIBO
  boost::mutex mutex_;
# endif

};

# include "kernel/uconnection.hxx"
#endif // !KERNEL_UCONNECTION_HH
