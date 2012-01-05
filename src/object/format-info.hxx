/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_FORMAT_INFO_HXX
# define OBJECT_FORMAT_INFO_HXX

namespace urbi
{
  namespace object
  {
    template <>
    struct CxxConvert<FormatInfo::Align::position>
    {
      typedef FormatInfo::Align::position target_type;
      static target_type
      to(const rObject& o)
      {
        type_check<Float>(o);
        switch (o->as<Float>()->sign())
        {
        case -1: return FormatInfo::Align::RIGHT;
        case  0: return FormatInfo::Align::CENTER;
        case  1: return FormatInfo::Align::LEFT;
        }
        pabort("Unreachable");
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
      to(const rObject& o)
      {
        type_check<Float>(o);
        switch (o->as<Float>()->sign())
        {
        case -1: return FormatInfo::Case::UPPER;
        case 0:  return FormatInfo::Case::UNDEFINED;
        case 1:  return FormatInfo::Case::LOWER;
        }
        unreachable();
      }

      static rObject
      from(target_type v)
      {
        return new Float(int(v));
      }
    };
  } // namespace object
}

#endif // !OBJECT_FORMAT_INFO_HXX
