/**
 ** \file object/object-kind.hh
 ** \brief Definition of object::object_kind_type.
 */

#ifndef OBJECT_OBJECT_KIND_HH
# define OBJECT_OBJECT_KIND_HH

# include "object/fwd.hh"

namespace object
{

  /// The kinds of primitive objects.
  enum object_kind_type
  {
#define CASE(What, Name) object_kind_ ## What,
    APPLY_ON_ALL_PRIMITIVES(CASE)
#undef CASE
  };

  /// Return the kind as a string.  Used by dump.
  const char* string_of (object_kind_type k);

} // namespace object

#endif // !OBJECT_OBJECT_KIND_HH
