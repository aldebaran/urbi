/**
 ** \file object/float-class.cc
 ** \brief Creation of the URBI object float.
 */

#include <cmath>

#include "object/float-class.hh"
#include "object/object.hh"
#include "object/atom.hh"
#include "object/urbi-exception.hh"

namespace object
{
  rObject float_class;

  /*-------------------.
  | Float primitives.  |
  `-------------------*/

  // This macro is used to generate modulo
  // and division functions.
#define FCT_OP_PROTECTED(Name, Operator, ErrorMessage)                  \
  static                                                                \
  libport::ufloat                                                       \
  float_##Name (libport::ufloat l, libport::ufloat r)                   \
  {                                                                     \
    if (r == 0)                                                         \
      throw UrbiException("Operator " #Operator, ErrorMessage);         \
    return l Operator r;                                                \
  }

#define FCT_M_PROTECTED(Name, Method, ErrorMessage)                     \
  static                                                                \
  libport::ufloat                                                       \
  float_##Name (libport::ufloat l, libport::ufloat r)                   \
  {                                                                     \
    if (r == 0)                                                         \
      throw UrbiException(#Method, ErrorMessage);                       \
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

# define epsilontilde 0.0001
    return fabs(l - r) <= epsilontilde;
# undef epsilontilde
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
    if (x > 0)
      return 1;
    else if (x < 0)
      return -1;
    return 0;
  }

  static float
  float_random (libport::ufloat x)
  {
    float res = 0.f;
    const long long range = libport::to_long_long (x);
    if (range != 0)
      res = rand () % range;
    return res;
  }

  static float
  float_sqr (libport::ufloat x)
  {
    return x * x;
  }

  /// Internal macro used to define a primitive for float numbers.
  /// \param Name primitive's name
  /// \param Call C++ code executed when primitive is called.
  /// \param Pre C++ code executed before call (typically to check args)
#define PRIMITIVE_1_FLOAT_(Name, Call, Pre)             \
  rObject                                               \
  float_class_ ## Name (rContext, objects_type args)	\
  {                                                     \
    FETCH_ARG(1, Float);                                \
    Pre;                                                \
    return new Float(Call(arg1->value_get()));          \
  }

  /// Define a primitive for float numbers.
  /// \param Call Name primitive's name
  /// \param Pre C++ code executed when primitive is called.
#define PRIMITIVE_1_FLOAT(Name, Call)                   \
  PRIMITIVE_1_FLOAT_(Name, Call, ;)

#define PRIMITIVE_1_FLOAT_CHECK_POSITIVE(Name, Call)    \
  PRIMITIVE_1_FLOAT_(Name, Call,                        \
    if (args[1].unsafe_cast<Float>()->value_get() < 0)  \
      throw UrbiException(#Name, "argument has to be positive"))

#define PRIMITIVE_1_FLOAT_CHECK_RANGE(Name, Call, Min, Max)     \
  PRIMITIVE_1_FLOAT_(Name, Call,                                \
    if ((args[1].unsafe_cast<Float>()->value_get() < Min)       \
        || (args[1].unsafe_cast<Float>()->value_get() > Max))   \
      throw UrbiException(#Name, "invalid range"))

#define PRIMITIVE_2_FLOAT(Name, Call)                   \
  PRIMITIVE_2_V(float, Name, Call, Float, Float, Float)

#define PRIMITIVE_OP_FLOAT(Name, Op)                    \
  PRIMITIVE_OP_V(float, Name, Op, Float, Float, Float)

  PRIMITIVE_OP_FLOAT(add, +)
  PRIMITIVE_2_FLOAT(div, float_div)
  PRIMITIVE_OP_FLOAT(mul, *)
  PRIMITIVE_OP_FLOAT(sub, -)
  PRIMITIVE_2_FLOAT(pow, powf)
  PRIMITIVE_2_FLOAT(mod, float_mod)

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

  PRIMITIVE_1_FLOAT(sin, sin)
  PRIMITIVE_1_FLOAT_CHECK_RANGE(asin, asin, -1, 1)
  PRIMITIVE_1_FLOAT(cos, cos)
  PRIMITIVE_1_FLOAT_CHECK_RANGE(acos, acos, -1, 1)
  PRIMITIVE_1_FLOAT(tan, tan)
  PRIMITIVE_1_FLOAT(atan, atan)
  PRIMITIVE_1_FLOAT(sgn, float_sgn)
  PRIMITIVE_1_FLOAT(abs, fabs)
  PRIMITIVE_1_FLOAT(exp, exp)
  PRIMITIVE_1_FLOAT_CHECK_POSITIVE(log, log)
  PRIMITIVE_1_FLOAT(round, round)
  PRIMITIVE_1_FLOAT(random, float_random)
  PRIMITIVE_1_FLOAT(trunc, trunc)
  PRIMITIVE_1_FLOAT(sqr, float_sqr)
  PRIMITIVE_1_FLOAT_CHECK_POSITIVE(sqrt, sqrt)

#undef PRIMITIVE_2_FLOAT
#undef PRIMITIVE_1_FLOAT_CHECK_RANGE
#undef PRIMITIVE_1_FLOAT_CHECK_POSITIVE
#undef PRIMITIVE_1_FLOAT
#undef PRIMITIVE_1_FLOAT_
#undef PRIMITIVE_OP_FLOAT

  /// Initialize the Float class.
  void
  float_class_initialize ()
  {
#define DECLARE(Call, Name)                      \
    DECLARE_PRIMITIVE(float, Name, Call)

    DECLARE(add, +);
    DECLARE(div, /);
    DECLARE(mul, *);
    DECLARE(sub, -);
    DECLARE(pow, **);
    DECLARE(mod, %);

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
    DECLARE(sqr);
    DECLARE(sqrt);
#undef DECLARE

  }

}; // namespace object
