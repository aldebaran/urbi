/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/**
 ** \file object/format-info.hh
 ** \brief Definition of the Urbi object format-info.
 */

#ifndef OBJECT_FORMAT_INFO_HH
# define OBJECT_FORMAT_INFO_HH

# include <string>

# include <libport/attributes.hh>

# include <object/fwd.hh>
# include <object/cxx-object.hh>
# include <object/cxx-conversions.hh>

namespace object
{
  class FormatInfo: public CxxObject
  {
  public:
    FormatInfo();
    FormatInfo(rFormatInfo model);

    void init(const std::string& pattern);

    std::string as_string() const;

  public:
    struct Align
    {
      enum position
      {
        LEFT = -1,
        CENTER,
        RIGHT
      };
    };

    struct Case
    {
      enum mode
      {
        LOWER = -1,
        UNDEFINED,
        UPPER
      };
    };


    ATTRIBUTE_R(Align::position, alignment);
    ATTRIBUTE_R(bool, alt);
  private:
    mutable bool consistent_;
    ATTRIBUTE_R(std::string, group);
    ATTRIBUTE_R(std::string, pad);
    mutable std::string pattern_;
    const std::string& pattern_get() const;
    ATTRIBUTE_R(unsigned int, precision);
    ATTRIBUTE_R(std::string, prefix);
    ATTRIBUTE_R(std::string, spec);
    ATTRIBUTE_R(Case::mode, uppercase);
    ATTRIBUTE_R(size_t, width);

  private:

    void compute_pattern() const;
    rObject update_hook(const std::string& slot, rObject val);

    URBI_CXX_OBJECT(FormatInfo);
  };

} // namaspace object

# include <object/format-info.hxx>

#endif // !OBJECT_FORMAT_INFO_HH
