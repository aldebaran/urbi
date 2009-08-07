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

# define APPLY_ON_ALL_PRIMITIVES(Macro)         \
    Macro(Barrier);                             \
    Macro(Code);                                \
    Macro(Dictionary);                          \
    Macro(Directory);                           \
    Macro(Executable);                          \
    Macro(File);                                \
    Macro(Finalizable);                         \
    Macro(Float);                               \
    Macro(FormatInfo);                          \
    Macro(Formatter);                           \
    Macro(InputStream);                         \
    Macro(List);                                \
    Macro(Lobby);                               \
    Macro(OutputStream);                        \
    Macro(Path);                                \
    Macro(Primitive);                           \
    Macro(Process);                             \
    Macro(Semaphore);                           \
    Macro(Server);                              \
    Macro(Socket);                              \
    Macro(String);                              \
    Macro(Tag);                                 \
    Macro(Task);                                \
    Macro(TextOutputStream)                     \

# define APPLY_ON_ALL_OBJECTS(Macro)            \
  APPLY_ON_ALL_PRIMITIVES(Macro);               \
  Macro(Event);                                 \


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

  APPLY_ON_ALL_OBJECTS(FWD_DECL);

  extern URBI_SDK_API rObject false_class;
  extern URBI_SDK_API rObject nil_class;
  extern URBI_SDK_API rObject true_class;
  extern URBI_SDK_API rObject void_class;
} // namespace object

#endif // !OBJECT_FWD_HH
