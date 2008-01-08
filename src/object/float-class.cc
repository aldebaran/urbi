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

  static libport::ufloat
  float_req (libport::ufloat l, libport::ufloat r)
  {
    // FIXME: get epsilontilde from environment

    // ENSURE_COMPARISON ("Approximate", l, r);
    // UVariable *epsilontilde =
    //   ::urbiserver->getVariable(MAINDEVICE, "epsilontilde");
    // if (epsilontilde)
    //   $EXEC$
    // else
    //   return 0;

#define epsilontilde 0.0001
    return fabs(l - r) <= epsilontilde;
#undef epsilontilde
  }

  static float
  float_deq (libport::ufloat l, libport::ufloat r)
  {
    // FIXME: get deltas for l and r

    // ENSURE_COMPARISON ("Approximate", l, r);
    libport::ufloat dl = 0.f;
    libport::ufloat dr = 0.f;
    // dl = 0, get l->delta
    // dr = 0, get r->delta

    return fabs(l - r) <= dl + dr;
  }

  static float
  float_peq (libport::ufloat l, libport::ufloat r)
  {
    // FIXME: get epsilonpercent from environment
    // FIXME: return error on div by 0

    // ENSURE_COMPARISON ("Approximate", l, r);
    // UVariable *epsilonpercent =
    //   ::urbiserver->getVariable(MAINDEVICE, "epsilonpercent");
    // if (epsilonpercent)
    //   $EXEC$
    // else
    //   return 0;

    if (r == 0)
      // FIXME: error
      return 0;

# define epsilonpercent 0.0001
    return fabs(1.f - l / r) < epsilonpercent;
# undef epsilonpercent
  }

  static float
  float_sgn (libport::ufloat x)
  {
    return
      0 < x ?  1 :
      x < 0 ? -1 :
      0;
  }

  static float
  float_random (libport::ufloat x)
  {
    float res = 0.f;
    const long long range = libport::to_long_long (x);
    if (range)
      res = rand () % range;
    return res;
  }

  // This is quite hideous, to be cleaned once we have integers.
# define INTEGER_BIN_OP(Name, Op)					\
  static float								\
  float_ ## Name (libport::ufloat lhs, libport::ufloat rhs)		\
  {									\
    return libport::to_long_long (lhs) Op libport::to_long_long (rhs);	\
  }

  INTEGER_BIN_OP(lshift, <<)
  INTEGER_BIN_OP(rshift, >>)
  INTEGER_BIN_OP(xor,    ^)

# undef INTEGER_BIN_OP



/*------------------------------------------------------------.
| Float Primitives.                                           |
|                                                             |
| I.e., the signature is rLobby x objects_type -> rObject.  |
`------------------------------------------------------------*/


  /// Clone.
  static rObject
  float_class_clone(rLobby, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    FETCH_ARG(0, Float);
    return clone(arg0);
  }


  /// Change the value.
  static rObject
  float_class_set(rLobby, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    FETCH_ARG(0, Float);
    FETCH_ARG(1, Float);
    arg0->value_set (arg1->value_get());
    return arg0;
  }


  /// Unary or binary minus.
  static rObject
  float_class_sub(rLobby, objects_type args)
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
  float_class_ ## Name (rLobby, objects_type args)	\
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
  PRIMITIVE_OP_FLOAT(add, +)
  PRIMITIVE_2_FLOAT(div, float_div)
  PRIMITIVE_OP_FLOAT(mul, *)
  PRIMITIVE_2_FLOAT(mod, float_mod)
  PRIMITIVE_2_FLOAT(pow, powf)

  PRIMITIVE_2_FLOAT(lshift, float_lshift) // <<
  PRIMITIVE_2_FLOAT(rshift, float_rshift) // >>
  PRIMITIVE_2_FLOAT(xor,    float_xor)    // ^

  PRIMITIVE_OP_FLOAT(land, &&)
  PRIMITIVE_OP_FLOAT(lor, ||)

  PRIMITIVE_OP_FLOAT(equ, ==)
  PRIMITIVE_2_FLOAT(req, float_req) //REQ ~=
  PRIMITIVE_2_FLOAT(deq, float_deq) //DEQ =~=
  PRIMITIVE_2_FLOAT(peq, float_peq) //PEQ %=
  PRIMITIVE_OP_FLOAT(neq, !=)

  PRIMITIVE_OP_FLOAT(lth, <)
  PRIMITIVE_OP_FLOAT(leq, <=)
  PRIMITIVE_OP_FLOAT(gth, >)
  PRIMITIVE_OP_FLOAT(geq, >=)

  PRIMITIVE_0_FLOAT(sin, sin)
  PRIMITIVE_0_FLOAT_CHECK_RANGE(asin, asin, -1, 1)
  PRIMITIVE_0_FLOAT(cos, cos)
  PRIMITIVE_0_FLOAT_CHECK_RANGE(acos, acos, -1, 1)
  PRIMITIVE_0_FLOAT(tan, tan)
  PRIMITIVE_0_FLOAT(atan, atan)
  PRIMITIVE_0_FLOAT(sgn, float_sgn)
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
#define DECLARE(Call, Name)                      \
    DECLARE_PRIMITIVE(float, Name, Call)

    DECLARE(add, +);
    DECLARE(div, /);
    DECLARE(mul, *);
    DECLARE(sub, -);
    DECLARE(pow, **);
    DECLARE(mod, %);

    DECLARE(lshift, <<);
    DECLARE(rshift, >>);
    DECLARE(xor,    ^);

    DECLARE(land, &&);
    DECLARE(lor, ||);

    DECLARE(equ, ==);
    DECLARE(req, ~=);
    DECLARE(deq, =~=);
    DECLARE(peq, %=);
    DECLARE(neq, !=);

    DECLARE(lth, <);
    DECLARE(leq, <=);
    DECLARE(gth, >);
    DECLARE(geq, >=);
#undef  DECLARE

#define DECLARE(Name)                      \
    DECLARE_PRIMITIVE(float, Name, Name)

    DECLARE(clone);
    DECLARE(set);

    DECLARE(sin);
    DECLARE(asin);
    DECLARE(cos);
    DECLARE(acos);
    DECLARE(tan);
    DECLARE(atan);
    DECLARE(sgn);
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
