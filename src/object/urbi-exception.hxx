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
  PrimitiveError::PrimitiveError (const std::string& primitive,
				  const std::string& msg)
    : UrbiException (msg, primitive)
  {
  }

  inline
  StackExhaustedError::StackExhaustedError (const std::string& msg)
    : UrbiException (msg)
  {
  }

  inline
  WrongArgumentType::WrongArgumentType (object_kind_type formal,
					object_kind_type effective,
					const std::string& fun)
    : UrbiException (std::string ("unexpected argument type `")
		     + string_of(effective) + "', expected `"
		     + string_of(formal) + '\'',
		     fun)
  {
  }

  inline
  WrongArgumentType::WrongArgumentType (const std::string& fun)
    : UrbiException (std::string ("unexpected void"), fun)
  {
  }

  // In WrongArgumentCount constructors, we subtract one to all number
  // of arguments, provided or expected. This is because we expect the
  // target to be included when checking the number of arguments because
  // it seems more natural to do so, as this looks like the argc check
  // in main(). However, it is more user-friendly to tell the user about
  // the number of real arguments, not including the target, to avoid
  // confusion.

  inline
  WrongArgumentCount::WrongArgumentCount (unsigned formal,
					  unsigned effective,
					  const std::string& fun)
    : UrbiException ((boost::format ("expected %1% arguments, given %2%")
		      % (formal-1)
		      % (effective-1)).str (),
		     fun)
  {
  }

  inline
  WrongArgumentCount::WrongArgumentCount (unsigned minformal,
					  unsigned maxformal,
					  unsigned effective,
					  const std::string& fun)
    : UrbiException ((boost::format ("expected between %1% and %2% arguments, "
				     "given %3%")
		      % (minformal-1)
		      % (maxformal-1)
		      % (effective-1)).str (),
		     fun)
  {
  }

  inline
  BadInteger::BadInteger (libport::ufloat effective, const std::string& fun)
    : UrbiException ((boost::format ("expected integer, got %1%")
		      % effective).str (),
		     fun)
  {
  }

  inline
  ImplicitTagComponentError::ImplicitTagComponentError (const ast::loc& l)
    : UrbiException ("illegal component for implicit tag", l)
  {
  }

  inline
  SchedulingError::SchedulingError (const std::string& msg)
    : UrbiException (msg)
  {
  }

  inline
  void
  check_arg_count (unsigned formal, unsigned effective, const std::string& fun)
  {
    if (formal != effective)
      throw WrongArgumentCount(formal, effective, fun);
  }

  inline
  void
  check_arg_count (unsigned minformal, unsigned maxformal,
		   unsigned effective, const std::string& fun)
  {
    if (effective < minformal || maxformal < effective)
      throw WrongArgumentCount(minformal, maxformal, effective, fun);
  }

}; // end of namespace object

#endif //! OBJECT_URBI_EXCEPTION_HXX
