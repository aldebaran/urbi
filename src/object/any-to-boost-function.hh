#ifndef ANY_TO_BOOST_FUNCTION_HH
# define ANY_TO_BOOST_FUNCTION_HH

namespace object
{
  template <typename T>
  struct AnyToBoostFunction
  {
    typedef T type;
    static T convert(T v);
  };
}

#include <object/any-to-boost-function.hxx>

#endif
