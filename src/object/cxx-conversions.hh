#ifndef OBJECT_CXX_CONVERSIONS_HH
# define OBJECT_CXX_CONVERSIONS_HH

# include <object/object.hh>

namespace object
{
  template <typename T>
  struct CxxConvert
  {
    static rObject
    to(rObject o, const libport::Symbol& name)
    {
      return T::No_such_conversion;
    }

    static rObject
    from(const T& v,  const libport::Symbol&)
    {
      return T::No_such_conversion;
    }
  };
}

#include <object/cxx-conversions.hxx>

#endif
