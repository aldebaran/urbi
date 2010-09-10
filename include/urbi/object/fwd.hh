/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

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

namespace urbi
{
  namespace object
  {
    // rObject & objects_type.
    class Object;

    typedef libport::intrusive_ptr<Object> rObject;
    typedef libport::ReservedVector<rObject, 8> objects_type;

# define APPLY_ON_ALL_PRIMITIVES(Macro)         \
    Macro(Barrier);                             \
    Macro(Code);                                \
    Macro(Date);                                \
    Macro(Dictionary);                          \
    Macro(Directory);                           \
    Macro(Duration);                            \
    Macro(Event);                               \
    Macro(Executable);                          \
    Macro(File);                                \
    Macro(Finalizable);                         \
    Macro(Float);                               \
    Macro(FormatInfo);                          \
    Macro(Formatter);                           \
    Macro(InputStream);                         \
    Macro(IoService);				\
    Macro(Job);                                 \
    Macro(List);                                \
    Macro(Lobby);                               \
    Macro(Location);                            \
    Macro(OutputStream);                        \
    Macro(Path);                                \
    Macro(Position);                            \
    Macro(Primitive);                           \
    Macro(Process);                             \
    Macro(Regexp);                              \
    Macro(Semaphore);                           \
    Macro(Server);                              \
    Macro(Socket);                              \
    Macro(Stream);                              \
    Macro(String);                              \
    Macro(Tag);                                 \
    Macro(TextOutputStream);                    \
    Macro(UValue);                              \
    Macro(UVar);

# define APPLY_ON_ALL_OBJECTS(Macro)            \
    APPLY_ON_ALL_PRIMITIVES(Macro);             \
    Macro(Event);                               \


    /*
      Help the generation of precompiled symbols.

      SYMBOL(Call)
      SYMBOL(Code)
      SYMBOL(Float)
      SYMBOL(Integer)
      SYMBOL(Job)
      SYMBOL(List)
      SYMBOL(Lobby)
      SYMBOL(Object)
      SYMBOL(Primitive)
      SYMBOL(String)

    */

# define FWD_DECL(Class)                                \
    class Class;                                        \
    typedef libport::intrusive_ptr<Class> r ## Class    \

    APPLY_ON_ALL_OBJECTS(FWD_DECL);

    extern URBI_SDK_API rObject false_class;
    extern URBI_SDK_API rObject nil_class;
    extern URBI_SDK_API rObject true_class;
    extern URBI_SDK_API rObject void_class;
  } // namespace object
}

namespace object
{
  // Temporary merge until everything is moved in the urbi namespace.
  using namespace urbi::object;
}

#endif // !OBJECT_FWD_HH
