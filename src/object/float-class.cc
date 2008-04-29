/**
 ** \file object/float-class.cc
 ** \brief Creation of the URBI object float.
 */

#include <cmath>

#include "sdk/config.h"
#ifndef HAVE_ROUND
#  include <libport/ufloat.h>
#endif
#include <libport/lexical-cast.hh>

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
  Name (libport::ufloat l, libport::ufloat r)                           \
  {                                                                     \
    if (!r)								\
      throw PrimitiveError("Operator " #Operator, ErrorMessage);	\
    return l Operator r;                                                \
  }

#define FCT_M_PROTECTED(Name, Method, ErrorMessage)                     \
  static                                                                \
  libport::ufloat                                                       \
  Name (libport::ufloat l, libport::ufloat r)                           \
  {                                                                     \
    if (!r)								\
      throw PrimitiveError(#Method, ErrorMessage);			\
    return Method(l, r);                                                \
  }

  FCT_OP_PROTECTED(SLASH, /, "division by 0")
  FCT_M_PROTECTED(PERCENT, fmod, "modulo by 0")

#undef FCT_M_PROTECTED
#undef FCT_OP_PROTECTED

  static float
  random (libport::ufloat x)
  {
    float res = 0.f;
    const long long range = libport::to_long_long (x);
    if (range)
      res = rand () % range;
    return res;
  }

  int
  ufloat_to_int (libport::ufloat val, const std::string& func)
  {
    try {
      return libport::ufloat_to_int (val);
    }
    catch (libport::bad_numeric_cast& ue)
    {
      throw BadInteger (val, func);
      return 0;  // Keep the compiler happy
    }
  }

  // This is quite hideous, to be cleaned once we have integers.
# define INTEGER_BIN_OP(Name, Op)				\
  static float							\
  Name (libport::ufloat lhs, libport::ufloat rhs)               \
  {								\
    int ilhs = ufloat_to_int (lhs, #Op);			\
    int irhs = ufloat_to_int (rhs, #Op);			\
    return ilhs Op irhs;					\
  }

  INTEGER_BIN_OP(LT_LT, <<)
  INTEGER_BIN_OP(GT_GT, >>)
  INTEGER_BIN_OP(CARET, ^)

# undef INTEGER_BIN_OP

#define BIN_WRAPPER(Name, Call)                                 \
  static float                                                  \
  Name (libport::ufloat lhs, libport::ufloat rhs)               \
  {                                                             \
    return Call(lhs, rhs);                                      \
  }

BIN_WRAPPER(STAR_STAR, powf)

#undef BIN_WRAPPER

static float
abs(libport::ufloat v)
{
  return std::abs(v);
}

/*-------------------------------------------------------------------.
| Float Primitives.                                                  |
|                                                                    |
| I.e., the signature is runner::Runner& x objects_type -> rObject.  |
`--------------------------------------------------------------------*/


  /// asString.
  static rObject
  float_class_asString(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    if (args[0] == float_class)
      return String::fresh(SYMBOL(LT_Float_GT));
    // FIXME: Get rid of this cast by setting the precision for
    // streams to the same as used by lexical_cast (which is one more
    // than the default value, and this results in different output bw
    // 1.x and 2.x, a useless nuisance in the test suite.
    FETCH_ARG(0, Float);
    return String::fresh(libport::Symbol(string_cast
					 (float (arg0->value_get()))));
  }

  /// Clone.
  static rObject
  float_class_clone(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    FETCH_ARG(0, Float);
    return arg0->clone();
  }

  /// Infinity
  static rObject
  float_class_inf(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    return Float::fresh(std::numeric_limits<ufloat>::infinity());
  }

  /// NaN
  static rObject
  float_class_nan(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    return Float::fresh(std::numeric_limits<ufloat>::quiet_NaN());
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
    CHECK_ARG_COUNT_RANGE(1, 2);

    // Unary minus.
    FETCH_ARG(0, Float);
    if (args.size() == 1)
      return Float::fresh(- arg0->value_get());

    // Binary minus.
    FETCH_ARG(1, Float);
    return Float::fresh(arg0->value_get() - arg1->value_get());
  }

  // FIXME: Code duplication here, factor this macro with PRIMITIVE_1_
  // in primitives.hh.

  /// Internal macro used to define a primitive for float numbers.
  /// \param Name primitive's name
  /// \param Call C++ code executed when primitive is called.
  /// \param Pre C++ code executed before call (typically to check args)
#define PRIMITIVE_0_FLOAT_(Name, Pre)                           \
  static rObject                                                \
  float_class_ ## Name (runner::Runner&, objects_type args)	\
  {                                                             \
    CHECK_ARG_COUNT(1);                                         \
    FETCH_ARG(0, Float);                                        \
    Pre;                                                        \
    return Float::fresh(Name(arg0->value_get()));               \
  }

  /// Define a primitive for float numbers.
  /// \param Call Name primitive's name
#define PRIMITIVE_0_FLOAT(Name)                            \
  PRIMITIVE_0_FLOAT_(Name, )

#define PRIMITIVE_0_FLOAT_CHECK_POSITIVE(Name)			\
  PRIMITIVE_0_FLOAT_(Name,					\
     if (args[0]->value<Float>() < 0)                           \
       throw							\
	 PrimitiveError(#Name,					\
			"argument has to be positive"))

#define PRIMITIVE_0_FLOAT_CHECK_RANGE(Name,Min, Max)                    \
  PRIMITIVE_0_FLOAT_(Name,						\
     if (args[0]->value<Float>() < Min || Max < args[0]->value<Float>())\
       throw PrimitiveError(#Name, "invalid range"))

#define PRIMITIVE_2_FLOAT(Name)                            \
  PRIMITIVE_2_V(float, Name, Float, Float, Float)

#define PRIMITIVE_OP_FLOAT(Name, Op)                    \
  PRIMITIVE_OP_V(float, Name, Op, Float, Float, Float)

  // Binary arithmetics operators.
  PRIMITIVE_OP_FLOAT(PLUS, +)
  PRIMITIVE_2_FLOAT(SLASH)
  PRIMITIVE_OP_FLOAT(STAR, *)
  PRIMITIVE_2_FLOAT(PERCENT)

  PRIMITIVE_2_FLOAT(STAR_STAR)

  PRIMITIVE_2_FLOAT(LT_LT) // <<
  PRIMITIVE_2_FLOAT(GT_GT) // >>
  PRIMITIVE_2_FLOAT(CARET) // ^

  PRIMITIVE_OP_FLOAT(EQ_EQ, ==)

  PRIMITIVE_OP_FLOAT(LT, <)

  PRIMITIVE_0_FLOAT(sin)
  PRIMITIVE_0_FLOAT_CHECK_RANGE(asin, -1, 1)
  PRIMITIVE_0_FLOAT(cos)
  PRIMITIVE_0_FLOAT_CHECK_RANGE(acos, -1, 1)
  PRIMITIVE_0_FLOAT(tan)
  PRIMITIVE_0_FLOAT(atan)
  PRIMITIVE_0_FLOAT(abs)
  PRIMITIVE_0_FLOAT(exp)
  PRIMITIVE_0_FLOAT_CHECK_POSITIVE(log)
  PRIMITIVE_0_FLOAT(round)
  PRIMITIVE_0_FLOAT(random)
  PRIMITIVE_0_FLOAT(trunc)
  PRIMITIVE_0_FLOAT_CHECK_POSITIVE(sqrt)

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
    DECLARE_PRIMITIVE(float, Name)

    DECLARE(CARET);
    DECLARE(EQ_EQ);
    DECLARE(GT_GT);
    DECLARE(LT);
    DECLARE(LT_LT);
    DECLARE(MINUS);
    DECLARE(PERCENT);
    DECLARE(PLUS);
    DECLARE(SLASH);
    DECLARE(STAR);
    DECLARE(STAR_STAR);
    DECLARE(abs);
    DECLARE(acos);
    DECLARE(asString);
    DECLARE(asin);
    DECLARE(atan);
    DECLARE(clone);
    DECLARE(cos);
    DECLARE(exp);
    DECLARE(inf);
    DECLARE(log);
    DECLARE(nan);
    DECLARE(random);
    DECLARE(round);
    DECLARE(set);
    DECLARE(sin);
    DECLARE(sqrt);
    DECLARE(tan);
    DECLARE(trunc);
#undef DECLARE

  }

}; // namespace object
