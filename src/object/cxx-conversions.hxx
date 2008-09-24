#ifndef CXX_CONVERSIONS_HXX
# define CXX_CONVERSIONS_HXX

# include <libport/assert.hh>
# include <libport/symbol.hh>

# include <object/cxx-object.hh>
# include <object/float.hh>
# include <object/list.hh>
# include <object/string.hh>
# include <object/tag.hh>
# include <scheduler/tag.hh>

namespace object
{
  // Nothing to do for objects
  template <>
  class CxxConvert<libport::shared_ptr<Object, true> >
  {
  public:
    static rObject
    to(const rObject& o, const libport::Symbol&, unsigned)
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
    to(const rObject& o, const libport::Symbol&, unsigned idx)
    {
      type_check(o, Urbi::proto, idx);
      return o->as<Urbi>();
    }

    static rObject
    from(const T& v, const libport::Symbol&)
    {
      return v;
    }
  };

  // Convert between Urbi types pointers
  template <typename Urbi>
  struct CxxConvert<Urbi*>
  {
    static Urbi*
    to(const rObject& o, const libport::Symbol&, unsigned idx)
    {
      type_check(o, Urbi::proto, idx);
      return o->as<Urbi>().get();
    }

    static rObject
    from(Urbi* v, const libport::Symbol&)
    {
      return v;
    }
  };

  // Conversion with int
  template<>
  struct CxxConvert<int>
  {
    static int
    to(const rObject& o, const libport::Symbol& name, unsigned idx)
    {
      type_check(o, Float::proto, idx);
      return o->as<Float>()->to_int(name);
    }

    static rObject
    from(const int& v, const libport::Symbol&)
    {
      return new Float(v);
    }
  };

  // Conversion with unsigned int
  template<>
  struct CxxConvert<unsigned int>
  {
    static unsigned int
    to(const rObject& o, const libport::Symbol& name, unsigned idx)
    {
      type_check(o, Float::proto, idx);
      return o->as<Float>()->to_unsigned_int(name);
    }

    static rObject
    from(const unsigned int& v, const libport::Symbol&)
    {
      return new Float(v);
    }
  };

  // Conversion with floating point
  template<>
  struct CxxConvert<Float::value_type>
  {
    static Float::value_type
    to(const rObject& o, const libport::Symbol&, unsigned idx)
    {
      type_check(o, Float::proto, idx);
      return o->as<Float>()->value_get();
    }

    static rObject
    from(const Float::value_type& v, const libport::Symbol&)
    {
      return new Float(v);
    }
  };

  // Conversion with std::strings
  template <>
  struct CxxConvert<std::string>
  {
    static std::string
    to(const rObject& o, const libport::Symbol&, unsigned idx)
    {
      type_check(o, String::proto, idx);
      return o->as<String>()->value_get();
    }

    static rObject
    from(const std::string& v,  const libport::Symbol&)
    {
      return new String(v);
    }
  };

  // Conversion with libport::Symbols
  template <>
  struct CxxConvert<libport::Symbol>
  {
    static libport::Symbol
    to(const rObject& o, const libport::Symbol&, unsigned idx)
    {
      type_check(o, String::proto, idx);
      return libport::Symbol(o->as<String>()->value_get());
    }

    static rObject
    from(const libport::Symbol& v,  const libport::Symbol&)
    {
      return new String(v.name_get());
    }
  };

  // Conversion with bools
  template <>
  struct CxxConvert<bool>
  {
    static bool
    to(const rObject& o, const libport::Symbol& name, unsigned)
    {
      return is_true(o, name);
    }

    static rObject
    from(bool v,  const libport::Symbol&)
    {
      return v ? true_class : false_class;
    }
  };

  // Conversion with tags_type
  template <>
  struct CxxConvert<scheduler::tags_type>
  {
    static scheduler::tags_type
    to(const rObject&, const libport::Symbol& name, unsigned)
    {
      (void)name;
      pabort(name);
    }

    static rObject
    from(const scheduler::tags_type& v, const libport::Symbol&)
    {
      List::value_type res;
      foreach (const scheduler::rTag& tag, v)
	res.push_back(new Tag(tag));
      return new List(res);
    }
  };

  // Conversion with std::set
  template <typename T>
  struct CxxConvert<std::set<T> >
  {
    static std::set<T>
    to(const rObject& o, const libport::Symbol& name, unsigned idx)
    {
      type_check(o, List::proto);
      std::set<T> res;
      foreach (const rObject& elt, o->as<List>()->value_get())
        res.insert(CxxConvert<T>::to(elt, name, idx));
      return res;
    }

    static rObject
    from(const std::set<T>& v, const libport::Symbol& name)
    {
      objects_type res;
      foreach (const T& elt, v)
        res.push_back(CxxConvert<T>::from(elt, name));
      return new List(res);
    }
  };

}

#endif
