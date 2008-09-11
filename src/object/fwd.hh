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

# define APPLY_ON_ALL_PRIMITIVES(Macro)			\
  APPLY_ON_ALL_PRIMITIVES_BUT_OBJECT(Macro)		\
  Macro(object,     Object)


# define APPLY_ON_ALL_ROOT_CLASSES_BUT_OBJECT(Macro)		\
  Macro(global,     Global)


# define APPLY_ON_ALL_ROOT_CLASSES(Macro)	\
  APPLY_ON_ALL_ROOT_CLASSES_BUT_OBJECT(Macro)   \
  Macro(object, Object)

  /*
    Help the generation of precompiled symbols.

    SYMBOL(Call)
    SYMBOL(Code)
    SYMBOL(Float)
    SYMBOL(Integer)
    SYMBOL(List)
    SYMBOL(Lobby)
    SYMBOL(Object)
    SYMBOL(Primitive)
    SYMBOL(String)
    SYMBOL(Task)

  */

#define FWD_DECL(Class)                                 \
  class Class;                                          \
  typedef libport::shared_ptr<Class> r ## Class         \

  FWD_DECL(Barrier);
  FWD_DECL(Code);
  FWD_DECL(Dictionary);
  FWD_DECL(Directory);
  FWD_DECL(Event);
  FWD_DECL(File);
  FWD_DECL(Float);
  FWD_DECL(List);
  FWD_DECL(Lobby);
  FWD_DECL(Path);
  FWD_DECL(Primitive);
  FWD_DECL(Semaphore);
  FWD_DECL(String);
  FWD_DECL(Tag);
  FWD_DECL(Task);

  // urbi-exception.hh
  class UrbiException;
  class LookupError;
  class RedefinitionError;
  class PrimitiveError;
  class WrongArgumentType;
  class WrongArgumentCount;

  extern rObject false_class;
  extern rObject nil_class;
  extern rObject true_class;
  extern rObject void_class;
} // namespace object

#endif // !OBJECT_FWD_HH
