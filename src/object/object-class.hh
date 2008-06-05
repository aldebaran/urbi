/**
 ** \file object/object-class.hh
 ** \brief Definition of the URBI object object.
 */

#ifndef OBJECT_OBJECT_CLASS_HH
# define OBJECT_OBJECT_CLASS_HH

# include <object/fwd.hh>

namespace object
{
  extern rObject object_class;

  /// Initialize the Object class.
  void object_class_initialize ();
}; // namespace object

#endif // !OBJECT_OBJECT_CLASS_HH
