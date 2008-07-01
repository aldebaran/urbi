#include <boost/bind.hpp>

#include <object/any-to-boost-function.hh>
#include <object/cxx-conversions.hh>
#include <object/cxx-helper.hh>
#include <object/cxx-primitive.hh>

namespace object
{
  template <typename M>
  struct MakePrimitive
  {};

# define BOOST_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3) \
  boost::function##ArgsC                        \
  <IF(Ret, R, void),                            \
   S                                            \
   COMMA(Run)  WHEN(Run, runner::Runner&)       \
   COMMA(Arg1) WHEN(Arg1, A1)                   \
   COMMA(Arg2) WHEN(Arg2, A2)                   \
   COMMA(Arg3) WHEN(Arg3, A3)                   \
   >                                            \

# define PRIMITIVE(Ret, ArgsC, Run, Arg1, Arg2, Arg3)                   \
  template                                                              \
  <WHEN(Ret, typename R) COMMA(Ret)                                     \
   typename S                                                           \
   COMMA(Arg1) WHEN(Arg1, typename A1)                                  \
    COMMA(Arg2) WHEN(Arg2, typename A2)                                 \
    COMMA(Arg3) WHEN(Arg3, typename A3)                                 \
    >                                                                   \
  struct MakePrimitive<BOOST_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3)>   \
  {                                                                     \
    static rObject primitive(                                           \
      runner::Runner& WHEN(Run, r),                                     \
      object::objects_type args,                                        \
      BOOST_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3) f,                  \
      const libport::Symbol& name)                                      \
    {                                                                   \
      WHEN(Ret, return) f(                                              \
        CxxConvert<S>::to(args[0], name)                                \
        COMMA(Run) WHEN(Run, r)                                         \
        COMMA(Arg1) WHEN(Arg1, CxxConvert<A1>::to(args[1], name))       \
        COMMA(Arg2) WHEN(Arg2, CxxConvert<A2>::to(args[2], name))       \
        COMMA(Arg3) WHEN(Arg3, CxxConvert<A3>::to(args[3], name))       \
        );                                                              \
      return object::void_class;                                        \
                                                                        \
    }                                                                   \
  };                                                                    \

# define BOOST_LIST_TYPE_MET(Ret, ArgsC, Run)   \
  boost::function##ArgsC                        \
  <IF(Ret, R, void),                            \
   S,                                           \
   WHEN(Run, runner::Runner&) COMMA(Run)        \
   object::objects_type                         \
   >                                            \

# define BOOST_LIST_TYPE(Ret, ArgsC, Run)       \
  boost::function##ArgsC                        \
  <IF(Ret, R, void),                            \
   WHEN(Run, runner::Runner&) COMMA(Run)        \
   object::objects_type                         \
   >                                            \

# define PRIMITIVE_LIST_MET(Ret, ArgsC, Run)                    \
  template <WHEN(Ret, typename R) COMMA(Ret) typename S>        \
  struct MakePrimitive<BOOST_LIST_TYPE_MET(Ret, ArgsC, Run)>    \
  {                                                             \
    static rObject primitive(                                   \
      runner::Runner& WHEN(Run, r),                             \
      object::objects_type args,                                \
      BOOST_LIST_TYPE_MET(Ret, ArgsC, Run) f,                   \
      const libport::Symbol& name)                              \
    {                                                           \
      S tgt =                                                   \
        CxxConvert<S>::to(args[0], name);                       \
      args.pop_front();                                         \
      WHEN(Ret, return) f(                                      \
        tgt,                                                    \
        WHEN(Run, r) COMMA(Run)                                 \
        args                                                    \
        );                                                      \
      return object::void_class;                                \
                                                                \
    }                                                           \
  };                                                            \

# define PRIMITIVE_LIST(Ret, ArgsC, Run)                        \
  template <WHEN(Ret, typename R)>                              \
  struct MakePrimitive<BOOST_LIST_TYPE(Ret, ArgsC, Run)>        \
  {                                                             \
    static rObject primitive(                                   \
      runner::Runner& WHEN(Run, r),                             \
      object::objects_type args,                                \
      BOOST_LIST_TYPE(Ret, ArgsC, Run) f,                       \
      const libport::Symbol&)                                   \
    {                                                           \
      WHEN(Ret, return) f(                                      \
        WHEN(Run, r) COMMA(Run)                                 \
        args                                                    \
        );                                                      \
      return object::void_class;                                \
                                                                \
    }                                                           \
  };                                                            \

  PRIMITIVE_LIST_MET(true, 2, false);
  PRIMITIVE_LIST_MET(true, 3, true);
  PRIMITIVE_LIST_MET(false, 2, false);
  PRIMITIVE_LIST_MET(false, 3, true);
  PRIMITIVE_LIST(true, 1, false);
  PRIMITIVE_LIST(true, 2, true);
  PRIMITIVE_LIST(false, 1, false);
  PRIMITIVE_LIST(false, 2, true);
  ALL_PRIMITIVE(PRIMITIVE);

# undef PRIMITIVE
# undef BOOST_TYPE
# undef PRIMITIVE_LIST
# undef BOOST_LIST_TYPE

  template<typename M>
  inline rPrimitive
  make_primitive(M f, const libport::Symbol& name)
  {
    typedef AnyToBoostFunction<M> C;
    // If primitive is unfound in MakePrimitive here, you gave an
    // unsupported type to make Primitive. AnyToBoostFunction must be
    // able to convert the given values. It handles:
    // * boost::functions
    // * function pointers
    // * method pointers
    return new Primitive(
      boost::bind(MakePrimitive<typename C::type>::primitive,
                  _1, _2, C::convert(f), name));
  }
}
