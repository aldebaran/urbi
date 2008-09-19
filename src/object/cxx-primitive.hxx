#include <boost/bind.hpp>
#include <boost/tr1/type_traits.hpp>

#include <object/any-to-boost-function.hh>
#include <object/cxx-conversions.hh>
#include <object/cxx-helper.hh>
#include <object/cxx-primitive.hh>

namespace object
{
  template <typename M>
  struct MakePrimitive
  {};

  namespace
  {
    // Remove const and reference
    template <typename T>
    struct Flatten
    {
      typedef typename boost::remove_const
      <typename boost::remove_reference<T>::type>::type type;
    };
  }

# define BOOST_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3)          \
  boost::function##ArgsC                                        \
  <IF(Ret, R, void)                                             \
   COMMA(Run)  WHEN(Run, runner::Runner&)                       \
   , S                                                          \
   COMMA(Arg1) WHEN(Arg1, A1)                                   \
   COMMA(Arg2) WHEN(Arg2, A2)                                   \
   COMMA(Arg3) WHEN(Arg3, A3)                                   \
   >                                                            \

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
      runner::Runner& r,                                                \
      object::objects_type& args,                                       \
      BOOST_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3) f,                  \
      const libport::Symbol& name)                                      \
    {                                                                   \
      check_arg_count(args.size() - 1, ArgsC WHEN(Run, - 1) - 1);       \
      WHEN(Ret, R res =) f(                                             \
                                                                        \
        WHEN(Run, r) COMMA(Run)                                         \
                                                                        \
        CxxConvert<typename Flatten<S>::type>                           \
        ::to(args[0], name, r, 0)                                       \
                                                                        \
        COMMA(Arg1)                                                     \
        WHEN(Arg1,                                                      \
             CxxConvert<typename Flatten<A1>::type>                     \
             ::to(args[1], name, r, 1))                                 \
                                                                        \
        COMMA(Arg2)                                                     \
        WHEN(Arg2,                                                      \
             CxxConvert<typename Flatten<A2>::type>                     \
             ::to(args[2], name, r, 2))                                 \
                                                                        \
        COMMA(Arg3)                                                     \
        WHEN(Arg3,                                                      \
             CxxConvert<typename Flatten<A3>::type>                     \
             ::to(args[3], name, r, 3))                                 \
        );                                                              \
      IF(Ret,                                                           \
         return CxxConvert<typename Flatten<R>::type>                   \
         ::from(res, name, r),                                          \
         return object::void_class);                                    \
    }                                                                   \
  };                                                                    \

# define BOOST_LIST_TYPE_MET(Ret, ArgsC, Run)   \
  boost::function##ArgsC                        \
  <IF(Ret, R, void)                             \
   COMMA(Run) WHEN(Run, runner::Runner&)        \
   , S                                          \
   , object::objects_type&                      \
   >                                            \

# define PRIMITIVE_LIST_MET(Ret, ArgsC, Run)                    \
  template <WHEN(Ret, typename R) COMMA(Ret) typename S>        \
  struct MakePrimitive<BOOST_LIST_TYPE_MET(Ret, ArgsC, Run)>    \
  {                                                             \
    static rObject primitive(                                   \
      runner::Runner& r,                                        \
      object::objects_type& args,                               \
      BOOST_LIST_TYPE_MET(Ret, ArgsC, Run) f,                   \
      const libport::Symbol& name)                              \
    {                                                           \
      S tgt =                                                   \
        CxxConvert<S>::to(args[0], name, r, 0);                 \
      args.pop_front();                                         \
      WHEN(Ret, return) f(                                      \
        WHEN(Run, r) COMMA(Run)                                 \
        tgt,                                                    \
        args                                                    \
        );                                                      \
      return void_class;                                        \
    }                                                           \
  };                                                            \

# define BOOST_LIST_TYPE(Ret, ArgsC, Run)       \
  boost::function##ArgsC                        \
  <IF(Ret, R, void),                            \
   WHEN(Run, runner::Runner&) COMMA(Run)        \
   object::objects_type&                        \
   >                                            \

# define PRIMITIVE_LIST(Ret, ArgsC, Run)                        \
  template <WHEN(Ret, typename R)>                              \
  struct MakePrimitive<BOOST_LIST_TYPE(Ret, ArgsC, Run)>        \
  {                                                             \
    static rObject primitive(                                   \
      runner::Runner& WHEN(Run, r),                             \
      object::objects_type& args,                               \
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

//   boost::function2<libport::shared_ptr<object::Tag, true>, object::objects_type&, runner::Runner&, std::allocator<boost::function_base> >

//   template <typename R>
//   struct MakePrimitive<boost::function2 <R, runner::Runner&, object::objects_type&> >
//   {
//     static rObject primitive(
//       runner::Runner& r,
//       object::objects_type& args,
//       boost::function2 <R, runner::Runner&, object::objects_type&> f,
//       const libport::Symbol&)
//     {
//       return f(r, args);
//       return object::void_class;
//     }
//   };

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
