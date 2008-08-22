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

  class Barrier;
  typedef libport::shared_ptr<Barrier> rBarrier;
  class Code;
  typedef libport::shared_ptr<Code> rRoutine;
  class Dictionary;
  typedef libport::shared_ptr<Dictionary> rDictionary;
  class Event;
  typedef libport::shared_ptr<Event> rEvent;
  class Float;
  typedef libport::shared_ptr<Float> rFloat;
  class List;
  typedef libport::shared_ptr<List> rList;
  class Lobby;
  typedef libport::shared_ptr<Lobby> rLobby;
  class Primitive;
  typedef libport::shared_ptr<Primitive> rPrimitive;
  class Semaphore;
  typedef libport::shared_ptr<Semaphore> rSemaphore;
  class String;
  typedef libport::shared_ptr<String> rString;
  class Tag;
  typedef libport::shared_ptr<Tag> rTag;
  class Task;
  typedef libport::shared_ptr<Task> rTask;

  // urbi-exception.hh
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
