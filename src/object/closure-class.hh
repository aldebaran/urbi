/**
 ** \file object/closure-class.hh
 ** \brief Definition of the URBI closure object.
 */

#ifndef OBJECT_CLOSURE_CLASS_HH
# define OBJECT_CLOSURE_CLASS_HH

# include "object/fwd.hh"

namespace object
{
  extern rObject closure_class;

  /// Initialize the Closure class.
  void closure_class_initialize ();
}; // namespace object

#endif // !OBJECT_CLOSURE_CLASS_HH
