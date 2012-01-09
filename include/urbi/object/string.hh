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
 ** \file object/string.hh
 ** \brief Definition of the Urbi object string.
 */

#ifndef OBJECT_STRING_HH
# define OBJECT_STRING_HH

# include <libport/ufloat.hh>
# include <urbi/object/cxx-object.hh>
# include <urbi/object/equality-comparable.hh>

namespace urbi
{
  namespace object
  {
    class URBI_SDK_API String
      : public CxxObject
      , public EqualityComparable<String, std::string>
    {
    public:
      typedef String self_type;
      typedef std::string value_type;

      // size_t would make more sense, but the only conversion coded is
      // "unsigned int", and on some machines (OSX) size_t is "unsigned
      // long".  Can't define conversion for "size_t" since in that
      // case, machines with "size_t == unsigned int" (e.g., Linux 386)
      // would break.  Either convert everthing to size_t, or to
      // "unsigned int".
      typedef unsigned int size_type;

      String();
      String(rString model);
      String(const value_type& value);
      const value_type& value_get() const;
      value_type& value_get();

      // Comparison.
      bool operator<=(const value_type& rhs) const;

      /// Urbi methods
      libport::ufloat as_float() const;
      /// False iff empty.
      virtual bool as_bool() const;
      /// Can be inefficient depending on the implementation of
      /// std::string.  If the concrete type is known, consider using
      /// value_get instead of as_string.
      virtual std::string as_string() const;
      std::string as_printable() const;
#if !defined COMPILATION_MODE_SPACE
      std::string format(rFormatInfo finfo) const;
#endif
      size_type distance(const std::string& other) const;
      bool empty() const;
      std::string plus(rObject rhs) const;
      std::string fresh() const;
      /// Convert every occurrence of \a from to \a to.
      std::string replace(const std::string& from, const std::string& to) const;
      const std::string& set(const std::string& rhs);
      size_type size() const;

      std::string
        join(const objects_type& os,
             const std::string& prefix = "",
             const std::string& suffix = "") const;

      std::vector<std::string>
        split(const std::string& sep,
              int limit = -1,
              bool keep_delim = false, bool keep_empty = true) const;
      std::vector<std::string>
        split(const std::vector<std::string>& sep,
              int limit = -1,
              bool keep_delim = false, bool keep_empty = true) const;

      std::string star(size_type times) const;

      /// [from, to].
      std::string sub(size_type from, size_type to) const;
      /// [idx].
      std::string sub(size_type idx) const;

      /// [from, to] = v.
      std::string sub_eq(size_type from, size_type to,
                         const std::string& v);
      /// [idx] = v.
      std::string sub_eq(size_type idx,
                         const std::string& v);

      /// Return a new string with all upper case made lower case.
      std::string to_lower() const;
      /// Return a new string with all lower case made upper case.
      std::string to_upper() const;

      /// Functions is_XXX:
      /// Return whether C isXXX is true for all characters.
      bool is_upper() const;
      bool is_lower() const;
      bool is_alpha() const;
      bool is_cntrl() const;
      bool is_space() const;
      bool is_digit() const;
      bool is_xdigit() const;
      bool is_alnum() const;
      bool is_punct() const;
      bool is_graph() const;
      bool is_print() const;

      static std::string fromAscii(rObject, unsigned char code);

      rHash hash() const;

      unsigned char toAscii() const;
    private:
      value_type content_;

      void check_bounds(size_type from, size_type to) const;

      URBI_CXX_OBJECT(String, CxxObject);
    };

    // Urbi functions

  }; // namespace object
}

# include <urbi/object/cxx-object.hxx>

#endif // !OBJECT_STRING_HH
