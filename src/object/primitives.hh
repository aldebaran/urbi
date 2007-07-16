/**
 ** \file object/primitives.hh
 ** \brief Definition of the root Objects.
 */

#ifndef OBJECT_PRIMITIVES_HH
# define OBJECT_PRIMITIVES_HH

# include "object/fwd.hh"

namespace object
{

  extern rObject object_class;
  extern rObject connection_class;

  /*--------.
  | Float.  |
  `--------*/

  extern rObject float_class;

  // FIXME: Do we really want to declare them?  They are only
  // for sake of the test suite.
# define DECLARE(Name, Op)				\
  rObject float_class_ ## Name (objects_type args);

  DECLARE(add, +)
  DECLARE(div, /)
  DECLARE(mul, *)
  DECLARE(sub, -)

# undef DECLARE

  /*----------.
  | Integer.  |
  `----------*/

  extern rObject integer_class;

  /*------------.
  | Primitive.  |
  `------------*/

  extern rObject primitive_class;

  /*---------.
  | String.  |
  `---------*/

  extern rObject string_class;

} // namespace object

#endif // !OBJECT_PRIMITIVES_HH
