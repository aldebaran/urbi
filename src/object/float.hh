/**
 ** \file object/float-class.hh
 ** \brief Definition of the URBI object float.
 */

#ifndef OBJECT_FLOAT_CLASS_HH
# define OBJECT_FLOAT_CLASS_HH

# include <libport/ufloat.hh>

# include <object/cxx-object.hh>
# include <object/fwd.hh>
# include <object/equality-comparable.hh>

namespace object
{

  class URBI_SDK_API Float
    : public CxxObject
    , public EqualityComparable<Float, libport::ufloat>
  {

  /*--------------.
  | C++ methods.  |
  `--------------*/

  public:

    typedef Float self_type;
    typedef libport::ufloat value_type;

    /// The preferred target for unsigned casts.
    typedef unsigned long unsigned_type;

    value_type& value_get();
    const value_type& value_get() const;

    /// False iff equals zero.
    virtual bool as_bool() const;

    // Comparison.
    bool operator<=(const value_type& rhs) const;

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

    value_type acos() const;
    value_type asin() const;
    std::string format(rFormatInfo finfo) const;
    std::string as_string() const;
    value_type atan() const;
    value_type atan2(value_type) const;
    value_type cos() const;
    value_type exp() const;
    value_type fabs() const;
    value_type log() const;
    value_type minus(const objects_type& args) const;
    value_type plus(const objects_type& args) const;
    using Object::operator<;

    // Operations on unsigned.
    unsigned_type operator ~() const;
    unsigned_type operator |(unsigned_type rhs) const;
    unsigned_type operator &(unsigned_type rhs) const;
    unsigned_type operator ^(unsigned_type rhs) const;
    unsigned_type operator <<(unsigned_type rhs) const;
    unsigned_type operator >>(unsigned_type rhs) const;

    value_type pow(value_type rhs) const;
    value_type operator *(value_type rhs) const;
    value_type operator /(value_type rhs) const;
    value_type operator %(value_type rhs) const;

    int random() const;
    value_type round() const;
    rList seq() const;
    value_type sin() const;
    value_type sqrt() const;
    value_type tan() const;
    value_type trunc() const;


  /*-----------------.
  | Urbi functions.  |
  `-----------------*/

  public:

    static value_type inf();
    static value_type nan();


  /*----------.
  | Details.  |
  `----------*/

  private:

    value_type value_;

    URBI_CXX_OBJECT(Float);
  };

} // namespace object

# include <object/cxx-object.hxx>
# include <object/slot.hxx>

#endif // !OBJECT_FLOAT_CLASS_HH
