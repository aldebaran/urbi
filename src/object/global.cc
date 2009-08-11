/**
 ** \file object/global-class.cc
 ** \brief Creation of the URBI object global.
 */

#include <object/format-info.hh>

#include <object/global.hh>
#include <object/object.hh>

namespace object
{
  rObject global_class;

  /*--------------------.
  | Global primitives.  |
  `--------------------*/

  std::string
  global_format(const rObject, const std::string& str, rFormatInfo finfo)
  {
    int padsize;
    if ((padsize = finfo->width_get() - str.size()) <= 0)
      return str;

    std::string res(padsize, finfo->pad_get()[0]);
    if (finfo->alignment_get() == FormatInfo::Align::LEFT)
      return std::string(str).append(res);
    else if (finfo->alignment_get() == FormatInfo::Align::RIGHT)
      return res.append(str);
    else
      return res.insert((res.size() + 1) / 2, str);
  }

  void
  global_class_initialize()
  {
    global_class->slot_set(SYMBOL(format),
                           object::make_primitive(&global_format),
                           true);
  }

}; // namespace object
