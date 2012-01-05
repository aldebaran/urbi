/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef ANY_TO_BOOST_FUNCTION_HH
# define ANY_TO_BOOST_FUNCTION_HH

namespace urbi
{
  namespace object
  {
    template <typename T>
    struct AnyToBoostFunction
    {
      typedef T type;
      static T convert(T v);
    };
  }
}

#include <urbi/object/any-to-boost-function.hxx>

#endif
