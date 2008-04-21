/// \file uconnection.hxx

#ifndef KERNEL_UCONNECTION_HXX
# define KERNEL_UCONNECTION_HXX

# include "kernel/uconnection.hh"
# include <sstream>

  /*----------.
  | Accessors |
  `----------*/

inline
UQueue&
UConnection::recv_queue_get ()
{
  return *recv_queue_;
}

inline
UQueue&
UConnection::send_queue_get ()
{
  return *send_queue_;
}

inline
parser::UParser&
UConnection::parser_get ()
{
  return *parser_;
}

inline
UErrorValue
UConnection::error_get () const
{
  return error_;
}

inline
UServer&
UConnection::server_get() const
{
  return *server_;
}

inline
bool&
UConnection::active_get()
{
  return active_;
}

inline
IPAdd&
UConnection::client_ip_get ()
{
  return client_ip_;
}

inline
bool&
UConnection::blocked_get ()
{
  return blocked_ ;
}

inline
object::rLobby
UConnection::lobby_get()
{
  return lobby_;
}

inline
bool&
UConnection::new_data_added_get()
{
  return new_data_added_;
}

inline
bool&
UConnection::closing_get()
{
  return closing_;
}

  /*------------------.
  | Utility fonctions |
  `------------------*/

inline
void
UConnection::error_signal_set (UErrorCode n)
{
  error_signals_[(int)n] = true;
}

/// Check if the error_signal is active and tries to effectively send the message
/// If the message can be sent, the error_signal is canceled, otherwise not.
inline
void
UConnection::error_check_and_send (UErrorCode n)
{
  if (error_signals_[(int)n] && (send(n).error_get () == USUCCESS))
    error_signals_[(int)n] = false;
}

inline
void
UConnection::flush ()
{
  if (!blocked_)
    continue_send();
}

template <typename T>
UConnection&
UConnection::operator<<(const T& t)
{
  std::ostringstream os;
  os << t;
  send_queue(os.str().c_str(), os.str().length());
  return *this;
}

inline
UConnection&
UConnection::operator<<(std::ostream& (*pf)(std::ostream&))
{
  if (pf == static_cast<std::ostream& (*)(std::ostream&)>(std::endl))
    endline();
  return *this;
}

inline
UConnection&
UConnection::send (const char* buf, const char* tag, bool flush)
{
  return send(buf, strlen(buf), tag, flush);
}

#endif // !KERNEL_UCONNECTION_HXX
