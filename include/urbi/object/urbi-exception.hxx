/*
 * Copyright (C) 2007-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file urbi/object/urbi-exception.hxx
 ** \brief Implementation of UrbiException.
 */

#ifndef URBI_OBJECT_EXCEPTION_HXX
# define URBI_OBJECT_EXCEPTION_HXX

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

    inline const rObject&
    UrbiException::value() const
    {
      return *value_;
    }

  } // namespace object
}

#endif //! URBI_OBJECT_EXCEPTION_HXX
