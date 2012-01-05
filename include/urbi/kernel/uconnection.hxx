/*
 * Copyright (C) 2007-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/kernel/uconnection.hxx
/// \brief Inline implementation of kernel::UConnection.

#ifndef KERNEL_UCONNECTION_HXX
# define KERNEL_UCONNECTION_HXX

# include <sstream>
# include <urbi/kernel/uconnection.hh>

namespace kernel
{
    /*----------.
    | Accessors |
    `----------*/

  inline
  UErrorValue
  UConnection::error_get() const
  {
    return error_;
  }

  inline
  UServer&
  UConnection::server_get() const
  {
    return server_;
  }

  inline
  bool&
  UConnection::active_get()
  {
    return active_;
  }

  inline
  bool&
  UConnection::blocked_get()
  {
    return blocked_ ;
  }

  inline
  urbi::object::rLobby&
  UConnection::lobby_get()
  {
    return lobby_;
  }

  inline
  runner::rShell&
  UConnection::shell_get()
  {
    return shell_;
  }

  inline
  bool&
  UConnection::closing_get()
  {
    return closing_;
  }

  inline
  void
  UConnection::flush()
  {
    if (!blocked_)
      continue_send();
  }

  inline
  void
  UConnection::received(const std::string& s)
  {
    received(s.c_str(), s.length());
  }


  inline
  void
  UConnection::send(const char* buf, const char* tag, bool flush)
  {
    send(buf, strlen(buf), tag, flush);
  }

  inline
  void
  UConnection::send(const std::string&s, const char* tag, bool flush)
  {
    send(s.c_str(), s.length(), tag, flush);
  }

  inline
  void
  UConnection::send_queue(const std::string& s)
  {
    send_queue(s.c_str(), s.length());
  }

  inline
  size_t
  UConnection::bytes_sent() const
  {
    return bytes_sent_;
  }

  inline
  size_t
  UConnection::bytes_received() const
  {
    return bytes_received_;
  }
}

#endif // !KERNEL_UCONNECTION_HXX
