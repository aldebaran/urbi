/**
 ** \file object/urbi-exception.hxx
 ** \brief Implementation of Exception
 */

#ifndef OBJECT_URBI_EXCEPTION_HXX
# define OBJECT_URBI_EXCEPTION_HXX

# include <boost/format.hpp>

namespace object
{

  inline
  RedefinitionError::RedefinitionError (libport::Symbol slot)
    : Exception ((boost::format ("slot redefinition: %1%")
		      % slot.name_get()).str())
  {
  }

  inline
  PrimitiveError::PrimitiveError (const libport::Symbol primitive,
				  const std::string& msg)
    : Exception(msg, primitive)
  {
  }

  inline
  StackExhaustedError::StackExhaustedError(const std::string& msg)
    : Exception(msg)
  {
  }

  inline
  WrongArgumentType::WrongArgumentType(const std::string& formal,
                                       const std::string& effective,
                                       const libport::Symbol fun)
    : Exception (std::string("unexpected argument type `")
		     + effective + "', expected `"
		     + formal + '\'',
		     fun)
  {
  }

  inline
  WrongArgumentType::WrongArgumentType (const libport::Symbol fun)
    : Exception("unexpected void", fun)
  {
  }

  inline
  WrongArgumentCount::WrongArgumentCount(unsigned formal,
                                         unsigned effective,
                                         const libport::Symbol fun)
    : Exception((boost::format ("expected %1% arguments, given %2%")
		      % (formal)
		      % (effective)).str (),
		     fun)
  {
  }

  inline
  WrongArgumentCount::WrongArgumentCount(unsigned minformal,
                                         unsigned maxformal,
                                         unsigned effective,
                                         const libport::Symbol fun)
    : Exception((boost::format("expected between %1% and %2% arguments, "
                                   "given %3%")
                     % minformal
                     % maxformal
                     % effective).str(),
                    fun)
  {
  }

  inline
  BadInteger::BadInteger(libport::ufloat effective, const libport::Symbol fun,
			 std::string fmt)
    : Exception((boost::format(fmt)
                     % effective).str(),
                    fun)
  {
  }

  inline
  ImplicitTagComponentError::ImplicitTagComponentError(const ast::loc& l)
    : Exception("illegal component for implicit tag", l)
  {
  }

  inline
  SchedulingError::SchedulingError(const std::string& msg)
    : Exception(msg)
  {
  }

  inline
  InternalError::InternalError(const std::string& msg)
    : Exception(msg)
  {
  }

  inline
  ParserError::ParserError(const ast::loc& loc, const std::string& msg)
    : Exception(msg, loc)
  {
  }

  inline bool
  Exception::was_displayed() const
  {
    return displayed_;
  }

  inline void
  Exception::set_displayed()
  {
    displayed_ = true;
  }
} // namespace object

#endif //! OBJECT_URBI_EXCEPTION_HXX
