#ifndef CXX_PRIMITIVE_HH
# define CXX_PRIMITIVE_HH

# include <object/fwd.hh>
# include <object/object.hh>

namespace object
{
  template<typename M>
  rPrimitive make_primitive(M f, const libport::Symbol& name);
}

/// Make a function that bounces depending on the type of its argument
/**
 *  @param Name Name of the function to generate
 *  @param Msg  Name of the message, to report errors
 *  @param T1   First possible type for the argument
 *  @param V1   Primitive to bounce on when the argument is of type T1
 *  @param T2   Second possible type for the argument
 *  @param V2   Primitive to bounce on when the argument is of type T2
 */
#define OVERLOAD_TYPE(Name, Msg, T1, V1, T2, V2)                        \
                                                                        \
  static rObject Name(runner::Runner& r, object::objects_type args)     \
  {                                                                     \
    static rPrimitive v1 = make_primitive(V1, SYMBOL(Msg));             \
    static rPrimitive v2 = make_primitive(V2, SYMBOL(Msg));             \
                                                                        \
    object::check_arg_count (args.size() - 1, 1);                       \
    if (args[1]->is_a<T2>())                                            \
      return (*v2)(r, args);                                            \
    else                                                                \
      return (*v1)(r, args);                                            \
  }                                                                     \

/// Make a function that bounces depending on its number of arguments
/**
 *  @param Name Name of the function to generate
 *  @param Msg  Name of the message, to report errors
 *  @param N    Base number of arguments
 *  @param V1   Primitive to bounce on for N1     arguments
 *  @param V2   Primitive to bounce on for N1 + 1 arguments
 */
#define OVERLOAD_2(Name, Msg, N, V1, V2)                                \
                                                                        \
  static rObject Name(runner::Runner& r, object::objects_type args)     \
  {                                                                     \
    static rPrimitive v1 = make_primitive(V1, SYMBOL(Msg));             \
    static rPrimitive v2 = make_primitive(V2, SYMBOL(Msg));             \
                                                                        \
    object::check_arg_count (args.size() - 1, N - 1, N);                \
    switch (args.size())                                                \
    {                                                                   \
      case N:                                                           \
        return (*v1)(r, args);                                          \
        break;                                                          \
      case N + 1:                                                       \
        return (*v2)(r, args);                                          \
        break;                                                          \
      default:                                                          \
        pabort("Unreachable");                                          \
    }                                                                   \
  }                                                                     \


#include <object/cxx-primitive.hxx>

#endif
