/**
 ** \file object/float-class.cc
 ** \brief Creation of the URBI object float.
 */

#include <cmath>

#include "sdk/config.h"
#ifndef HAVE_ROUND
#  include "libport/ufloat.h"
#endif

#include "object/float-class.hh"
#include "object/object.hh"
#include "object/atom.hh"
#include "object/urbi-exception.hh"

namespace object
{
  rObject float_class;

  /*-----------------------------------------.
  | Routines to implement float primitives.  |
  `-----------------------------------------*/

  // This macro is used to generate modulo
  // and division functions.
#define FCT_OP_PROTECTED(Name, Operator, ErrorMessage)                  \
  static                                                                \
  libport::ufloat                                                       \
  float_##Name (libport::ufloat l, libport::ufloat r)                   \
  {                                                                     \
    if (!r)								\
      throw PrimitiveError("Operator " #Operator,                       \
			   ErrorMessage);                               \
    return l Operator r;                                                \
  }

#define FCT_M_PROTECTED(Name, Method, ErrorMessage)                     \
  static                                                                \
  libport::ufloat                                                       \
  float_##Name (libport::ufloat l, libport::ufloat r)                   \
  {                                                                     \
    if (!r)								\
      throw PrimitiveError(#Method, ErrorMessage);                      \
    return Method(l, r);                                                \
  }

  FCT_OP_PROTECTED(div, /, "division by 0")
  FCT_M_PROTECTED(mod, fmod, "modulo by 0")

#undef FCT_M_PROTECTED
#undef FCT_OP_PROTECTED

  static float
  float_random (libport::ufloat x)
  {
    float res = 0.f;
    const long long range = libport::to_long_long (x);
    if (range)
      res = rand () % range;
    return res;
  }

  static int
  ufloat_to_int (libport::ufloat val, const std::string& func)
  {
    try {
      return libport::ufloat_to_int (val);
    }
    catch (boost::numeric::bad_numeric_cast& ue)
    {
      throw BadInteger (val, func);
    }
  }

  // This is quite hideous, to be cleaned once we have integers.
# define INTEGER_BIN_OP(Name, Op)				\
  static float							\
  float_ ## Name (libport::ufloat lhs, libport::ufloat rhs)	\
  {								\
    int ilhs = ufloat_to_int (lhs, #Op);			\
    int irhs = ufloat_to_int (rhs, #Op);			\
    return ilhs Op irhs;					\
  }

  INTEGER_BIN_OP(lshift, <<)
  INTEGER_BIN_OP(rshift, >>)
  INTEGER_BIN_OP(xor,    ^)

# undef INTEGER_BIN_OP



/*-------------------------------------------------------------------.
| Float Primitives.                                                  |
|                                                                    |
| I.e., the signature is runner::Runner& x objects_type -> rObject.  |
`--------------------------------------------------------------------*/


  /// Clone.
  static rObject
  float_class_clone(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    FETCH_ARG(0, Float);
    return clone(arg0);
  }


  /// Change the value.
  static rObject
  float_class_set(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    FETCH_ARG(0, Float);
    FETCH_ARG(1, Float);
    arg0->value_set (arg1->value_get());
    return arg0;
  }


  /// Unary or binary minus.
  static rObject
  float_class_MINUS(runner::Runner&, objects_type args)
  {
    // FIXME: The error message requires 2 although 1 is ok.
    if (args.size () != 1 && args.size() != 2)
      throw WrongArgumentCount(2, args.size (), __PRETTY_FUNCTION__);
    FETCH_ARG(0, Float);
    if (args.size() == 1)
      return new Float(- arg0->value_get());
    else
    {
      FETCH_ARG(1, Float);
      return new Float(arg0->value_get() - arg1->value_get());
    }
  }

  /// Internal macro used to define a primitive for float numbers.
  /// \param Name primitive's name
  /// \param Call C++ code executed when primitive is called.
  /// \param Pre C++ code executed before call (typically to check args)
#define PRIMITIVE_0_FLOAT_(Name, Call, Pre)             \
  static rObject					\
  float_class_ ## Name (runner::Runner&, objects_type args)	\
  {                                                     \
    CHECK_ARG_COUNT(1);                                 \
    FETCH_ARG(0, Float);                                \
    Pre;                                                \
    return new Float(Call(arg0->value_get()));          \
  }

  /// Define a primitive for float numbers.
  /// \param Call Name primitive's name
#define PRIMITIVE_0_FLOAT(Name, Call)                   \
  PRIMITIVE_0_FLOAT_(Name, Call, )

#define PRIMITIVE_0_FLOAT_CHECK_POSITIVE(Name, Call)            \
  PRIMITIVE_0_FLOAT_(Name, Call,                                \
     if (VALUE(args[0], Float) < 0)                             \
       throw PrimitiveError(#Name,                              \
			    "argument has to be positive"))

#define PRIMITIVE_0_FLOAT_CHECK_RANGE(Name, Call, Min, Max)		\
  PRIMITIVE_0_FLOAT_(Name, Call,					\
     if (VALUE(args[0], Float) < Min || Max < VALUE(args[0], Float))	\
      throw PrimitiveError(#Name, "invalid range"))

#define PRIMITIVE_2_FLOAT(Name, Call)                   \
  PRIMITIVE_2_V(float, Name, Call, Float, Float, Float)

#define PRIMITIVE_OP_FLOAT(Name, Op)                    \
  PRIMITIVE_OP_V(float, Name, Op, Float, Float, Float)

  // Binary arithmetics operators.
  PRIMITIVE_OP_FLOAT(PLUS, +)
  PRIMITIVE_2_FLOAT(SLASH, float_div)
  PRIMITIVE_OP_FLOAT(STAR, *)
  PRIMITIVE_2_FLOAT(PERCENT, float_mod)
  PRIMITIVE_2_FLOAT(STAR_STAR, powf)

  PRIMITIVE_2_FLOAT(LT_LT, float_lshift) // <<
  PRIMITIVE_2_FLOAT(GT_GT, float_rshift) // >>
  PRIMITIVE_2_FLOAT(CARET,    float_xor) // ^

  PRIMITIVE_OP_FLOAT(EQ_EQ, ==)

  PRIMITIVE_OP_FLOAT(LT, <)

  PRIMITIVE_0_FLOAT(sin, sin)
  PRIMITIVE_0_FLOAT_CHECK_RANGE(asin, asin, -1, 1)
  PRIMITIVE_0_FLOAT(cos, cos)
  PRIMITIVE_0_FLOAT_CHECK_RANGE(acos, acos, -1, 1)
  PRIMITIVE_0_FLOAT(tan, tan)
  PRIMITIVE_0_FLOAT(atan, atan)
  PRIMITIVE_0_FLOAT(abs, fabs)
  PRIMITIVE_0_FLOAT(exp, exp)
  PRIMITIVE_0_FLOAT_CHECK_POSITIVE(log, log)
  PRIMITIVE_0_FLOAT(round, round)
  PRIMITIVE_0_FLOAT(random, float_random)
  PRIMITIVE_0_FLOAT(trunc, trunc)
  PRIMITIVE_0_FLOAT_CHECK_POSITIVE(sqrt, sqrt)

#undef PRIMITIVE_2_FLOAT
#undef PRIMITIVE_0_FLOAT_CHECK_RANGE
#undef PRIMITIVE_0_FLOAT_CHECK_POSITIVE
#undef PRIMITIVE_0_FLOAT
#undef PRIMITIVE_0_FLOAT_
#undef PRIMITIVE_OP_FLOAT

  /// Initialize the Float class.
  void
  float_class_initialize ()
  {
    /// \a Call gives the name of the C++ function, and \a Name that in Urbi.
#define DECLARE(Name)                      \
    DECLARE_PRIMITIVE(float, Name, Name)

    DECLARE(PLUS);
    DECLARE(SLASH);
    DECLARE(STAR);
    DECLARE(MINUS);
    DECLARE(STAR_STAR);
    DECLARE(PERCENT);

    DECLARE(LT_LT);
    DECLARE(GT_GT);
    DECLARE(CARET);

    DECLARE(EQ_EQ);

    DECLARE(LT);

    DECLARE(clone);
    DECLARE(set);

    DECLARE(sin);
    DECLARE(asin);
    DECLARE(cos);
    DECLARE(acos);
    DECLARE(tan);
    DECLARE(atan);
    DECLARE(abs);
    DECLARE(exp);
    DECLARE(log);
    DECLARE(round);
    DECLARE(random);
    DECLARE(trunc);
    DECLARE(sqrt);
#undef DECLARE

  }

}; // namespace object
