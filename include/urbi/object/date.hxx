/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_OBJECT_DATE_HXX
# define URBI_OBJECT_DATE_HXX

namespace urbi
{
  namespace object
  {
    template<>
    struct CxxConvert<boost::posix_time::ptime>
    {
      typedef boost::posix_time::ptime target_type;
      static target_type
      to(const rObject& o, unsigned idx)
      {
        type_check(o, Date::proto, idx);
        return o->as<Date>()->as_boost();
      }

      static rObject
      from(const target_type& v)
      {
        return new Date(v);
      }
    };
  }
}

#endif
