/*
 * Copyright (C) 2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_OBJECT_DURATION_HXX
# define URBI_OBJECT_DURATION_HXX

namespace urbi
{
  namespace object
  {
    template<>
    struct CxxConvert<boost::posix_time::time_duration>
    {
      typedef boost::posix_time::time_duration target_type;
      static target_type
      to(const rObject& o)
      {
        type_check(o, Float::proto);
        // Keep the fractional part: compute with doubles.
        return boost::posix_time::microseconds(o->as<Float>()->value_get()
                                               * 1000000);
      }

      static rObject
      from(const target_type& v)
      {
        return new Duration(v);
      }
    };
  }
}

#endif
