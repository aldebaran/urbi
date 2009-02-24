/**
 ** \file object/primitives.hh
 ** \brief Definition of the root Objects.
 */

#ifndef OBJECT_PRIMITIVES_HH
# define OBJECT_PRIMITIVES_HH

# include <object/object.hh>
# include <object/symbols.hh>

/**
 * Declare a primitive \a Name in class \a Class with C++
 * implementation \a Name.
 */
#define DECLARE_PRIMITIVE(Class, Name)					\
  Class ## _class->slot_set (SYMBOL(Name),                              \
                             new Primitive(Class ## _class_ ## Name),   \
                             true)

#endif // !OBJECT_PRIMITIVES_HH
