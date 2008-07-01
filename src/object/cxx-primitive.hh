#ifndef CXX_PRIMITIVE_HH
# define CXX_PRIMITIVE_HH

# include <object/fwd.hh>
# include <object/object.hh>

namespace object
{
  template<typename M>
  rPrimitive make_primitive(M f, const libport::Symbol& name);
}

#include <object/cxx-primitive.hxx>

#endif
