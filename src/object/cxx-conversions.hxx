#ifndef CXX_CONVERSIONS_HXX
# define CXX_CONVERSIONS_HXX

# include <object/cxx-object.hh>

namespace object
{
  // Nothing to do for objects
  template <>
  class CxxConvert<libport::shared_ptr<Object, true> >
  {
  public:
    static rObject
    to(rObject o, const libport::Symbol&)
    {
      return o;
    }

    static rObject
    from(rObject o,  const libport::Symbol&)
    {
      return o;
    }
  };

 // Convert between Urbi types
  template <typename Urbi>
  struct CxxConvert<libport::shared_ptr<Urbi, true> >
  {
    typedef libport::shared_ptr<Urbi, true> T;

    static T
    to(rObject o, const libport::Symbol& name)
    {
      type_check<Urbi>(o, name);
      return o->as<Urbi>();
    }

    static rObject
    from(const T& v,  const libport::Symbol&)
    {
      return v;
    }
  };

}

#endif
