#ifndef OBJECT_CXX_CONVERSIONS_HH
# define OBJECT_CXX_CONVERSIONS_HH

# include <object/object.hh>

namespace object
{

  template <typename T>
  struct CxxConvert
  {
    typedef T target_type;

    static target_type
    to(const rObject&, unsigned)
    {
      return target_type::No_such_conversion;
    }

    static rObject
    from(const target_type&)
    {
      return target_type::No_such_conversion;
    }
  };

  // Helper function
  template <typename T>
  rObject to_urbi(const T&);
}

#include <object/cxx-conversions.hxx>

#endif
