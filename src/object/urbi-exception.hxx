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
