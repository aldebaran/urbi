/**
 ** \file object/fwd.hh
 ** \brief Forward declarations of all node-classes of OBJECT
 ** (needed by the visitors)
 */
#ifndef OBJECT_FWD_HH
# define OBJECT_FWD_HH

# include <libport/fwd.hh>
# include <libport/intrusive-ptr.hh>
# include <libport/reserved-vector.hh>

# include <urbi/export.hh>

namespace object
{
  // rObject & objects_type.
  class Object;

  typedef libport::intrusive_ptr<Object> rObject;
  typedef libport::ReservedVector<rObject, 8> objects_type;

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

# define FWD_DECL(Class)                                \
  class Class;                                          \
  typedef libport::intrusive_ptr<Class> r ## Class         \

  FWD_DECL(Barrier);
  FWD_DECL(Code);
  FWD_DECL(Dictionary);
  FWD_DECL(Directory);
  FWD_DECL(Event);
  FWD_DECL(Executable);
  FWD_DECL(File);
  FWD_DECL(Float);
  FWD_DECL(List);
  FWD_DECL(Lobby);
  FWD_DECL(OutputStream);
  FWD_DECL(Path);
  FWD_DECL(Primitive);
  FWD_DECL(Semaphore);
  FWD_DECL(Server);
  FWD_DECL(Socket);
  FWD_DECL(String);
  FWD_DECL(Tag);
  FWD_DECL(Task);
  FWD_DECL(TextOutputStream);

  extern URBI_SDK_API rObject false_class;
  extern URBI_SDK_API rObject nil_class;
  extern URBI_SDK_API rObject true_class;
  extern URBI_SDK_API rObject void_class;
} // namespace object

#endif // !OBJECT_FWD_HH
