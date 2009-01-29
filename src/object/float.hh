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

  class URBI_SDK_API Float: public CxxObject
  {

  /*------------.
  | C++ methods |
  `------------*/

  public:

    typedef libport::ufloat value_type;

    /// The preferred target for unsigned casts.
    typedef unsigned long unsigned_type;

    value_type& value_get();
    const value_type& value_get() const;

    /// \name Conversions.
    /// \{
    // FIXME: For some reason I don't understand, MSVC fails to
    // link when we pass const ref strings here...
    int
      to_int(const std::string fmt =
             "expected integer, got %s") const;
    long long
      to_long_long(const std::string fmt =
                   "expected integer, got %s") const;
    unsigned int
      to_unsigned_int(const std::string fmt =
                      "expected non-negative integer, got %s") const;
    /// The prefered conversion.
    unsigned_type
      to_unsigned_type(const std::string fmt =
                       "expected non-negative integer, got %s") const;
    /// \}

    virtual
      std::ostream& special_slots_dump(std::ostream& o) const;


  /*---------------.
  | Urbi methods.  |
  `---------------*/

  public:

    // Construction
    Float(value_type value_);
    Float(const rFloat& model);

    value_type acos();
    value_type asin();
    std::string as_string(int base = 10);
    value_type atan();
    value_type atan2(value_type);
    value_type cos();
    value_type exp();
    value_type fabs();
    value_type log();
    rFloat minus(objects_type& args);
    rFloat plus(objects_type& args);
    using Object::operator<;
    bool operator <(value_type rhs);

    // Operations on unsigned.
    unsigned_type operator ~();
    unsigned_type operator |(unsigned_type rhs);
    unsigned_type operator &(unsigned_type rhs);
    unsigned_type operator ^(unsigned_type rhs);
    unsigned_type operator <<(unsigned_type rhs);
    unsigned_type operator >>(unsigned_type rhs);

    value_type pow(value_type rhs);
    value_type operator *(value_type rhs);
    value_type operator /(value_type rhs);
    value_type operator %(value_type rhs);

    int random();
    value_type round();
    rList seq();
    value_type sin();
    value_type sqrt();
    value_type tan();
    value_type trunc();


  /*---------------.
  | Urbi functions |
  `---------------*/

  public:

    static value_type inf();
    static value_type nan();


  /*--------.
  | Details |
  `--------*/

  private:

    value_type value_;

    URBI_CXX_OBJECT(Float);
  };

} // namespace object

# include <object/cxx-object.hxx>
# include <object/slot.hxx>

#endif // !OBJECT_FLOAT_CLASS_HH
