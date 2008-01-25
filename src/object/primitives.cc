/**
 ** \file object/primitives.cc
 ** \brief Creation of the root Objects.
 */

#include "object/object.hh"
#include "object/primitives.hh"
#include "object/atom.hh"

namespace object
{
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

    object_class = new Object;
    // Construct the (empty) objects for the base classes.
    // They all derive from Object.
#define DECLARE(What, Name)			\
      What ## _class = clone(object_class);
    APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(DECLARE);
#undef DECLARE

    // Now that these classes exists, in particular string_class
    // from which any String is a clone, we can initialize the
    // "type" field for all of them, including Object.
#define DECLARE(What, Name)					\
      What ## _class->slot_set("type", new String (#Name));
    APPLY_ON_ALL_PRIMITIVES(DECLARE);
#undef DECLARE

    // Now finalize the construction for each base class:
    // bind some initial methods.
#define DECLARE(What, Name)			\
      What ## _class_initialize ();
    APPLY_ON_ALL_PRIMITIVES(DECLARE);
#undef DECLARE

    // Register all these classes in Object, so that when we look up
    // for "Object" for instance, we find it.
#define DECLARE(What, Name)				\
      object_class->slot_set(#Name, What ## _class);
    APPLY_ON_ALL_PRIMITIVES(DECLARE);
#undef DECLARE

    // Create and register void
    void_class = clone(object_class);
    object_class->slot_set("void", void_class);
    void_class->slot_set("type", new String("void"));

    // Create and register nil
    nil_class = clone (object_class);
    object_class->slot_set("nil", nil_class);
    nil_class->slot_set ("type", new String ("nil"));
  }

  rObject void_class;
  rObject nil_class;
} // namespace object
