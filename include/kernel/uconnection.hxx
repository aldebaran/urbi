/// \file kernel/uconnection.hxx

#ifndef KERNEL_UCONNECTION_HXX
# define KERNEL_UCONNECTION_HXX

# include <sstream>
# include <kernel/uconnection.hh>

namespace kernel
{
    /*----------.
    | Accessors |
    `----------*/

  inline
  UQueue&
  UConnection::recv_queue_get()
  {
    return *recv_queue_;
  }

  inline
  UQueue&
  UConnection::send_queue_get()
  {
    return *send_queue_;
  }

  inline
  parser::UParser&
  UConnection::parser_get()
  {
    return *parser_;
  }

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
  object::rLobby&
  UConnection::lobby_get()
  {
    return lobby_;
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

}

#endif // !KERNEL_UCONNECTION_HXX
