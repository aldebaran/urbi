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

  /*------------.
  | C++ methods |
  `------------*/

  public:

    typedef libport::ufloat value_type;
    value_type& value_get();
    const value_type&  value_get() const;
    int to_int(const libport::Symbol func) const;
    unsigned int to_unsigned_int
     (const libport::Symbol func,
      const std::string fmt = "expected non-negative integer, got %1%") const;

    virtual
    std::ostream& special_slots_dump(std::ostream& o,
                                     runner::Runner&) const;


  /*-------------.
  | Urbi methods |
  `-------------*/

  public:

    // Construction

    Float();
    Float(value_type value_);
    Float(const rFloat& model);

    value_type acos();
    value_type asin();
    value_type atan();
    value_type cos();
    value_type exp();
    value_type fabs();
    value_type log();
    rFloat minus(objects_type& args);
    rFloat plus(objects_type& args);
    using Object::operator<;
    bool operator <(value_type rhs);
    int operator ~();
    int operator |(int rhs);
    int operator &(int rhs);
    int operator ^(int rhs);
    value_type operator *(value_type rhs);
    value_type operator /(value_type rhs);
    value_type operator %(value_type rhs);
    int operator <<(unsigned int rhs);
    int operator >>(unsigned int rhs);
    value_type pow(value_type rhs);
    int random();
    value_type round();
    rList seq();
    rFloat set(value_type rhs);
    value_type sin();
    value_type sqrt();
    value_type tan();
    value_type trunc();


  /*---------------.
  | Urbi functions |
  `---------------*/

  public:

    static std::string as_string(const rObject& from);
    static value_type inf();
    static value_type nan();


  /*--------.
  | Details |
  `--------*/

  private:

    value_type value_;


  /*---------------.
  | Binding system |
  `---------------*/

  public:

    static void initialize(CxxObject::Binder<Float>& binder);
    static const std::string type_name;
    static bool float_added;
    virtual std::string type_name_get() const;

  };

} // namespace object

# include <object/cxx-object.hxx>

#endif // !OBJECT_FLOAT_CLASS_HH
