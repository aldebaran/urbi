#ifndef ANY_TO_BOOST_FUNCTION_HXX
# define ANY_TO_BOOST_FUNCTION_HXX

# include <boost/function.hpp>
# include <object/cxx-helper.hh>
# include <runner/runner.hh>

namespace object
{
  template <typename T>
  T AnyToBoostFunction<T>::convert(T v)
  {
    // If you fail here, the given type is not supported for
    // conversion to boost::function
    return v;
  }

# define FUN_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3, Name)              \
  IF(Ret, R, void) (*Name)(                                             \
    WHEN(Run, runner::Runner&) COMMA(Run)                               \
    S                                                                   \
    COMMA(Arg1) WHEN(Arg1, A1)                                          \
    COMMA(Arg2) WHEN(Arg2, A2)                                          \
    COMMA(Arg3) WHEN(Arg3, A3)                                          \
    )                                                                   \

# define MET_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3, Name)              \
  IF(Ret, R, void) (S::*Name)(                                          \
    WHEN(Run, runner::Runner&)                                          \
    COMMA2(Run, Arg1) WHEN(Arg1, A1)                                    \
    COMMA(Arg2) WHEN(Arg2, A2)                                          \
    COMMA(Arg3) WHEN(Arg3, A3)                                          \
    )                                                                   \

# define BOOST_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3, Self)            \
  boost::function##ArgsC<IF(Ret, R, void),                              \
                         Self                                           \
                         COMMA(Run) WHEN(Run, runner::Runner&)          \
                         COMMA(Arg1) WHEN(Arg1, A1)                     \
                         COMMA(Arg2) WHEN(Arg2, A2)                     \
                         COMMA(Arg3) WHEN(Arg3, A3)                     \
                         >                                              \

# define A2F_FUNCTION(Ret, ArgsC, Run, Arg1, Arg2, Arg3)                \
  template                                                              \
  <WHEN(Ret, typename R) COMMA(Ret)                                     \
   typename S                                                           \
   COMMA(Arg1) WHEN(Arg1, typename A1)                                  \
   COMMA(Arg2) WHEN(Arg2, typename A2)                                  \
   COMMA(Arg3) WHEN(Arg3, typename A3)                                  \
   >                                                                    \
  struct AnyToBoostFunction                                             \
  <FUN_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3, )>                       \
  {                                                                     \
    typedef BOOST_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3,               \
                       S) type;                                         \
    static type                                                         \
      convert(FUN_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3, v))           \
    {                                                                   \
      return                                                            \
        type(v);                                                        \
    }                                                                   \
  };                                                                    \

# define A2F_METHOD(Ret, ArgsC, Run, Arg1, Arg2, Arg3)                  \
  template                                                              \
  <WHEN(Ret, typename R) COMMA(Ret)                                     \
   typename S                                                           \
   COMMA(Arg1) WHEN(Arg1, typename A1)                                  \
   COMMA(Arg2) WHEN(Arg2, typename A2)                                  \
   COMMA(Arg3) WHEN(Arg3, typename A3)                                  \
   >                                                                    \
  struct AnyToBoostFunction                                             \
  <MET_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3, )>                       \
  {                                                                     \
    typedef BOOST_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3,               \
                       libport::shared_ptr<S>) type;                    \
    static type                                                         \
      convert(MET_TYPE(Ret, ArgsC, Run, Arg1, Arg2, Arg3, v))           \
    {                                                                   \
      return v;                                                         \
    }                                                                   \
  };                                                                    \

  ALL_PRIMITIVE(A2F_FUNCTION);
  ALL_PRIMITIVE(A2F_METHOD);

  // Treat the case of argument-less functions manually
  namespace
  {
    template <typename R>
    R ignore_self(R (*f)(), rObject)
    {
      return f();
    }
  }

  template <typename R>
  struct AnyToBoostFunction<R (*) ()>
  {
    typedef boost::function1<R, rObject> type;
    static type
    convert(R (*f) ())
    {
      return boost::bind(ignore_self<R>, f, _1);
    }
  };

# undef A2F_METHOD
# undef A2F_FUNCTION
# undef MET_TYPE
# undef FUN_TYPE
# undef BOOST_TYPE

}
#endif
