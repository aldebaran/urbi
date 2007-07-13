/**
 ** \file object/primitives.cc
 ** \brief Creation of the root Objects.
 */

#include "object/object.hh"
#include "object/atom.hh"

namespace object
{

  rObject object_class;
  rObject string_class;
  rObject float_class;
  rObject integer_class;

  namespace
  {
    /// Create the Object class.
    static
    rObject
    new_object_class ()
    {
      // It has no parents, and for the time being, no contents.
      return new Object;
    }

    /// Create the Float class.
    static
    rObject
    new_float_class (rObject object_class)
    {
      rObject res = clone(object_class);
      return res;
    }

    /// Create the Integer class.
    static
    rObject
    new_integer_class (rObject object_class)
    {
      rObject res = clone(object_class);
      return res;
    }

    /// Create the Float class.
    static
    rObject
    new_string_class (rObject object_class)
    {
      rObject res = clone(object_class);
      return res;
    }
    
    /// Initialize the root classes.
    /// There are some dependency issues.  For instance, String
    /// is a clone of Object, but Object[type] is a String.
    /// So we need to control the initialization sequence.
    static
    bool
    root_classes_initialize ()
    {
      object_class = new_object_class();
      string_class = new_string_class(object_class);
      float_class = new_float_class(object_class);
      integer_class = new_integer_class(object_class);

      // Now that these classes exists, in particular string_class
      // from which any String is a clone, we can initialize the
      // "type" field for all of them.
#define DECLARE(What, Name)      \
      (*What ## _class) ["type"] = new String (#Name);

  DECLARE(float, Float);
  DECLARE(integer, Integer);
  DECLARE(string, String);

#undef DECLARE
      return true;
    }

    /// Whether the root classes where initialized.
    // Actually made to run the function root_classes_initialize().
    // Not static so that GCC does not complain that it is unused.
    bool root_classes_initialized = root_classes_initialize();
  }


} // namespace object
