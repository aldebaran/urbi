/**
 ** \file object/primitives.hh
 ** \brief Definition of the root Objects.
 */

#ifndef OBJECT_PRIMITIVES_HH
# define OBJECT_PRIMITIVES_HH

# include "libport/compiler.hh"

# include "object/fwd.hh"
# include "object/symbols.hh"

# include "object/alien-class.hh"
# include "object/call-class.hh"
# include "object/code-class.hh"
# include "object/delegate-class.hh"
# include "object/lobby-class.hh"
# include "object/float-class.hh"
# include "object/integer-class.hh"
# include "object/list-class.hh"
# include "object/object-class.hh"
# include "object/primitive-class.hh"
# include "object/string-class.hh"
# include "object/task-class.hh"
# include "object/urbi-exception.hh"

namespace object
{
  void root_classes_initialize ();
}; // namespace object

  /*-------------------.
  | Primitives macros  |
  `-------------------*/

/**
 * Throw an exception if \a Obj is not of type \a Type.
 */
#define TYPE_CHECK(Obj, Type)                                           \
  do {									\
    if (!(Obj)->type_is<Type>())					\
      throw object::WrongArgumentType(object::Object::kind_type(Type::kind), \
				      Obj->kind_get(),			\
				      "");				\
  } while (0)

/**
 * Fetch the N-th argument, of type Type. Name it 'arg ## N'.
 * If the argument's type is wrong, throw an UrbiException.
 */
#define FETCH_ARG(N, Type)				\
  TYPE_CHECK(args[N], Type);				\
  r ## Type arg ## N = args[N].unsafe_cast<Type>()

/**
 * Check argument's count and throw an Urbi exception if it is wrong.
 * \param N expected number of arguments (including self).
 */
#define CHECK_ARG_COUNT(N)                              \
  check_arg_count(N, args.size(), __PRETTY_FUNCTION__)

/**
 * Check argument count and throw an Urbi exception if it does not
 * fall into the expected \param MIN - \param MAX range, including self.
 */
#define CHECK_ARG_COUNT_RANGE(MIN, MAX)				\
check_arg_count(MIN, MAX, args.size(), __PRETTY_FUNCTION__)

/**
 * Define a primitive for class Class named name, which takes one
 * argument of type Type1, returns type Ret and whose result is Call
 * applied to all arguments.
 */
#define PRIMITIVE_1_(Class, Name, Call, Ret, Type1, Get)        \
  static rObject						\
  Class ## _class_ ## Name (runner::Runner&, objects_type args)	\
  {                                                             \
    CHECK_ARG_COUNT(1);                                         \
    FETCH_ARG(0, Type1);                                        \
    return Ret(Call(arg0 Get));                                 \
  }

#define PRIMITIVE_1(Class, Name, Call, Type1)                   \
  PRIMITIVE_1_(Class, Name, Call, , Type1, )

#define PRIMITIVE_1_V(Class, Name, Call, Ret, Type1)            \
  PRIMITIVE_1_(Class, Name, Call, new Ret, Type1, ->value_get())


/**
 * Define a primitive for class \a Class named name, which takes two
 * arguments of type \a Type1 and \a Type2, returns type \a Ret and whose
 * result is \a Call applied to all arguments.
 */
#define PRIMITIVE_2_(Class, Name, Call, Ret, Type1, Type2, Get) \
  static rObject						\
  Class ## _class_ ## Name (runner::Runner&, objects_type args)	\
  {                                                             \
    CHECK_ARG_COUNT(2);                                         \
    FETCH_ARG(0, Type1);                                        \
    FETCH_ARG(1, Type2);                                        \
    return Ret(Call(arg0 Get, arg1 Get));                       \
  }

#define PRIMITIVE_2(Class, Name, Call, Type1, Type2)            \
  PRIMITIVE_2_(Class, Name, Call, , Type1, Type2, )

#define PRIMITIVE_2_V(Class, Name, Call, Ret, Type1, Type2)     \
  PRIMITIVE_2_(Class, Name, Call, new Ret, Type1, Type2, ->value_get())

/**
 * Define a primitive for class Class named Name, which takes two
 * arguments of type Type1 and rObject, returns type Ret and whose
 * result is Call applied to all arguments.
 */
#define PRIMITIVE_2_OBJECT(Class, Name, Call, Type1)            \
  static rObject						\
  Class ## _class_ ## Name (runner::Runner&, objects_type args)	\
  {                                                             \
    CHECK_ARG_COUNT(2);                                         \
    FETCH_ARG(0, Type1);                                        \
    rObject arg1 = args[1];                                     \
    return (Call(arg0, arg1));                                  \
  }

/**
 * Define an operator-primitive. @see PRIMITIVE_2_.
 */
#define PRIMITIVE_OP_(Class, Name, Op, Ret, Type1, Type2, Get)  \
  static rObject						\
  Class ## _class_ ## Name (runner::Runner&, objects_type args)	\
  {                                                             \
    CHECK_ARG_COUNT(2);                                         \
    FETCH_ARG(0, Type1);                                        \
    FETCH_ARG(1, Type2);                                        \
    return Ret(arg0 Get Op arg1 Get);				\
  }

#define PRIMITIVE_OP(Class, Name, Op, Type1, Type2)             \
  PRIMITIVE_OP_(Class, Name, Op, , Type1, Type2, )

#define PRIMITIVE_OP_V(Class, Name, Op, Ret, Type1, Type2)      \
  PRIMITIVE_OP_(Class, Name, Op, new Ret, Type1, Type2, ->value_get())

/**
 * Declare a primitive Name in class Class with C++ implementation Call.
 */
#define DECLARE_PRIMITIVE(Class, Name)  				\
  Class ## _class->slot_set (object::symbol_##Name,			\
			     new Primitive(Class ## _class_ ## Name))

#endif // !OBJECT_PRIMITIVES_HH
