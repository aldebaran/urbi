/**
 ** \file object/urbi-exception.cc
 ** \brief Implementation of UrbiException
 */

#include "object/urbi-exception.hh"

namespace object
{
  /// Error messages definition.
  /// \{
  const char UrbiException::lookupFailed_[] =
  "lookup failed: %1%.";
  const char UrbiException::primitiveError_[] =
  "%1%: %2%.";
  const char UrbiException::wrongArgumentType_[] =
  "Argument type is ``%1%'', but ``%2%'' was expected.";
  const char UrbiException::wrongArgumentCount_[] =
  "expected %1% arguments, given %2%.";
  /// \}

  UrbiException::UrbiException (std::string msg)
    : msg_ (msg),
      loc_ ()
  {}

  UrbiException::UrbiException (std::string msg, const ast::loc& loc)
    : msg_ (msg),
      loc_ (loc)
  {}

  UrbiException::~UrbiException () throw ()
  {}

  const char*
  UrbiException::what () const throw ()
  {
    return msg_.c_str();
  }

  const ast::loc&
  UrbiException::location_get () const
  {
    return loc_;
  }

  void
  UrbiException::location_set (const ast::loc& l)
  {
    loc_ = l;
  }

}; // namespace object
