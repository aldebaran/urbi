/*
 * Copyright (C) 2009-2010, 2012, Gostai S.A.S.
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
# include <urbi/runner/raise.hh>

namespace urbi
{
  namespace object
  {
    template <typename T>
    struct CxxConvert
    {
      /// type returned when conversion from Urbi.
      typedef       T& target_type;
      /// type taken when conversion to Urbi.
      typedef const T& source_type;

      static target_type
      to(rObject o);

      static rObject
      from(source_type v);
    };

    // Helper function
    template <typename T>
    rObject to_urbi(const T&);

    // Helper function
    template <typename T>
    typename CxxConvert<T>::target_type
    from_urbi(rObject);

    template<typename T>
    typename CxxConvert<T>::target_type
    from_urbi(rObject, unsigned idx);
  }
}

# include <urbi/object/cxx-conversions.hxx>

#endif
