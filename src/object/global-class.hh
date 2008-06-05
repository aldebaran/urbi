/**
 ** \file object/global-class.hh
 ** \brief Definition of the URBI object global.
 */

#ifndef OBJECT_GLOBAL_CLASS_HH
# define OBJECT_GLOBAL_CLASS_HH

# include <object/fwd.hh>

namespace object
{
  /// The prototype for Global objects.
  extern rObject global_class;

  /// Initialize the Global class.
  void global_class_initialize ();
}; // namespace object

#endif // !OBJECT_GLOBAL_CLASS_HH
