/**
 ** \file object/float-class.hh
 ** \brief Definition of the URBI object float.
 */

#ifndef OBJECT_FLOAT_CLASS_HH
# define OBJECT_FLOAT_CLASS_HH

# include "object/fwd.hh"

namespace object
{
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

  /// Initialize the Float class.
  void float_class_initialize ();
}; // namespace object

#endif // !OBJECT_FLOAT_CLASS_HH

