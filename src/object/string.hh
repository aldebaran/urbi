/**
 ** \file object/string-class.hh
 ** \brief Definition of the URBI object string.
 */

#ifndef OBJECT_STRING_CLASS_HH
# define OBJECT_STRING_CLASS_HH

# include <object/cxx-object.hh>

namespace object
{
  class URBI_SDK_API String: public CxxObject
  {
  public:
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

    /// Urbi methods
    float as_float() const;
    /// False iff empty.
    virtual bool as_bool() const;
    std::string as_string() const;
    std::string as_printable() const;

    size_type distance(const std::string& other) const;
    bool empty() const;
    std::string format(rObject values) const;
    std::string plus(rObject rhs) const;
    bool lt(const std::string& rhs) const;
    std::string fresh() const;
    /// Convert every occurrence of \a from to \a to.
    std::string replace(const std::string& from, const std::string& to) const;
    std::string set(const std::string& rhs);
    size_type size() const;
    std::vector<std::string> split(const std::string& sep,
                                   int limit = -1,
                                   bool keep_delim = false,
                                   bool keep_empty = true) const;
    std::vector<std::string> split(const std::vector<std::string>& sep,
                                   int limit = -1,
                                   bool keep_delim = false,
                                   bool keep_empty = true) const;
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

    static std::string fromAscii(rObject, int code);

    int toAscii() const;
  private:
    value_type content_;

    void check_bounds(size_type from, size_type to) const;

    URBI_CXX_OBJECT(String);
  };

  // Urbi functions

}; // namespace object

# include <object/cxx-object.hxx>

#endif // !OBJECT_STRING_CLASS_HH
