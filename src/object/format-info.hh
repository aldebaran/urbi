/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
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

# include <urbi/object/fwd.hh>
# include <urbi/object/cxx-object.hh>
# include <urbi/object/cxx-conversions.hh>

namespace urbi
{
  namespace object
  {
    class FormatInfo: public CxxObject
    {
      URBI_CXX_OBJECT(FormatInfo, CxxObject);
    public:
      FormatInfo();
      FormatInfo(rFormatInfo model);

      /// \param check_end  whether to reject trailing characters
      /// after a format string (e.g., reject "%ss").
      void init_(const std::string& pattern, bool check_end);

      /// Initialize, with check_end enabled.
      void init(const std::string& pattern);

      virtual std::string as_string() const;

      /// \param for_float whether to return a form suitable to format
      /// floats with Boost.Format.
      std::string compute_pattern(bool for_float = false) const;


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


    private:
      ATTRIBUTE_R(Align::position, alignment);
      ATTRIBUTE_R(bool, alt);
      mutable bool consistent_;
      ATTRIBUTE_R(std::string, group);
      ATTRIBUTE_R(std::string, pad);
      ATTRIBUTE_r(std::string, pattern, , , , mutable);
      ATTRIBUTE_R(size_t, precision);
      ATTRIBUTE_R(std::string, prefix);
      // For positional arguments, such as %2%.
      // Applies if non-null.
      ATTRIBUTE_R(size_t, rank);
      ATTRIBUTE_R(std::string, spec);
      ATTRIBUTE_R(Case::mode, uppercase);
      ATTRIBUTE_R(size_t, width);

    private:
      void alignment_set(Align::position v);
      void alt_set(bool v);
      void group_set(std::string s);
      void pad_set(std::string v);
      void precision_set(size_t v);
      void prefix_set(std::string v);
      void rank_set(size_t v);
      void spec_set(std::string v);
      void uppercase_set(int v);
      void width_set(size_t v);
    };

  } // namaspace object
}

# include <object/format-info.hxx>

#endif // !OBJECT_FORMAT_INFO_HH
