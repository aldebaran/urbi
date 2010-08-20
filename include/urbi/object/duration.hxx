/*
 * Copyright (C) 2010, Gostai S.A.S.
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
      to(const rObject& o, unsigned idx)
      {
        type_check(o, Float::proto, idx);
        typedef long long time_t;
       // Multiply the time_duration by 1000000, not the time_t,
       // because in boost 1.38 the argument type of
       // posix_time::microseconds is not large enough.
        return boost::posix_time::microseconds(time_t(o->as<Float>()->value_get())) * 1000000;
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
