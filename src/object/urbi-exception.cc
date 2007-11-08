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
    initialize_msg ();
  }

  UrbiException::UrbiException (const std::string& msg,
				const ast::loc& loc)
    : msg_ (msg),
      loc_ (loc)
  {
    initialize_msg ();
  }

  UrbiException::UrbiException (const std::string& msg,
				const std::string& fun)
    : msg_ (msg),
      loc_ (),
      fun_ (fun)
  {
    initialize_msg ();
  }

  UrbiException::~UrbiException () throw ()
  {
  }

  void
  UrbiException::initialize_msg () throw ()
  {
    if (getenv("DEBUG") && !fun_.empty ())
      msg_ = fun_ + ": " + msg_;
  }

  const char*
  UrbiException::what () const throw ()
  {
    return msg_.c_str();
  }

}; // namespace object
