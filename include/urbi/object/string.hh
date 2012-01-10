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
  using libport::ufloat;

  namespace object
  {
    class URBI_SDK_API String
      : public CxxObject
      , public EqualityComparable<String, std::string>
    {
      URBI_CXX_OBJECT(String, CxxObject);
    public:
      typedef std::string value_type;
      typedef std::vector<value_type> strings_type;

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
      virtual value_type as_string() const;
      /// Return \this.
      rString asString();
      value_type as_printable() const;
#if !defined COMPILATION_MODE_SPACE
      value_type format(rFormatInfo finfo) const;
#endif
      size_type distance(const value_type& other) const;
      bool empty() const;
      value_type plus(rObject rhs) const;
      value_type fresh() const;
      /// Find next occurence of \a pattern starting at position \a pos.
      long find(const value_type& pattern, long pos = 0) const;
      /// Find previous occurence of \a pattern starting at position \a pos.
      long rfind(const value_type& pattern, long pos = -1) const;
      /// Convert every occurrence of \a from to \a to.
      value_type replace(const value_type& from, const value_type& to) const;
      const value_type& set(const value_type& rhs);
      size_type size() const;

      value_type
      join(const objects_type& os,
           const value_type& prefix = "",
           const value_type& suffix = "") const;

      /// Split on a single string-type separator.
      strings_type
      split(const value_type& sep,
            int limit = -1,
            bool keep_delim = false, bool keep_empty = true) const;
      /// The type of the previous function.
      typedef strings_type
        (String::*split_string_type)(const value_type& sep,
                                     int limit,
                                     bool keep_delim, bool keep_empty) const;

      /// Split on a list of string separators.
      strings_type
      split(const strings_type& sep,
            int limit = -1,
            bool keep_delim = false, bool keep_empty = true) const;
      /// The type of the previous function.
      typedef strings_type
      (String::*split_list_type)(const strings_type& sep,
                                 int limit,
                                 bool keep_delim, bool keep_empty) const;


      value_type star(size_type times) const;

      /// [from, to].
      value_type sub(ufloat from, ufloat to) const;
      /// [idx].
      value_type sub(ufloat idx) const;

      /// [from, to] = v.
      value_type sub_eq(ufloat from, ufloat to,
                         const value_type& v);
      /// [idx] = v.
      value_type sub_eq(ufloat idx,
                         const value_type& v);

      /// Return a new string with all upper case made lower case.
      value_type to_lower() const;
      /// Return a new string with all lower case made upper case.
      value_type to_upper() const;

      /// Functions is_XXX:
      /// Whether C isXXX is true for all characters.
      bool is_alnum() const;
      bool is_alpha() const;
      bool is_ascii() const;
      bool is_blank() const;
      bool is_cntrl() const;
      bool is_digit() const;
      bool is_graph() const;
      bool is_lower() const;
      bool is_print() const;
      bool is_punct() const;
      bool is_space() const;
      bool is_upper() const;
      bool is_xdigit() const;

      static value_type fromAscii(rObject, unsigned char code);

      rHash hash() const;

      unsigned char toAscii() const;

    private:
      value_type content_;

      /// Check that is a valid index, and return its value in bounds.
      /// \param large  Whether we accept idx == size().
      size_type index(ufloat idx, bool large = false) const;

      void check_bounds(ufloat from, ufloat to) const;

      long find_default(const value_type& pattern) const;
      long rfind_default(const value_type& pattern) const;
    };

    // Urbi functions

  }; // namespace object
}

# include <urbi/object/cxx-object.hxx>

#endif // !OBJECT_STRING_HH
