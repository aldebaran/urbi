/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#ifndef OBJECT_FORMAT_INFO_HXX
# define OBJECT_FORMAT_INFO_HXX

namespace object
{

  template <>
  struct CxxConvert<FormatInfo::Align::position>
  {
    typedef FormatInfo::Align::position target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      type_check<Float>(o, idx);
      int val = o->as<Float>()->to_int();
      return (val) ? ((val > 0)
                      ? FormatInfo::Align::RIGHT
                      : FormatInfo::Align::LEFT)
        : FormatInfo::Align::CENTER ;
    }

    static rObject
    from(target_type v)
    {
      return new Float(int(v));
    }
  };

  template <>
  struct CxxConvert<FormatInfo::Case::mode>
  {
    typedef FormatInfo::Case::mode target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      type_check<Float>(o, idx);
      int val = o->as<Float>()->to_int();
      return (val) ? ((val > 0)
                      ? FormatInfo::Case::UPPER
                      : FormatInfo::Case::LOWER)
        : FormatInfo::Case::UNDEFINED ;
    }

    static rObject
    from(target_type v)
    {
      return new Float(int(v));
    }
  };

} // namespace object

#endif // !OBJECT_FORMAT_INFO_HXX
