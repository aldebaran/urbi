/**
 ** \file object/urbi-exception.cc
 ** \brief Implementation of UrbiException
 */

#include "object/urbi-exception.hh"

namespace object
{
  UrbiException::UrbiException (const std::string& msg)
    : boost::exception ()
  {
    *this << msg_info (msg);
  }

  UrbiException::UrbiException (const std::string& msg,
				const ast::loc& loc)
    : boost::exception ()
  {
    *this << msg_info (msg);
    *this << location_info (loc);
  }

  UrbiException::UrbiException (const std::string& msg,
				const std::string& fun)
    : boost::exception ()
  {
    if (fun.empty ())
      *this << msg_info (msg);
    else
      *this << msg_info (fun + ": " + msg);
    *this << function_info (fun);
  }

  UrbiException::~UrbiException () throw ()
  {
  }

  const char*
  UrbiException::what () const throw ()
  {
    return boost::get_error_info<msg_info> (*this)->c_str();
  }

}; // namespace object
