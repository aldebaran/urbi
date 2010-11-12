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

/*--------------.
| Conversions.  |
`--------------*/

namespace urbi
{
  namespace object
  {
    template<>
    struct CxxConvert<boost::posix_time::ptime>
    {
      typedef boost::posix_time::ptime target_type;
      static target_type
      to(const rObject& o)
      {
        type_check(o, Date::proto);
        return o->as<Date>()->as_boost();
      }

      static rObject
      from(const target_type& v)
      {
        return new Date(v);
      }
    };

# define BOOST_GREG_CONVERSION(Name)                                      \
  template<>                                                              \
  struct CxxConvert<Date::Name ## _type>                                  \
  {                                                                       \
    typedef Date::Name ## _type target_type;                              \
    static target_type                                                    \
    to(const rObject& o)                                                  \
    {                                                                     \
      type_check(o, Float::proto);                                        \
      try                                                                 \
      {                                                                   \
        return target_type(o->as<Float>()->to_int_type());                \
      }                                                                   \
      catch(std::exception& e)                                            \
      {                                                                   \
        runner::raise_primitive_error(e.what());                          \
      }                                                                   \
    }                                                                     \
                                                                          \
    static rObject                                                        \
    from(target_type v)                                                   \
    {                                                                     \
      return new Float(v);                                                \
    }                                                                     \
  };                                                                      \

    BOOST_GREG_CONVERSION(year);
    BOOST_GREG_CONVERSION(month);
    BOOST_GREG_CONVERSION(day);

# undef BOOST_GREG_CONVERSION
  }
}


#endif
