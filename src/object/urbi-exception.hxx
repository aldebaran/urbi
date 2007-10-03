/**
 ** \file object/urbi-exception.hxx
 ** \brief Implementation of UrbiException
 */

#ifndef OBJECT_URBI_EXCEPTION_HXX
# define OBJECT_URBI_EXCEPTION_HXX
# include <boost/format.hpp>

namespace object
{

  inline
  const ast::loc&
  UrbiException::location_get () const
  {
    return loc_;
  }

  inline
  void
  UrbiException::location_set (const ast::loc& l)
  {
    loc_ = l;
  }

  inline
  LookupError::LookupError (libport::Symbol slot)
    : UrbiException ((boost::format ("lookup failed: %1%")
                      % slot.name_get()).str ())
  {
  }

  inline
  RedefinitionError::RedefinitionError (libport::Symbol slot)
    : UrbiException ((boost::format ("slot redefinition: %1%")
                      % slot.name_get()).str ())
  {
  }

  inline
  PrimitiveError::PrimitiveError (std::string primitive,
                                  std::string msg)
    : UrbiException (primitive + ": " + msg)
  {
  }

  inline
  WrongArgumentType::WrongArgumentType (Object::kind_type real,
                                        Object::kind_type expected)
    : UrbiException (std::string ("unexpected argument type `")
                     + Object::string_of (real) + "', expected `"
                     + Object::string_of (expected) + '\'')
  {
  }

  inline
  WrongArgumentCount::WrongArgumentCount (unsigned argReal,
                                          unsigned argExpected)
    : UrbiException ((boost::format ("expected %1% arguments, given %2%")
                      % argReal
                      % argExpected).str ())
  {
  }

  inline
  void
  check_arg_count (unsigned formal, unsigned effective)
  {
    if (formal != effective)
      throw WrongArgumentCount(formal, effective);
  }

}; // end of namespace object

#endif //! OBJECT_URBI_EXCEPTION_HXX
