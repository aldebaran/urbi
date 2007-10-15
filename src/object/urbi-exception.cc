/**
 ** \file object/urbi-exception.cc
 ** \brief Implementation of UrbiException
 */

#include "object/urbi-exception.hh"

namespace object
{
  UrbiException::UrbiException (const std::string& msg)
    : msg_ (msg),
      loc_ ()
  {
  }

  UrbiException::UrbiException (const std::string& msg,
				const ast::loc& loc)
    : msg_ (msg),
      loc_ (loc)
  {
  }

  UrbiException::UrbiException (const std::string& msg,
				const std::string& fun)
    : msg_ (msg),
      loc_ (),
      fun_ (fun)
  {
  }

  UrbiException::~UrbiException () throw ()
  {
  }

  const char*
  UrbiException::what () const throw ()
  {
    std::string msg;
    if (getenv("DEBUG"))
      msg = fun_ + ": ";
    msg += msg_;
    return msg.c_str();
  }

}; // namespace object
