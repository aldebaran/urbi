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
      Float(rFloat model);

      value_type& value_get();
      value_type  value_get() const;
      int to_int(const std::string& func) const;

      // Urbi methods
      rFloat acos();
      rFloat asin();
      rFloat atan();
      rFloat cos();
      rFloat exp();
      rFloat fabs();
      rFloat log();
      rFloat minus(objects_type args);
      using Object::operator<;
      rObject operator <(rFloat rhs);
      rFloat operator +(rFloat rhs);
      rFloat operator *(rFloat rhs);
      rFloat operator /(rFloat rhs);
      rFloat operator %(rFloat rhs);
      rFloat operator <<(rFloat rhs);
      rFloat operator >>(rFloat rhs);
      rFloat operator ^(rFloat rhs);
      rFloat pow(rFloat rhs);
      rFloat random();
      rFloat round();
      rFloat set(rFloat rhs);
      rFloat sin();
      rFloat sqrt();
      rFloat tan();
      rFloat trunc();

      // Urbi functions
      static rString as_string(rObject from);
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

#endif // !OBJECT_FLOAT_CLASS_HH
