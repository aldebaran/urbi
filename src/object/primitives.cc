/**
 ** \file object/primitives.cc
 ** \brief Creation of the root Objects.
 */

#include "object/object.hh"
#include "object/atom.hh"

namespace object
{

  rObject object_class;
  rObject float_class;
  rObject integer_class;
  rObject primitive_class;
  rObject string_class;


  /*-------------------.
  | Float primitives.  |
  `-------------------*/

#define DECLARE(Name, Operator)						\
    rObject								\
    float_class_ ## Name (objects_type args)				\
    {									\
      assert(args[0]->kind_get() == Object::kind_float);		\
      assert(args[1]->kind_get() == Object::kind_float);		\
      rFloat l = args[0].unsafe_cast<Float> ();				\
      rFloat r = args[1].unsafe_cast<Float> ();				\
      return new Float(l->value_get () Operator r->value_get());	\
    }

    DECLARE(add, +)
    DECLARE(div, /)
    DECLARE(mul, *)
    DECLARE(sub, -)
#undef DECLARE

  namespace
  {
    /// Initialize the Float class.
    static
    void
    float_class_initialize ()
    {
#define DECLARE(Name, Operator)						\
      float_class->slot_set (#Operator,					\
			     new Primitive(float_class_ ## Name));

      DECLARE(add, +);
      DECLARE(div, /);
      DECLARE(mul, *);
      DECLARE(sub, -);
#undef DECLARE
    }
  }

  namespace
  {
    /// Initialize the Object class.
    static
    void
    object_class_initialize ()
    {
    }

    /// Initialize the Integer class.
    static
    void
    integer_class_initialize ()
    {
    }

    /// Initialize the Primitive class.
    static
    void
    primitive_class_initialize ()
    {
    }

    /// Initialize the Float class.
    static
    void
    string_class_initialize ()
    {
    }

    /// Initialize the root classes.
    /// There are some dependency issues.  For instance, String
    /// is a clone of Object, but Object[type] is a String.
    /// So we need to control the initialization sequence.
    static
    bool
    root_classes_initialize ()
    {
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
#define DECLARE(What, Name)				\
      What ## _class->slot_set("type", new String (#Name));
      APPLY_ON_ALL_PRIMITIVES(DECLARE);
#undef DECLARE

      // Now finalize the construction for each base class:
      // bind some initial methods.
#define DECLARE(What, Name)			\
      What ## _class_initialize ();
      APPLY_ON_ALL_PRIMITIVES(DECLARE);
#undef DECLARE

      return true;
    }

    /// Whether the root classes where initialized.
    // Actually made to run the function root_classes_initialize().
    // Not static so that GCC does not complain that it is unused.
    bool root_classes_initialized = root_classes_initialize();
  }

} // namespace object
