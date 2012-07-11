#ifndef ANY_TO_BOOST_FUNCTION_HXX
# define ANY_TO_BOOST_FUNCTION_HXX

# include <boost/bind.hpp>
# include <boost/function.hpp>
# include <urbi/object/fwd.hh>

namespace urbi
{
  namespace object
  {
    template <typename T>
    T AnyToBoostFunction<T>::convert(T v)
    {
      // If you fail here, the given type is not supported for
      // conversion to boost::function
      return v;
    }


    // Return: True, Method: 0, Arguments: 0
    template <typename R, typename S>
    struct AnyToBoostFunction<R (*)(S)>
    {
      typedef boost::function1<R, S> type;
      enum { arity = 1 };
      static type
      convert(R (*v)(S))
      {
        return type(v);
      }
    };

    // Return: True, Method: 0, Arguments: 1
    template <typename R, typename S, typename Arg0>
    struct AnyToBoostFunction<R (*)(S, Arg0)>
    {
      typedef boost::function2<R, S, Arg0> type;
      enum { arity = 2 };
      static type
      convert(R (*v)(S, Arg0))
      {
        return type(v);
      }
    };

    // Return: True, Method: 0, Arguments: 2
    template <typename R, typename S, typename Arg0, typename Arg1>
    struct AnyToBoostFunction<R (*)(S, Arg0, Arg1)>
    {
      typedef boost::function3<R, S, Arg0, Arg1> type;
      enum { arity = 3 };
      static type
      convert(R (*v)(S, Arg0, Arg1))
      {
        return type(v);
      }
    };

    // Return: True, Method: 0, Arguments: 3
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2>
    struct AnyToBoostFunction<R (*)(S, Arg0, Arg1, Arg2)>
    {
      typedef boost::function4<R, S, Arg0, Arg1, Arg2> type;
      enum { arity = 4 };
      static type
      convert(R (*v)(S, Arg0, Arg1, Arg2))
      {
        return type(v);
      }
    };

    // Return: True, Method: 0, Arguments: 4
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
    struct AnyToBoostFunction<R (*)(S, Arg0, Arg1, Arg2, Arg3)>
    {
      typedef boost::function5<R, S, Arg0, Arg1, Arg2, Arg3> type;
      enum { arity = 5 };
      static type
      convert(R (*v)(S, Arg0, Arg1, Arg2, Arg3))
      {
        return type(v);
      }
    };

    // Return: True, Method: 0, Arguments: 5
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    struct AnyToBoostFunction<R (*)(S, Arg0, Arg1, Arg2, Arg3, Arg4)>
    {
      typedef boost::function6<R, S, Arg0, Arg1, Arg2, Arg3, Arg4> type;
      enum { arity = 6 };
      static type
      convert(R (*v)(S, Arg0, Arg1, Arg2, Arg3, Arg4))
      {
        return type(v);
      }
    };

    // Return: True, Method: 0, Arguments: 6
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
    struct AnyToBoostFunction<R (*)(S, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5)>
    {
      typedef boost::function7<R, S, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5> type;
      enum { arity = 7 };
      static type
      convert(R (*v)(S, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5))
      {
        return type(v);
      }
    };

    // Return: True, Method: 0, Arguments: 7
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
    struct AnyToBoostFunction<R (*)(S, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6)>
    {
      typedef boost::function8<R, S, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6> type;
      enum { arity = 8 };
      static type
      convert(R (*v)(S, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6))
      {
        return type(v);
      }
    };

    // Return: True, Method: 0, Arguments: 8
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
    struct AnyToBoostFunction<R (*)(S, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7)>
    {
      typedef boost::function9<R, S, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7> type;
      enum { arity = 9 };
      static type
      convert(R (*v)(S, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7))
      {
        return type(v);
      }
    };

    // Return: True, Method: 0, Arguments: 9
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
    struct AnyToBoostFunction<R (*)(S, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8)>
    {
      typedef boost::function10<R, S, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8> type;
      enum { arity = 10 };
      static type
      convert(R (*v)(S, Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8))
      {
        return type(v);
      }
    };

    // Return: True, Method: 1, Arguments: 0
    template <typename R, typename S>
    struct AnyToBoostFunction<R (S::*)()>
    {
      typedef boost::function1<R, S* > type;
      enum { arity = 1 };
      static type
      convert(R (S::*v)())
      {
        return type(v);
      }
    };

