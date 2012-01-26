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

# include <libport/cmath>

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
        // On Debian Etch we cannot pass directly a double to
        // microseconds (even with Boost 1.40, whereas it is accepted
        // on Ubuntu 10.04, same Boost.  Besides, there can be
        // precision issues: be sure to preserve the fractional part.
        ufloat secs;
        ufloat frac = std::modf(o->as<Float>()->value_get(), &secs);
        target_type res;
        // "long" is explicitly the type used by these ctors.
        res += boost::posix_time::seconds(long(secs));
        res += boost::posix_time::microseconds(long(frac * 1000000));
        return res;
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
