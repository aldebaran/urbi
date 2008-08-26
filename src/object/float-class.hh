/**
 ** \file object/float-class.hh
 ** \brief Definition of the URBI object float.
 */

#ifndef OBJECT_FLOAT_CLASS_HH
# define OBJECT_FLOAT_CLASS_HH

# include <libport/ufloat.hh>

# include <object/cxx-object.hh>

namespace object
{

  extern rObject float_class;

  class Float: public CxxObject
  {
  public:
    typedef libport::ufloat value_type;

    Float();
    Float(value_type value_);
    Float(const rFloat& model);

    value_type& value_get();
    value_type  value_get() const;
    int to_int(const libport::Symbol func) const;
    unsigned int to_unsigned_int(const libport::Symbol func) const;

    // Urbi methods
    rFloat acos();
    rFloat asin();
    rFloat atan();
    rFloat cos();
    rFloat exp();
    rFloat fabs();
    rFloat log();
    rFloat minus(objects_type& args);
    using Object::operator<;
    rObject operator <(const rFloat& rhs);
    rFloat operator ~();
    rFloat operator |(const rFloat& rhs);
    rFloat operator &(const rFloat& rhs);
    rFloat operator +(const rFloat& rhs);
    rFloat operator *(const rFloat& rhs);
    rFloat operator /(const rFloat& rhs);
    rFloat operator %(const rFloat& rhs);
    rFloat operator <<(const rFloat& rhs);
    rFloat operator >>(const rFloat& rhs);
    rFloat operator ^(const rFloat& rhs);
    rFloat pow(const rFloat& rhs);
    rFloat random();
    rFloat round();
    rFloat set(const rFloat& rhs);
    rFloat sin();
    rFloat sqrt();
    rFloat tan();
    rFloat trunc();

    // Urbi functions
    static rString as_string(const rObject& from);
    static rFloat inf();
    static rFloat nan();

    // Binding mechanisms
    static void initialize(CxxObject::Binder<Float>& binder);
    static const std::string type_name;
    static bool float_added;
    virtual std::string type_name_get() const;

  private:
    value_type value_;
  };

} // namespace object

# include <object/cxx-object.hxx>

#endif // !OBJECT_FLOAT_CLASS_HH
