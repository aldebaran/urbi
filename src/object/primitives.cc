/**
 ** \file object/primitives.cc
 ** \brief Creation of the root Objects.
 */

#include "object/object.hh"
#include "object/primitives.hh"
#include "object/atom.hh"

namespace object
{

  // The initialization routines are useless, but allow a more
  // systematic initialization of all the base classes.
#define CLASS_INITIALIZE(What)					\
  rObject What ## _class;					\
  namespace { static void What ## _class_initialize () { } }

  CLASS_INITIALIZE(nil);
  CLASS_INITIALIZE(void);
#undef CLASS_INITIALIZE

  /*------------------------.
  | Global initialization.  |
  `------------------------*/

  /// Whether the root classes where initialized.
  bool root_classes_initialized = false;

  /// Initialize the root classes.
  /// There are some dependency issues.  For instance, String
  /// is a clone of Object, but Object[type] is a String.
  /// So we need to control the initialization sequence.
  void
  root_classes_initialize ()
  {
    if (root_classes_initialized)
      return;
    root_classes_initialized = true;

    // The construction of the primitive classes goes in several
    // steps.
    //
    // 1. Construct the (empty) objects for the base classes.  They
    // all derive from Object.
    //
    // 2. Now that these classes exists, in particular string_class
    // from which any String is a clone, we can initialize the "type"
    // field for all of them, including Object.
    //
    // 3. Now finalize the construction for each base class: bind some
    // initial methods.
    //
    // 4. Register all these classes in Object, so that when we look
    // up for "Object" for instance, we find it.
    //
    // CLASS_CREATE does 1, CLASS_INIT does 2 to 4, and CLASS_SETUP
    // runs 1 to 4.

#define CLASS_CREATE(What, Name)		\
    What ## _class = clone(object_class);

#define CLASS_INIT(What, Name)					\
    What ## _class->slot_set(SYMBOL(type),			\
			     new String (SYMBOL(Name)));	\
    What ## _class_initialize ();				\
    object_class->slot_set(symbol_ ## Name, What ## _class);

#define CLASS_SETUP(What, Name)			\
    CLASS_CREATE(What, Name)			\
    CLASS_INIT(What, Name)

    // Object is a special case: it is not built as a clone of itself.
    object_class = new Object;
    // The other primitives.  Because primitive initialization depend
    // a lot on one another (e.g., String is used everywhere for slot
    // names, and Primitive is used for... all the primitive methods
    // in the primitive classes), first create them all, then bind
    // them all.
    APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(CLASS_CREATE);
    APPLY_ON_ALL_PRIMITIVES(CLASS_INIT);

    CLASS_SETUP(nil, nil);
    CLASS_SETUP(void, void);
  }

} // namespace object
