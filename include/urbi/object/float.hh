/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/float.hh
 ** \brief Definition of urbi::object::Float.
 */

#ifndef OBJECT_FLOAT_HH
# define OBJECT_FLOAT_HH

# include <libport/ufloat.hh>

# include <urbi/object/cxx-object.hh>
# include <urbi/object/fwd.hh>
# include <urbi/object/equality-comparable.hh>

namespace urbi
{
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

      /// The preferred targets for conversions.
      typedef int int_type;
      typedef unsigned int unsigned_type;
      typedef long long long_type;

      // The type used when we try to convert a Float into an integral
      // value.
      typedef long_type integer_type;

      value_type& value_get();
      const value_type& value_get() const;

      /// False iff equals zero.
      virtual bool as_bool() const;

      /// Conversion to long long.
      /// \requires is_integer().
      integer_type as_integer() const;

      // Comparison. All of them for performance reasons.
      bool operator<=(const value_type& rhs) const;
      // < is a virtual method of object, we cannot overload it
      bool less_than(const value_type& rhs) const;
      bool operator>=(const value_type& rhs) const;
      bool operator> (const value_type& rhs) const;
      bool operator!=(const value_type& rhs) const;

      // Iteration
      void each(Executable* action);
      void each_pipe(Executable* action);

      /// \name Conversions.
      /// \{
      // FIXME: For some reason I don't understand, MSVC fails to
      // link when we pass const ref strings here...
      int_type
        to_int_type(const std::string fmt =
                    "expected integer, got %s") const;
      long_type
        to_long_type(const std::string fmt =
                     "expected integer, got %s") const;
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
      virtual std::string as_string() const;
      value_type atan() const;
      value_type atan2(value_type) const;
      value_type cos() const;
      value_type exp() const;
      value_type fabs() const;
      value_type log() const;
      value_type minus() const;
      value_type minus(Float::value_type rhs) const;
      value_type plus() const;
      value_type plus(Float::value_type rhs) const;

    protected:
      /// Limit object to be referenced by all Floats.
      static rObject limits;

    public:
      /// Number of digits (in radix base) in the mantissa.
      static unsigned_type limit_digits();
      /// Number of digits (in decimal base) that can be represented
      /// without change.
      static unsigned_type limit_digits10();

      /// Minimum positive normalized value.
      static value_type limit_min();
      /// Maximum finite value.
      static value_type limit_max();

      /// Machine epsilon (the difference between 1 and the least
      /// value greater than 1 that is representable).
      static value_type limit_epsilon();

      /// Minimum negative integer value for the exponent that
      /// generates a normalized floating-point number.
      static int_type limit_min_exponent();
      /// Minimum negative integer value such that 10 raised to that
      /// power generates a normalized floating-point number.
      static int_type limit_min_exponent10();

      /// Maximum integer value for the exponent that generates a
      /// normalized floating-point number.
      static unsigned_type limit_max_exponent();
      /// Maximum integer value such that 10 raised to that power
      /// generates a normalized finite floating-point number.
      static unsigned_type limit_max_exponent10();

      /// Base of the exponent of the representation.
      static unsigned_type limit_radix();

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

      value_type ceil() const;
      value_type floor() const;
      unsigned_type random() const;
      void srandom() const;
      value_type round() const;
      rList seq() const;
      /// -1, 0, or 1.
      int_type sign() const;
      value_type sin() const;
      value_type sqrt() const;
      value_type tan() const;
      value_type trunc() const;


    public:

      /// Whether is infinity.
      bool is_inf() const;
      /// Whether is convertible to an integer.
      bool is_integer() const;
      /// Whether is not a number.
      bool is_nan() const;
      /// Infinity.
      static value_type inf();
      /// Not a number.
      static value_type nan();

      rHash hash() const;


    /*----------.
    | Details.  |
    `----------*/

    private:

      value_type value_;

      URBI_CXX_OBJECT(Float, CxxObject);
    };

  } // namespace object
}

# include <urbi/object/cxx-object.hxx>
# include <urbi/object/slot.hxx>
# include <urbi/object/float.hxx>

#endif // !OBJECT_FLOAT_HH
