/**
 ** \file object/string-class.hh
 ** \brief Definition of the URBI object string.
 */

#ifndef OBJECT_STRING_CLASS_HH
# define OBJECT_STRING_CLASS_HH

# include <object/cxx-object.hh>

namespace object
{
  class String: public CxxObject
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
    std::string as_string();
    std::string as_printable();

    unsigned int distance(rString other);
    std::string format(runner::Runner& r, rList values);
    std::string plus(runner::Runner& r, rObject rhs);
    bool lt(const std::string& rhs);
    std::string fresh();
    std::string set(const std::string& rhs);
    size_type size();
    rList split(const std::string& sep);
    std::string star(unsigned int times);
    std::string sub(unsigned int from, unsigned int to);
    std::string sub(unsigned int idx);
    std::string sub_eq(unsigned int from, unsigned int to,
                       const std::string& v);
    std::string sub_eq(unsigned int idx,
                       const std::string& v);
    std::string to_lower();
    std::string to_upper();

  private:
    value_type content_;

    void check_bounds(unsigned int from, unsigned int to);

    URBI_CXX_OBJECT(String);
  };

  // Urbi functions

}; // namespace object

# include <object/cxx-object.hxx>

#endif // !OBJECT_STRING_CLASS_HH
