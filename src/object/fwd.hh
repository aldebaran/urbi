/**
 ** \file object/fwd.hh
 ** \brief Forward declarations of all node-classes of OBJECT
 ** (needed by the visitors)
 */
#ifndef OBJECT_FWD_HH
# define OBJECT_FWD_HH

# include <vector>

# include "libport/fwd.hh"
# include "libport/shared-ptr.hh"

namespace runner
{
  class Runner;
}

namespace object
{
  // state.hh
  struct State;

  // rObject & objects_type.
  class Object;
  typedef libport::shared_ptr<Object> rObject;
  typedef std::vector<rObject> objects_type;

  /// \a Macro should be a binary macro whose first arg, \p What, is the
  /// lower case C++ name, and the second argument, \p Name, the
  /// capitalized URBI name.
# define APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT_AND_PRIMITIVE(Macro)	\
  Macro(alien,     Alien)						\
  Macro(call,      Call)						\
  Macro(closure,   Closure)						\
  Macro(code,      Code)						\
  Macro(delegate,  Delegate)                                            \
  Macro(lobby,     Lobby)						\
  Macro(float,     Float)						\
  Macro(integer,   Integer)						\
  Macro(list,      List)						\
  Macro(string,    String)

# define APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(Macro)			\
  Macro(primitive, Primitive)						\
  APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT_AND_PRIMITIVE(Macro)

# define APPLY_ON_ALL_PRIMITIVES(Macro)			\
  Macro(object,    Object)				\
  APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(Macro)

  // All the atoms.
  template <typename Traits>
  class Atom;

  /// Define a primitive as an Atom parametrized with a traits.
# define DEFINE(What, Name)				\
  struct What ## _traits;				\
  typedef Atom< What ## _traits > Name;			\
  typedef libport::shared_ptr < Name > r ## Name;

  /* You have to understand that the primitives are defined here.  For
   * example, a Code is manipulated through a rCode (r = ref counted) and in
   * fact Code is just a typedef for Atom<code_traits>.  If get compilation
   * errors about non existent members, it's most likely because you did
   * obj.get_value () instead of obj->get_value ().  This is a side effect
   * of the operator-> used for the ref-counting.  Keep that in mind. */
  APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT_AND_PRIMITIVE(DEFINE)

  // primitive_type.
  // It is because we need this typedef that we have the
  // previous hideous macro.
  typedef rObject (*primitive_type) (runner::Runner&, objects_type);

  DEFINE(primitive, Primitive)
# undef DEFINE

  // urbi-exception.hh
  class IDelegate;
  class UrbiException;
  class LookupError;
  class RedefinitionError;
  class PrimitiveError;
  class WrongArgumentType;
  class WrongArgumentCount;

  extern rObject void_class;
  extern rObject nil_class;
} // namespace object

#endif // !OBJECT_FWD_HH
