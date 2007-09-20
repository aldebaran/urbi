/**
 ** \file object/urbi-exception.hxx
 ** \brief Implementation of UrbiException
 */

#ifndef OBJECT_URBI_EXCEPTION_HXX
# define OBJECT_URBI_EXCEPTION_HXX
# include <boost/format.hpp>

namespace object
{
  inline UrbiException
  UrbiException::lookupFailed (std::string slot)
  {
    return UrbiException ((boost::format (lookupFailed_)
			   % slot).str ());
  }

  inline UrbiException
  UrbiException::primitiveError (std::string primitive,
				 std::string msg)
  {
    return UrbiException ((boost::format (primitiveError_)
			   % primitive
			   % msg).str ());
  }

  inline UrbiException
  UrbiException::wrongArgumentType (Object::kind_type real,
				    Object::kind_type expected)
  {
    return UrbiException ((boost::format (wrongArgumentType_)
			   % Object::string_of (real)
			   % Object::string_of (expected)).str ());
  }

  inline UrbiException
  UrbiException::wrongArgumentCount (unsigned argReal,
				     unsigned argExpected)
  {
    return UrbiException ((boost::format (wrongArgumentCount_)
			   % argReal
			   % argExpected).str ());
  }

}; // end of namespace object

#endif //! OBJECT_URBI_EXCEPTION_HXX
