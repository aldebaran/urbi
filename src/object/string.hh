/**
 ** \file object/string-class.hh
 ** \brief Definition of the URBI object string.
 */

#ifndef OBJECT_STRING_CLASS_HH
# define OBJECT_STRING_CLASS_HH

# include <object/cxx-object.hh>

namespace object
{
  extern rObject string_class;

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
    std::string format(runner::Runner& r, rList values);
    std::string plus(runner::Runner& r, rObject rhs);
    bool lt(const std::string& rhs);
    std::string fresh();
    std::string set(const std::string& rhs);
    size_type size();
    rList split(const std::string& sep);
    std::string star(unsigned int times);

    static const std::string type_name;
    virtual std::string type_name_get() const;

  private:
    value_type content_;

  public:
    static void initialize(CxxObject::Binder<String>& binder);
    static bool string_added;
  };

  // Urbi functions
  std::string as_string(rObject from);
  std::string as_printable(rObject from);

}; // namespace object

# include <object/cxx-object.hxx>

#endif // !OBJECT_STRING_CLASS_HH
