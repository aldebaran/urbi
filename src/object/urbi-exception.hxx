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
    //FIXME: enable this? assert(location_set_);
    return loc_;
  }

  inline
  void
  UrbiException::location_set (const ast::loc& l)
  {
    loc_ = l;
    location_set_ = true;
  }

  inline
  bool
  UrbiException::location_is_set ()
  {
    return location_set_;
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
  PrimitiveError::PrimitiveError (const std::string& primitive,
				  const std::string& msg)
    : UrbiException (msg, primitive)
  {
  }

  inline
  WrongArgumentType::WrongArgumentType (Object::kind_type formal,
					Object::kind_type effective,
					const std::string& fun)
    : UrbiException (std::string ("unexpected argument type `")
		     + Object::string_of (effective) + "', expected `"
		     + Object::string_of (formal) + '\'',
		     fun)
  {
  }

  inline
  WrongArgumentType::WrongArgumentType (const std::string& fun)
    : UrbiException (std::string ("unexpected void"), fun)
  {
  }

  inline
  WrongArgumentCount::WrongArgumentCount (unsigned formal,
					  unsigned effective,
					  const std::string& fun)
    : UrbiException ((boost::format ("expected %1% arguments, given %2%")
		      % formal
		      % effective).str (),
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
		      % minformal
		      % maxformal
		      % effective).str (),
		     fun)
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
