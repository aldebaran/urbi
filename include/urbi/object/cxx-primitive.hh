/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef CXX_PRIMITIVE_HH
# define CXX_PRIMITIVE_HH

# include <libport/format.hh>

# include <urbi/object/fwd.hh>
# include <urbi/object/object.hh>

namespace urbi
{
  namespace object
  {
    template<typename M>
    rPrimitive primitive(M f);

    template<typename M>
    rPrimitive primitive(rPrimitive extend, M f);
  }
}

/// Make a function that bounces depending on the type of its argument
/**
 *  @param Name Name of the function to generate
 *  @param N    Number of arguments expected by the function
 *  @param Arg  Argument whose type to test
 *  @param T1   First possible type for the argument (ignored in fact)
 *  @param V1   Primitive to bounce on when the argument is of type T1
 *  @param T2   Second possible type for the argument
 *  @param V2   Primitive to bounce on when the argument is of type T2
 */
# define OVERLOAD_TYPE(Name, N, Arg, T1, V1, T2, V2)                    \
                                                                        \
  static rObject Name(const object::objects_type& args)                 \
  {                                                                     \
    static rPrimitive v1 = primitive(V1);                               \
    static rPrimitive v2 = primitive(V2);                               \
                                                                        \
    object::check_arg_count (args.size() - 1, N);                       \
    if (args[Arg]->as<T2>())                                            \
      return (*v2)(args);                                               \
    else                                                                \
      return (*v1)(args);                                               \
  }                                                                     \

# define OVERLOAD_TYPE_3(Name, N, Arg, T1, V1, T2, V2, T3, V3)          \
                                                                        \
  static rObject Name(const object::objects_type& args)                 \
  {                                                                     \
    static rPrimitive v1 = primitive(V1);                               \
    static rPrimitive v2 = primitive(V2);                               \
    static rPrimitive v3 = primitive(V3);                               \
                                                                        \
    object::check_arg_count (args.size() - 1, N);                       \
    return (args[Arg]->as<T1>()   ? (*v1)(args)                         \
            : args[Arg]->as<T2>() ? (*v2)(args)                         \
            :                         (*v3)(args));                     \
  }                                                                     \

/// Make a function that bounces depending on its number of arguments
/**
 *  @param Name Name of the function to generate
 *  @param N    Base number of arguments
 *  @param V1   Primitive to bounce on for N1     arguments
 *  @param V2   Primitive to bounce on for N1 + 1 arguments
 */
# define OVERLOAD_2(Name, N, V1, V2)                                    \
                                                                        \
  static rObject Name(const object::objects_type& args)                 \
  {                                                                     \
    static rPrimitive v1 = primitive(V1);                               \
    static rPrimitive v2 = primitive(V2);                               \
                                                                        \
    object::check_arg_count (args.size() - 1, N - 1, N);                \
    switch (args.size())                                                \
    {                                                                   \
      case N:                                                           \
        return (*v1)(args);                                             \
        break;                                                          \
      case N + 1:                                                       \
        return (*v2)(args);                                             \
        break;                                                          \
      default:                                                          \
        pabort("Unreachable");                                          \
    }                                                                   \
  }                                                                     \

/// Make a function that bounces depending on its number of arguments
/**
 *  @param Name Name of the function to generate
 *  @param N    Base number of arguments (without the optional argument)
 *  @param P    Primitive to bounce on
 *  @param V    Default value for argument N + 1
 */
# define OVERLOAD_DEFAULT(Name, N, P, V)                                \
                                                                        \
  static rObject Name(const object::objects_type& args_)                \
  {                                                                     \
    object::objects_type args = args_;                                  \
    static rPrimitive primitive = primitive(P);                         \
    int arity = args.size() - 1;                                        \
                                                                        \
    object::check_arg_count(arity, N, N + 1);                           \
    if (arity == N)                                                     \
      args.push_back(to_urbi(V));                                       \
    return (*primitive)(args);                                          \
  }                                                                     \

/// Raise an Urbi exception.
/**
 *  @param Message
 */
# define RAISE(Message)                         \
  runner::raise_primitive_error(Message)

/// If we happen to meet a preprocessor that does not support varargs,
/// change the uses to include two pairs of parens, and remove those
/// below.
# define FRAISE(...)                            \
  RAISE(libport::format(__VA_ARGS__))

# include <urbi/object/cxx-primitive.hxx>

#endif
