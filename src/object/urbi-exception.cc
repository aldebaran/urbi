/**
 ** \file object/urbi-exception.cc
 ** \brief Implementation of UrbiException
 */

#include "object/urbi-exception.hh"

namespace object
{
  UrbiException::UrbiException (std::string msg)
    : msg_ (msg)
  {}

  UrbiException::UrbiException (std::string primitive, std::string msg)
    : msg_ (primitive + ": " + msg + ".")
  {}

  UrbiException::~UrbiException () throw ()
  {}

  const char*
  UrbiException::what () const throw ()
  {
    return msg_.c_str();
  }
}; // namespace object
