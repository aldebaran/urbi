/**
 ** \file object/fwd.hh
 ** \brief Forward declarations of all node-classes of OBJECT
 ** (needed by the visitors)
 */
#ifndef OBJECT_FWD_HH
# define OBJECT_FWD_HH

# include <deque>

# include <libport/fwd.hh>
# include <libport/shared-ptr.hh>

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
  // FIXME: we probably want libport::refcouned smart pointers here
  typedef libport::shared_ptr<rObject, false> rrObject;
  typedef std::deque<rObject> objects_type;

  /// \a Macro should be a binary macro whose first arg, \p What, is
  /// the lower case C++ name, and the second argument, \p Name, the
  /// capitalized URBI name.  This is used in many different contexts,
  /// such as defining enums, so we do not use terminators here (;
  /// etc.): Macro must do it.
# define APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(Macro)	\
  Macro(alien,      Alien)				\
  Macro(delegate,   Delegate)				\
  Macro(float,      Float)				\
  Macro(integer,    Integer)				\
  Macro(list,       List)				\
  Macro(lobby,      Lobby)				\
  Macro(primitive,  Primitive)				\
  Macro(semaphore,  Semaphore)


# define APPLY_ON_ALL_PRIMITIVES(Macro)			\
  APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(Macro)		\
  Macro(object,     Object)


# define APPLY_ON_ALL_ROOT_CLASSES_BUT_OBJECT(Macro)		\
  APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(Macro)			\
  Macro(global,     Global)					\
  Macro(tag,        Tag)					\
  Macro(task,       Task)


# define APPLY_ON_ALL_ROOT_CLASSES(Macro)	\
  APPLY_ON_ALL_ROOT_CLASSES_BUT_OBJECT(Macro)	\
  Macro(object, Object)

  /*
    Help the generation of precompiled symbols.

    SYMBOL(Alien)
    SYMBOL(Call)
    SYMBOL(Code)
    SYMBOL(Delegate)
    SYMBOL(Float)
    SYMBOL(Integer)
    SYMBOL(List)
    SYMBOL(Lobby)
    SYMBOL(Object)
    SYMBOL(Primitive)
    SYMBOL(String)
    SYMBOL(Task)

  */

  // All the atoms.
  template <typename Traits>
  class Atom;

  /// Define a primitive as an Atom parametrized with a traits.
# define DEFINE(What, Name)				\
  struct What ## _traits;				\
  typedef Atom< What ## _traits > Name;			\
  typedef libport::shared_ptr < Name, true > r ## Name;

  /* You have to understand that the primitives are defined here.  For
   * example, a Code is manipulated through a rCode (r = ref counted) and in
   * fact Code is just a typedef for Atom<code_traits>.  If you get compilation
   * errors about non existent members, it's most likely because you did
   * obj.get_value () instead of obj->get_value ().  This is a side effect
   * of the operator-> used for the ref-counting.  Keep that in mind. */
  APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(DEFINE)
# undef DEFINE

  class Code;
  typedef libport::shared_ptr<Code> rCode;
  class Dictionary;
  typedef libport::shared_ptr<Dictionary> rDictionary;
  class String;
  typedef libport::shared_ptr<String> rString;

  // urbi-exception.hh
  class IDelegate;
  class UrbiException;
  class LookupError;
  class RedefinitionError;
  class PrimitiveError;
  class WrongArgumentType;
  class WrongArgumentCount;

  extern rObject false_class;
  extern rObject nil_class;
  extern rObject task_class;
  extern rObject true_class;
  extern rObject void_class;
} // namespace object

#endif // !OBJECT_FWD_HH
