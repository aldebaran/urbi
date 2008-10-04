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
