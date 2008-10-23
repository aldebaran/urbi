/**
 ** \file object/urbi-exception.hxx
 ** \brief Implementation of Exception
 */

#ifndef OBJECT_URBI_EXCEPTION_HXX
# define OBJECT_URBI_EXCEPTION_HXX

namespace object
{

  /*--------------.
  | UrbiException |
  `--------------*/

  inline
  UrbiException::UrbiException(rObject value, const call_stack_type& bt)
    : value_(value)
    , backtrace_(bt)
  {
  }
} // namespace object

#endif //! OBJECT_URBI_EXCEPTION_HXX
