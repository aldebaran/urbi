/// \file uconnection.hxx

#ifndef KERNEL_UCONNECTION_HXX
# define KERNEL_UCONNECTION_HXX

# include "kernel/uconnection.hh"

inline
UConnection&
UConnection::sendc_ (const std::string& s)
{
  return sendc_ (reinterpret_cast<const ubyte*> (s.c_str ()), s.size ());
}

//! Accessor for sendAdaptive_
inline
int
UConnection::sendAdaptive ()
{
  return sendAdaptive_;
}

//! Accessor for recvQueue_
inline
UCommandQueue&
UConnection::recvQueue ()
{
  return *recvQueue_;
}

//! Accessor for sendQueue_
inline
UQueue&
UConnection::send_queue ()
{
  return *sendQueue_;
}

//! Accessor for receiveAdaptive_
inline
int
UConnection::receiveAdaptive ()
{
  return recvAdaptive_;
}

inline
UParser&
UConnection::parser ()
{
  return *parser_;
}

inline
UErrorValue
UConnection::error () const
{
  return error_;
}

#endif // !KERNEL_UCONNECTION_HXX
