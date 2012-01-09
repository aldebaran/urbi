/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
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
# include <libport/vector.hh>

# include <urbi/export.hh>

namespace urbi
{
  namespace object
  {
    // rObject & objects_type.
    class Object;

    typedef libport::intrusive_ptr<Object> rObject;
    static const unsigned objects_type_floor = 8;
    typedef libport::FlooredAllocator<rObject, objects_type_floor>
      objects_type_allocator;
    typedef libport::Constructor<rObject>
      objects_type_constructor;
    typedef libport::FlooredExponentialCapacity<objects_type_floor>
      objects_type_capacity;
    typedef libport::Vector<rObject,
                            objects_type_allocator,
                            objects_type_constructor,
                            objects_type_capacity> objects_type;

# define APPLY_ON_ALL_OBJECTS(Macro)            \
    Macro(Barrier);                             \
    Macro(Code);                                \
    Macro(Date);                                \
    Macro(Dictionary);                          \
    Macro(Directory);                           \
    Macro(Duration);                            \
    Macro(Event);                               \
    Macro(EventHandler);                        \
    Macro(Executable);                          \
    Macro(File);                                \
    Macro(Finalizable);                         \
    Macro(Float);                               \
    Macro(FormatInfo);                          \
    Macro(Formatter);                           \
    Macro(FunctionProfile);                     \
    Macro(Hash);                                \
    Macro(InputStream);                         \
    Macro(IoService);				\
    Macro(Job);                                 \
    Macro(List);                                \
    Macro(Lobby);                               \
    Macro(Location);                            \
    Macro(Logger);                              \
    Macro(OutputStream);                        \
    Macro(Path);                                \
    Macro(Position);                            \
    Macro(Primitive);                           \
    Macro(Process);                             \
    Macro(Profile);                             \
    Macro(Regexp);                              \
    Macro(Semaphore);                           \
    Macro(Server);                              \
    Macro(Socket);                              \
    Macro(Stream);                              \
    Macro(String);                              \
    Macro(Tag);                                 \
    Macro(UConnection);                         \
    Macro(UValue);                              \
    Macro(UVar);

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
# undef FWD_DECL

    extern URBI_SDK_API rObject false_class;
    extern URBI_SDK_API rObject nil_class;
    extern URBI_SDK_API rObject true_class;
    extern URBI_SDK_API rObject void_class;

    template <typename T>
    rObject to_urbi(const T&);

    // From cxx-primitive.
    template<typename M>
    rPrimitive primitive(M f);

    template<typename M>
    rPrimitive primitive(rPrimitive extend, M f);

    /// Stack of Urbi tags, to control execution.
    typedef std::vector<object::rTag> tag_stack_type;

  } // namespace object
}

namespace object
{
  // Temporary merge until everything is moved in the urbi namespace.
  using namespace urbi::object;
}

#endif // !OBJECT_FWD_HH
