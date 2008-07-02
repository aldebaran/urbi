#ifndef CXX_CONVERSIONS_HXX
# define CXX_CONVERSIONS_HXX

# include <libport/symbol.hh>

# include <object/cxx-object.hh>
# include <object/string-class.hh>

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

  // Conversion with std::strings
  template <>
  struct CxxConvert<std::string>
  {
    static std::string
    to(rObject o, const libport::Symbol& name)
    {
      type_check<String>(o, name);
      return o->as<String>()->value_get().name_get();
    }

    static rObject
    from(const std::string& v,  const libport::Symbol&)
    {
      return new String(libport::Symbol(v));
    }
  };

  // Conversion with libport::Symbols
  template <>
  struct CxxConvert<libport::Symbol>
  {
    static libport::Symbol
    to(rObject o, const libport::Symbol& name)
    {
      type_check<String>(o, name);
      return o->as<String>()->value_get();
    }

    static rObject
    from(const libport::Symbol& v,  const libport::Symbol&)
    {
      return new String(v);
    }
  };

  // Conversion with bools
  template <>
  struct CxxConvert<bool>
  {
    static bool
    to(rObject o, const libport::Symbol& name)
    {
      return is_true(o, name);
    }

    static rObject
    from(bool v,  const libport::Symbol&)
    {
      return v ? true_class : false_class;
    }
  };

}

#endif
