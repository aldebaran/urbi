/**
 ** \file object/float-class.cc
 ** \brief Creation of the URBI object float.
 */

#include <cmath>

#include "object/float-class.hh"
#include "object/object.hh"
#include "object/atom.hh"

namespace object
{
  rObject float_class;

  /*-------------------.
  | Float primitives.  |
  `-------------------*/

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

#define DECLARE(Name, Call)				\
  rObject						\
  float_class_ ## Name (objects_type args)		\
  {							\
    assert(args[0]->kind_get() == Object::kind_float);	\
    assert(args[1]->kind_get() == Object::kind_float);	\
    rFloat l = args[0].unsafe_cast<Float> ();		\
    rFloat r = args[1].unsafe_cast<Float> ();		\
    return new Float(Call);				\
  }

#define DECLARE_U(Name, Call)				\
  rObject						\
  float_class_ ## Name (objects_type args)		\
  {							\
    assert(args[1]->kind_get() == Object::kind_float);	\
    rFloat x = args[1].unsafe_cast<Float> ();		\
    return new Float(Call);				\
  }

#define DECLARE_M(Name, Method)				\
  DECLARE(Name, Method(l->value_get(), r->value_get()))


  //FIXME: check if rvalue is 0 for % and / operators
#define DECLARE_OP(Name, Operator)			\
  DECLARE(Name, l->value_get() Operator r->value_get())

#define DECLARE_U_M(Name, Method)		\
  DECLARE_U(Name, Method(x->value_get()))


  DECLARE_OP(add, +)
  DECLARE_OP(div, /)
  DECLARE_OP(mul, *)
  DECLARE_OP(sub, -)
  DECLARE_M(pow, powf)
  DECLARE_M(mod, fmod)

  DECLARE_OP(land, &&)
  DECLARE_OP(lor, ||)

  DECLARE_OP(equ, ==)
  DECLARE_M(req, float_req) //REQ ~=
  DECLARE_M(deq, float_deq) //DEQ =~=
  DECLARE_M(peq, float_peq) //PEQ %=
  DECLARE_OP(neq, !=)

  DECLARE_OP(lth, <)
  DECLARE_OP(leq, <=)
  DECLARE_OP(gth, >)
  DECLARE_OP(geq, >=)


  DECLARE_U_M(sin, sin)
  DECLARE_U_M(asin, asin)
  DECLARE_U_M(cos, cos)
  DECLARE_U_M(acos, acos)
  DECLARE_U_M(tan, tan)
  DECLARE_U_M(atan, atan)
  DECLARE_U_M(sgn, float_sgn)
  DECLARE_U_M(abs, fabs)
  DECLARE_U_M(exp, exp)
  DECLARE_U_M(log, log)
  DECLARE_U_M(round, round)
  DECLARE_U_M(random, float_random)
  DECLARE_U_M(trunc, trunc)
  DECLARE_U_M(sqr, float_sqr)
  DECLARE_U_M(sqrt, sqrt)

#undef DECLARE_U_M
#undef DECLARE_M
#undef DECLARE_OP
#undef DECLARE
#undef DECLARE_U

  /// Initialize the Float class.
  void
  float_class_initialize ()
  {
#define DECLARE(Name, Operator)						\
      float_class->slot_set (#Operator,					\
			     new Primitive(float_class_ ## Name))

#define DECLARE_(Name)				\
      DECLARE(Name, Name)

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

      DECLARE_(sin);
      DECLARE_(asin);
      DECLARE_(cos);
      DECLARE_(acos);
      DECLARE_(tan);
      DECLARE_(atan);
      DECLARE_(sgn);
      DECLARE_(abs);
      DECLARE_(exp);
      DECLARE_(log);
      DECLARE_(round);
      DECLARE_(random);
      DECLARE_(trunc);
      DECLARE_(sqr);
      DECLARE_(sqrt);

#undef DECLARE_
#undef DECLARE
  }

}; // namespace object
