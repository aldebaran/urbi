/**
 ** \file object/primitives.hh
 ** \brief Definition of the root Objects.
 */

#ifndef OBJECT_PRIMITIVES_HH
# define OBJECT_PRIMITIVES_HH

# include "object/fwd.hh"

# include "object/code-class.hh"
# include "object/context-class.hh"
# include "object/float-class.hh"
# include "object/integer-class.hh"
# include "object/list-class.hh"
# include "object/object-class.hh"
# include "object/primitive-class.hh"
# include "object/string-class.hh"

namespace object
{
  void root_classes_initialize ();
}; // namespace object

  /*-------------------.
  | Primitives macros  |
  `-------------------*/

/**
 * Fetch the N-th argument, of type Type. Name it 'arg ## N'.
 */
#define FETCH_ARG(N, Type)                              \
  assert(args[N]->kind_get() ==                         \
         object::Object::kind_type(Type::traits::kind));\
  r ## Type arg ## N = args[N].unsafe_cast<Type>();

/**
 * Define a primitive for class Class named name, which takes one
 * argument of type Type1, returns type Ret and whose result is Call
 * applied to all arguments.
 */
#define PRIMITIVE_1(Class, Name, Call, Ret, Type1)      \
  rObject                                               \
  Class ## _class_ ## Name (objects_type args)          \
  {                                                     \
    FETCH_ARG(0, Type1);                                \
    return new Ret(Call(arg0->value_get()));            \
  }

/**
 * Define a primitive for class Class named name, which takes two
 * arguments of type Type1 and Type2, returns type Ret and whose
 * result is Call applied to all arguments.
 */
#define PRIMITIVE_2(Class, Name, Call, Ret, Type1, Type2)       \
  rObject                                                       \
  Class ## _class_ ## Name (objects_type args)                  \
  {                                                             \
    FETCH_ARG(0, Type1);                                        \
    FETCH_ARG(1, Type2);                                        \
    return new Ret(Call(arg0->value_get(), arg1->value_get())); \
  }

#define PRIMITIVE_OP(Class, Name, Op, Ret, Type1, Type2)        \
  rObject                                                       \
  Class ## _class_ ## Name (objects_type args)                  \
  {                                                             \
    FETCH_ARG(0, Type1);                                        \
    FETCH_ARG(1, Type2);                                        \
    return new Ret(arg0->value_get() Op arg1->value_get());     \
  }

/**
 * Declare a primitive Name in class Class with imlementation Call.
 */
#define DECLARE_PRIMITIVE(Class, Name, Call)            \
  Class ## _class->slot_set (# Name,                    \
    new Primitive(Class ## _class_## Call))

#endif // !OBJECT_PRIMITIVES_HH
