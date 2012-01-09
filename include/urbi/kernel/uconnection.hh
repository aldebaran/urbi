/*
 * Copyright (C) 2005-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/kernel/uconnection.hh
/// \brief Declaration of UConnection.

#ifndef KERNEL_UCONNECTION_HH
# define KERNEL_UCONNECTION_HH

# include <libport/cstring>
# include <iomanip>
# include <memory>

# include <libport/config.h>

# if LIBPORT_HAVE_WINDOWS_H
// Without this, windows.h may include winsock.h, which will conflict with
// winsock2.h when we will try to include it.
#  define WIN32_LEAN_AND_MEAN
# endif

# include <urbi/kernel/fwd.hh>
# include <urbi/export.hh>
# include <urbi/iostream.hh>
# include <urbi/kernel/utypes.hh>

namespace kernel
{
  /// Pure virtual class for a client connection.

  /*! UConnection is holding the message queue in and out. No
     assumption is made here on the kind of underlying connection
     (TCP, IPC, ...).

     The sending mechanism is asynchronous. Each time the send()
     function is called, it will pile the data in the internal buffer
     and try to send what is available in the internal buffer by
     calling continue_send(), except if the connection is blocked (see
     block()).

     The programmer has to call continue_send() each time the system
     is ready to send something, to make sure the internal buffer is
     progressively emptied.

     If the connection is not ready for sending, the programmer must
     call the block() function in order to prevent future send() to
     call continue_send().  Using this feature, it is always possible
     to use send(), but the data will simply be piled on the internal
     buffer and sent later, when the connection is unblocked.

     A call to continue_send() automatically unblocks the
     connection.

     The received() function must be called each time data has been
     received on the connection.

     The effective_send() function should be overloaded to define
     the way the system is actually sending data through the real
     connection.  */

  class URBI_SDK_API UConnection
  {
    friend class UServer;

  public:
    /// UConnection constructor.
    /*! This constructor must be called by any sub class.

      \param userver is the server to which the connection belongs

      \param packetSize is the maximal size of a packet sent in one shot
      via a call to effective_send(). This should be the biggest as
      possible to help emptying the sending queue as fast as
      possible. Check the capacity of your connection to know this
      limit.

      When exiting, UError can have the following values: - USUCCESS:
      success - UFAIL : memory allocation failed. */
    UConnection(UServer& userver, size_t packetSize);
    virtual ~UConnection();

    /// \brief Set up urbiscript support.
    ///
    /// This function must be called once the connection is
    /// operational, able to send data and to process urbiscript
    /// programs. It is a requirement for Urbi compliance to send the
    /// header at start, so this function must be called.  It is
    /// expected to load "local.u".
    void initialize();

    /*-------------------------.
    | Send/receive functions.  |
    `-------------------------*/

    /// The "base" high-level send function. Calls the send_queue() function.
    void
    send(const char* buf, size_t len, const char* tag = 0, bool flush = true);

    /// Overload using 'strlen' to compute buf size.
    void
    send(const char* buf, const char* tag = 0, bool flush = true);

    /// Overload for C++.
    void send(const std::string& s, const char* tag = 0, bool flush = true);

    //! Send at most packetSize bytes in the connection, calling effective_send()

    /*! Must be called when the system tells that the connection is ready to
      accept new data for sending, in order to carry on the processing of the
      sending queue stored in the internal buffer.

      Each call to continue_send sends packetSize bytes (at most) through
      the real connection until the internal buffer is empty.

      \return
      - USUCCESS: successful
      - UFAIL   : effective_send() failed or not enough memory
     */
    void continue_send();

    /// \brief Handle an incoming buffer of data.
    ///
    /// Must be called each time a buffer of data is received by the connection.
    /// \param buffer the incoming buffer
    /// \param length the length of the buffer
    /// \return UFAIL       buffer overflow
    /// \return UMEMORYFAIL critical memory overflow
    /// \return USUCCESS    otherwise
    void received(const char* buffer, size_t length);

    /// Handle an incoming string.
    /*! Must be called each time a string is received by the connection.
     \param s the incoming string
     \return UFAIL buffer overflow
     \return UMEMORYFAIL critical memory overflow
     \return USUCCESS otherwise
     */
    void received(const std::string& s);

  public:
    //! Close the connection.
    void close();

  protected:
    //! UConnection close. Must be redefined by the robot-specific sub class.
    virtual void close_() = 0;

    /// Abstract end of line.
  public:
    virtual void endline() = 0;

    void flush();

    /*------------.
    | Accessors.  |
    `------------*/

    UErrorValue error_get() const;
    UServer& server_get() const;
    urbi::object::rLobby& lobby_get();
    runner::rShell& shell_get();

    bool send_queue_empty() const;

    bool& active_get();
    bool& blocked_get();
    bool& closing_get();

    /*------------------.
    | Utility functions |
    \------------------*/

    bool has_pending_command() const;
    void drop_pending_commands();

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
    \sa send(const char*)
    */
    virtual void send_queue(const char* buffer, size_t length);
    /// Bounce to (const char* buffer, size_t length).
    void send_queue(const std::string& s);

    /// Whether an interactive session.
    bool interactive_p() const;
    void interactive_p(bool b);

    size_t bytes_sent() const;
    size_t bytes_received() const;
  protected:
    //! Sends a buffer through the real connection (redefined in the sub class)
    /*! Must be defined to implement the effective code that sends a
      buffer through the connection.

      ATTENTION: The buffer received is a short lived buffer. There is
      no warranty whatsoever that it will survive once the function
      returns. You must make a copy of it if your sending method
      requires to work asynchronously on the buffer, after the function
      has returned.

      \return  the number of bytes effectively sent.
             -1 upon error.
             FIXME: How can -1 be returned in a size_t? Type is wrong.
     */
    virtual size_t effective_send(const char*, size_t length) = 0;

  public:
    /// Error return code for the constructor.
    UErrorValue uerror_;

    /// Connection closing.
    bool closing_;

    /// Connection receiving (and processing) commands.
    bool receiving_;

  protected:
    /// Reference to the underlying server.
    UServer& server_;

    /// Store error on commands.
    UErrorValue error_;

    /// The Lobby into which the code is evaluated.
    urbi::object::rLobby lobby_;

  protected:
    typedef libport::Fifo<char, '\0'> queue_type;
    std::auto_ptr<queue_type> send_queue_;

    /// Each call to effective_send() will send packetSize byte (or less).
    size_t packet_size_;

    /// The state of the connection.
    bool blocked_;

    /// True when the connection is reading to send/receive data (usualy
    /// set at "true" on start).
    bool active_;

    /// The current shell.
    runner::rShell shell_;

    /// Whether this connection is interactive.
    bool interactive_p_;

    /// Number of bytes sent so far.
    size_t bytes_sent_;
    /// Number of bytes received so far.
    size_t bytes_received_;

  private:
    /// The received data stream.
    urbi::StreamBuffer stream_buffer_;
    std::istream stream_;
  };

}

# include <urbi/kernel/uconnection.hxx>

#endif // !KERNEL_UCONNECTION_HH
