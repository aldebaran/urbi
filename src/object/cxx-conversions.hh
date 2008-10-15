#ifndef OBJECT_CXX_CONVERSIONS_HH
# define OBJECT_CXX_CONVERSIONS_HH

# include <object/object.hh>

namespace object
{
  template <typename T>
  struct CxxConvert
  {
    static rObject
    to(const rObject&)
    {
      return T::No_such_conversion;
    }

    static rObject
    from(const T&)
    {
      return T::No_such_conversion;
    }
  };

  // Helper function
  template <typename T>
  rObject to_urbi(const T&);
}

#include <object/cxx-conversions.hxx>

#endif