    // Return: True, Method: 1, Arguments: 1
    template <typename R, typename S, typename Arg0>
    struct AnyToBoostFunction<R (S::*)(Arg0)>
    {
      typedef boost::function2<R, S* , Arg0> type;
      enum { arity = 2 };
      static type
      convert(R (S::*v)(Arg0))
      {
        return type(v);
      }
    };

    // Return: True, Method: 1, Arguments: 2
    template <typename R, typename S, typename Arg0, typename Arg1>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1)>
    {
      typedef boost::function3<R, S* , Arg0, Arg1> type;
      enum { arity = 3 };
      static type
      convert(R (S::*v)(Arg0, Arg1))
      {
        return type(v);
      }
    };

    // Return: True, Method: 1, Arguments: 3
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2)>
    {
      typedef boost::function4<R, S* , Arg0, Arg1, Arg2> type;
      enum { arity = 4 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2))
      {
        return type(v);
      }
    };

    // Return: True, Method: 1, Arguments: 4
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2, Arg3)>
    {
      typedef boost::function5<R, S* , Arg0, Arg1, Arg2, Arg3> type;
      enum { arity = 5 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2, Arg3))
      {
        return type(v);
      }
    };

    // Return: True, Method: 1, Arguments: 5
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2, Arg3, Arg4)>
    {
      typedef boost::function6<R, S* , Arg0, Arg1, Arg2, Arg3, Arg4> type;
      enum { arity = 6 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2, Arg3, Arg4))
      {
        return type(v);
      }
    };

    // Return: True, Method: 1, Arguments: 6
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5)>
    {
      typedef boost::function7<R, S* , Arg0, Arg1, Arg2, Arg3, Arg4, Arg5> type;
      enum { arity = 7 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5))
      {
        return type(v);
      }
    };

    // Return: True, Method: 1, Arguments: 7
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6)>
    {
      typedef boost::function8<R, S* , Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6> type;
      enum { arity = 8 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6))
      {
        return type(v);
      }
    };

    // Return: True, Method: 1, Arguments: 8
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7)>
    {
      typedef boost::function9<R, S* , Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7> type;
      enum { arity = 9 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7))
      {
        return type(v);
      }
    };

    // Return: True, Method: 1, Arguments: 9
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8)>
    {
      typedef boost::function10<R, S* , Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8> type;
      enum { arity = 10 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8))
      {
        return type(v);
      }
    };

    // Return: True, Method: 2, Arguments: 0
    template <typename R, typename S>
    struct AnyToBoostFunction<R (S::*)() const>
    {
      typedef boost::function1<R, const S* > type;
      enum { arity = 1 };
      static type
      convert(R (S::*v)() const)
      {
        return type(v);
      }
    };

    // Return: True, Method: 2, Arguments: 1
    template <typename R, typename S, typename Arg0>
    struct AnyToBoostFunction<R (S::*)(Arg0) const>
    {
      typedef boost::function2<R, const S* , Arg0> type;
      enum { arity = 2 };
      static type
      convert(R (S::*v)(Arg0) const)
      {
        return type(v);
      }
    };

    // Return: True, Method: 2, Arguments: 2
    template <typename R, typename S, typename Arg0, typename Arg1>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1) const>
    {
      typedef boost::function3<R, const S* , Arg0, Arg1> type;
      enum { arity = 3 };
      static type
      convert(R (S::*v)(Arg0, Arg1) const)
      {
        return type(v);
      }
    };

    // Return: True, Method: 2, Arguments: 3
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2) const>
    {
      typedef boost::function4<R, const S* , Arg0, Arg1, Arg2> type;
      enum { arity = 4 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2) const)
      {
        return type(v);
      }
    };

    // Return: True, Method: 2, Arguments: 4
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2, Arg3) const>
    {
      typedef boost::function5<R, const S* , Arg0, Arg1, Arg2, Arg3> type;
      enum { arity = 5 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2, Arg3) const)
      {
        return type(v);
      }
    };

    // Return: True, Method: 2, Arguments: 5
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2, Arg3, Arg4) const>
    {
      typedef boost::function6<R, const S* , Arg0, Arg1, Arg2, Arg3, Arg4> type;
      enum { arity = 6 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2, Arg3, Arg4) const)
      {
        return type(v);
      }
    };

    // Return: True, Method: 2, Arguments: 6
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5) const>
    {
      typedef boost::function7<R, const S* , Arg0, Arg1, Arg2, Arg3, Arg4, Arg5> type;
      enum { arity = 7 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5) const)
      {
        return type(v);
      }
    };

    // Return: True, Method: 2, Arguments: 7
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const>
    {
      typedef boost::function8<R, const S* , Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6> type;
      enum { arity = 8 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6) const)
      {
        return type(v);
      }
    };

    // Return: True, Method: 2, Arguments: 8
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) const>
    {
      typedef boost::function9<R, const S* , Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7> type;
      enum { arity = 9 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7) const)
      {
        return type(v);
      }
    };

    // Return: True, Method: 2, Arguments: 9
    template <typename R, typename S, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6, typename Arg7, typename Arg8>
    struct AnyToBoostFunction<R (S::*)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8) const>
    {
      typedef boost::function10<R, const S* , Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8> type;
      enum { arity = 10 };
      static type
      convert(R (S::*v)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7, Arg8) const)
      {
        return type(v);
      }
    };

    template <typename R>
    struct AnyToBoostFunction<boost::function0<R> >
    {
      typedef boost::function0<R> type;
      enum { arity = 0 };
      static type
      convert(type v)
      {
        return v;
      }
    };

    template <typename R, typename A0>
    struct AnyToBoostFunction<boost::function1<R, A0> >
    {
      typedef boost::function1<R, A0> type;
      enum { arity = 1 };
      static type
      convert(type v)
      {
        return v;
      }
    };

    template <typename R, typename A0, typename A1>
    struct AnyToBoostFunction<boost::function2<R, A0, A1> >
    {
      typedef boost::function2<R, A0, A1> type;
      enum { arity = 2 };
      static type
      convert(type v)
      {
        return v;
      }
    };

    template <typename R, typename A0, typename A1, typename A2>
    struct AnyToBoostFunction<boost::function3<R, A0, A1, A2> >
    {
      typedef boost::function3<R, A0, A1, A2> type;
      enum { arity = 3 };
      static type
      convert(type v)
      {
        return v;
      }
    };

    template <typename R, typename A0, typename A1, typename A2, typename A3>
    struct AnyToBoostFunction<boost::function4<R, A0, A1, A2, A3> >
    {
      typedef boost::function4<R, A0, A1, A2, A3> type;
      enum { arity = 4 };
      static type
      convert(type v)
      {
        return v;
      }
    };

    template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
    struct AnyToBoostFunction<boost::function5<R, A0, A1, A2, A3, A4> >
    {
      typedef boost::function5<R, A0, A1, A2, A3, A4> type;
      enum { arity = 5 };
      static type
      convert(type v)
      {
        return v;
      }
    };

    template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    struct AnyToBoostFunction<boost::function6<R, A0, A1, A2, A3, A4, A5> >
    {
      typedef boost::function6<R, A0, A1, A2, A3, A4, A5> type;
      enum { arity = 6 };
      static type
      convert(type v)
      {
        return v;
      }
    };

    template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    struct AnyToBoostFunction<boost::function7<R, A0, A1, A2, A3, A4, A5, A6> >
    {
      typedef boost::function7<R, A0, A1, A2, A3, A4, A5, A6> type;
      enum { arity = 7 };
      static type
      convert(type v)
      {
        return v;
      }
    };

    template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    struct AnyToBoostFunction<boost::function8<R, A0, A1, A2, A3, A4, A5, A6, A7> >
    {
      typedef boost::function8<R, A0, A1, A2, A3, A4, A5, A6, A7> type;
      enum { arity = 8 };
      static type
      convert(type v)
      {
        return v;
      }
    };

    template <typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    struct AnyToBoostFunction<boost::function9<R, A0, A1, A2, A3, A4, A5, A6, A7, A8> >
    {
      typedef boost::function9<R, A0, A1, A2, A3, A4, A5, A6, A7, A8> type;
      enum { arity = 9 };
      static type
      convert(type v)
      {
        return v;
      }
    };


    // Treat the case of argument-less functions manually
    template <typename R>
    R ignore_self(R (*f)(), urbi::object::rObject)
    {
      return f();
    }

    template <typename R>
    struct AnyToBoostFunction<R (*) ()>
    {
      typedef boost::function1<R, urbi::object::rObject> type;
      enum { arity = 1 };
      static type
      convert(R (*f) ())
      {
        return boost::bind(ignore_self<R>, f, _1);
      }
    };
  }
}
#endif

