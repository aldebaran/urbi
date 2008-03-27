/**
 ** \file object/globals-class.hh
 ** \brief Definition of the URBI object globals.
 */

#ifndef OBJECT_GLOBALS_CLASS_HH
# define OBJECT_GLOBALS_CLASS_HH

# include "object/fwd.hh"

namespace object
{
  /// The prototype for Globals objects.
  extern rObject globals_class;

  /// Initialize the Globals class.
  void globals_class_initialize ();
}; // namespace object

#endif // !OBJECT_GLOBALS_CLASS_HH
