/*
 * Copyright (C) 2007-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/urbi-exception.hxx
 ** \brief Implementation of Exception
 */

#ifndef OBJECT_URBI_EXCEPTION_HXX
# define OBJECT_URBI_EXCEPTION_HXX

namespace urbi
{
  namespace object
  {
    /*----------------.
    | UrbiException.  |
    `----------------*/

    inline
    UrbiException::UrbiException(rObject value, const call_stack_type& bt)
      : value_(value)
      , backtrace_(bt)
    {
    }

  } // namespace object
}

#endif //! OBJECT_URBI_EXCEPTION_HXX
