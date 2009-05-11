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
    float as_float();
    std::string as_string();
    std::string as_printable();

    unsigned int distance(rString other);
    std::string format(rObject values);
    std::string plus(rObject rhs);
    bool lt(const std::string& rhs);
    std::string fresh();
    /// Convert every occurrence of \a from to \a to.
    std::string replace(const std::string& from, const std::string& to);
    std::string set(const std::string& rhs);
    size_type size();
    std::vector<std::string> split(const std::string& sep,
                                   int limit = -1,
                                   bool keep_delim = false,
                                   bool keep_empty = true);
    std::vector<std::string> split(const std::vector<std::string>& sep,
                                   int limit = -1,
                                   bool keep_delim = false,
                                   bool keep_empty = true);
    std::string star(unsigned int times);

    /// [from, to].
    std::string sub(unsigned int from, unsigned int to);
    /// [idx].
    std::string sub(unsigned int idx);

    /// [from, to] = v.
    std::string sub_eq(unsigned int from, unsigned int to,
                       const std::string& v);
    /// [idx] = v.
    std::string sub_eq(unsigned int idx,
                       const std::string& v);

    /// Return a new string with all upper case made lower case.
    std::string to_lower();
    /// Return a new string with all lower case made upper case.
    std::string to_upper();

    static std::string fromAscii(rObject, int code);

    int         toAscii();
  private:
    value_type content_;

    void check_bounds(unsigned int from, unsigned int to);

    URBI_CXX_OBJECT(String);
  };

  // Urbi functions

}; // namespace object

# include <object/cxx-object.hxx>

#endif // !OBJECT_STRING_CLASS_HH
