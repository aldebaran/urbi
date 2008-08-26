#ifndef CXX_CONVERSIONS_HXX
# define CXX_CONVERSIONS_HXX

# include <libport/symbol.hh>

# include <object/cxx-object.hh>
# include <object/float-class.hh>
# include <object/string-class.hh>
# include <scheduler/tag.hh>

namespace object
{
  // Nothing to do for objects
  template <>
  class CxxConvert<libport::shared_ptr<Object, true> >
  {
  public:
    static rObject
    to(const rObject& o, const libport::Symbol&)
    {
      return o;
    }

    static rObject
    from(rObject o,  const libport::Symbol&)
    {
      if (!o)
        return void_class;
      return o;
    }
  };

 // Convert between Urbi types
  template <typename Urbi>
  struct CxxConvert<libport::shared_ptr<Urbi, true> >
  {
    typedef libport::shared_ptr<Urbi, true> T;

    static T
    to(const rObject& o, const libport::Symbol& name)
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

  // Conversion with unsigned int
  template<>
  struct CxxConvert<unsigned int>
  {
    static unsigned int
    to(const rObject& o, const libport::Symbol& name)
      {
	type_check<Float>(o, name);
	return o->as<Float>()->to_unsigned_int(name);
      }

    static rObject
    from(const unsigned int& v, const libport::Symbol&)
      {
	return new Float(v);
      }
  };

  // Conversion with std::strings
  template <>
  struct CxxConvert<std::string>
  {
    static std::string
    to(const rObject& o, const libport::Symbol& name)
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
    to(const rObject& o, const libport::Symbol& name)
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
    to(const rObject& o, const libport::Symbol& name)
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
