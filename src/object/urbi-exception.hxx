/**
 ** \file object/urbi-exception.hxx
 ** \brief Implementation of Exception
 */

#ifndef OBJECT_URBI_EXCEPTION_HXX
# define OBJECT_URBI_EXCEPTION_HXX

# include <boost/format.hpp>

namespace object
{

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
