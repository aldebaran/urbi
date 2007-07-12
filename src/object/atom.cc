/**
 ** \file object/atom.cc
 ** \brief Definition of object::Float, object::Integer, object::String.
 */

#include "object/atom.hh"

namespace object
{

  /*--------.
  | Float.  |
  `--------*/

#define DECLARE(Name, Operator)					\
  rFloat							\
  float_class_ ## Name (rObject lhs, rObject rhs)		\
  {								\
    assert(lhs->kind_get() == Object::kind_float);		\
    assert(rhs->kind_get() == Object::kind_float);		\
    rFloat l = lhs.unsafe_cast<Float> ();			\
    rFloat r = rhs.unsafe_cast<Float> ();			\
    return new Float(l->value_get () Operator r->value_get());	\
  }

  DECLARE(add, +)
  DECLARE(div, /)
  DECLARE(mul, *)
  DECLARE(sub, -)

#undef DECLARE

  /*----------.
  | Integer.  |
  `----------*/

  /*---------.
  | String.  |
  `---------*/

} // namespace object
