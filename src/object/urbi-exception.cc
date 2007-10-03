/**
 ** \file object/urbi-exception.cc
 ** \brief Implementation of UrbiException
 */

#include "object/urbi-exception.hh"

namespace object
{
  UrbiException::UrbiException (std::string msg)
    : msg_ (msg),
      loc_ ()
  {
  }

  UrbiException::UrbiException (std::string msg, const ast::loc& loc)
    : msg_ (msg),
      loc_ (loc)
  {
  }

  UrbiException::~UrbiException () throw ()
  {
  }

  const char*
  UrbiException::what () const throw ()
  {
    return msg_.c_str();
  }

}; // namespace object
