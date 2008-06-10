/**
 ** \file object/semaphore-class.hh
 ** \brief Definition of the URBI object semaphore.
 */

#ifndef OBJECT_SEMAPHORE_CLASS_HH
# define OBJECT_SEMAPHORE_CLASS_HH

# include <object/fwd.hh>
# include <scheduler/fwd.hh>

namespace object
{
  extern rObject semaphore_class;

  /// Initialize the Semaphore class.
  void semaphore_class_initialize();

}; // namespace object

#endif // OBJECT_SEMAPHORE_CLASS_HH
