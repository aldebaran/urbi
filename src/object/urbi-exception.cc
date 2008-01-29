/**
 ** \file object/urbi-exception.cc
 ** \brief Implementation of UrbiException
 */

#include "object/urbi-exception.hh"

namespace object
{
  UrbiException::UrbiException (const std::string& msg)
    : msg_ (msg),
      loc_ (),
      location_set_(false)
  {
    initialize_msg ();
  }

  UrbiException::UrbiException (const std::string& msg,
				const ast::loc& loc)
    : msg_ (msg),
      loc_ (loc),
      location_set_(false)
  {
    initialize_msg ();
  }

  UrbiException::UrbiException (const std::string& msg,
				const std::string& fun)
    : msg_ (msg),
      loc_ (),
      fun_ (fun),
      location_set_(false)
  {
    initialize_msg ();
  }

  UrbiException::~UrbiException () throw ()
  {
  }

  void
  UrbiException::initialize_msg () throw ()
  {
    msg_ = fun_ + (fun_.empty() ? "" : ": ") + msg_;
  }

  const char*
  UrbiException::what () const throw ()
  {
    return msg_.c_str();
  }

}; // namespace object
