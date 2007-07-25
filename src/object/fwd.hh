/**
 ** \file object/fwd.hh
 ** \brief Forward declarations of all node-classes of OBJECT
 ** (needed by the visitors)
 */
#ifndef OBJECT_FWD_HH
# define OBJECT_FWD_HH

# include "libport/fwd.hh"
# include "libport/shared-ptr.hh"

namespace ast
{
  class Exp;
}

namespace object
{

  // rObject & objects_type.
  class Object;
  typedef libport::shared_ptr<Object> rObject;
  typedef std::vector<rObject> objects_type;

  // primitive_type.
  typedef rObject (*primitive_type) (objects_type);

  /// Macro should be a binary macro whose first arg, \p What, is the
  /// lower case C++ name, and the second argument, \p Name, the
  /// capitalized Urbi name.
# define APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(Macro)	\
  Macro(code,      Code)				\
  Macro(context,   Context)				\
  Macro(float,     Float)				\
  Macro(integer,   Integer)				\
  Macro(primitive, Primitive)				\
  Macro(string,    String)

# define APPLY_ON_ALL_PRIMITIVES(Macro)			\
  Macro(object,    Object)				\
  APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(Macro)


  // All the atoms.
  template <typename Traits>
  class Atom;

# define DECLARE(What, Name)				\
  struct What ## _traits;				\
  typedef Atom< What ## _traits > Name;			\
  typedef libport::shared_ptr < Name > r ## Name;

  APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(DECLARE)
# undef DECLARE

} // namespace object

#endif // !OBJECT_FWD_HH
