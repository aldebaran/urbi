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
  UrbiException
  UrbiException::lookupFailed (libport::Symbol slot)
  {
    return UrbiException ((boost::format ("lookup failed: %1%")
			   % slot.name_get()).str ());
  }

  inline
  UrbiException
  UrbiException::redefinition (libport::Symbol slot)
  {
    return UrbiException ((boost::format ("slot redefinition: %1%")
			   % slot.name_get()).str ());
  }

  inline
  UrbiException
  UrbiException::primitiveError (std::string primitive,
				 std::string msg)
  {
    return UrbiException ((boost::format ("%1%: %2%")
			   % primitive
			   % msg).str ());
  }

  inline
  UrbiException
  UrbiException::wrongArgumentType (Object::kind_type real,
				    Object::kind_type expected)
  {
    return UrbiException
      ((boost::format ("unexpected argument type ``%1%'', expected ``%2%''")
	% Object::string_of (real)
	% Object::string_of (expected)).str ());
  }

  inline
  UrbiException
  UrbiException::wrongArgumentCount (unsigned argReal,
				     unsigned argExpected)
  {
    return UrbiException ((boost::format ("expected %1% arguments, given %2%")
			   % argReal
			   % argExpected).str ());
  }

  inline
  void
  check_arg_count (unsigned formal, unsigned effective)
  {
    if (formal != effective)
      throw UrbiException::wrongArgumentCount(formal, effective);
  }

}; // end of namespace object

#endif //! OBJECT_URBI_EXCEPTION_HXX
