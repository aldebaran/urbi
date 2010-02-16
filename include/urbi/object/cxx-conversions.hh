/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_CXX_CONVERSIONS_HH
# define OBJECT_CXX_CONVERSIONS_HH

# include <urbi/object/object.hh>

namespace urbi
{
  namespace object
  {
    template <typename T>
    struct CxxConvert
    {
      typedef T target_type;

      static target_type
      to(const rObject&, unsigned)
      {
        return target_type::No_such_conversion;
      }

      static rObject
      from(const target_type&)
      {
        return target_type::No_such_conversion;
      }
    };

    // Helper function
    template <typename T>
    rObject to_urbi(const T&);

    // Helper function
    template <typename T>
    T from_urbi(rObject);
  }
}

#include <urbi/object/cxx-conversions.hxx>

#endif
