#ifndef CXX_CONVERSIONS_HXX
# define CXX_CONVERSIONS_HXX

# include <libport/assert.hh>
# include <libport/symbol.hh>

# include <object/cxx-object.hh>
# include <object/float.hh>
# include <object/list.hh>
# include <object/string.hh>
# include <object/tag.hh>
# include <runner/runner.hh>

namespace object
{
  // Nothing to do for objects
  template <>
  class CxxConvert<libport::intrusive_ptr<Object> >
  {
  public:
    static rObject
    to(const rObject& o, unsigned)
    {
      return o;
    }

    static rObject
    from(rObject o)
    {
      if (!o)
        return void_class;
      return o;
    }
  };

  // Convert between Urbi types
  template <typename Urbi>
  struct CxxConvert<libport::intrusive_ptr<Urbi> >
  {
    typedef libport::intrusive_ptr<Urbi> T;

    static T
    to(const rObject& o, unsigned idx)
    {
      type_check<Urbi>(o, idx);
      return o->as<Urbi>();
    }

    static rObject
    from(const T& v)
    {
      return v;
    }
  };

  // Convert between Urbi types pointers
  template <typename Urbi>
  struct CxxConvert<Urbi*>
  {
    static Urbi*
    to(const rObject& o, unsigned idx)
    {
      type_check<Urbi>(o, idx);
      return o->as<Urbi>().get();
    }

    static rObject
    from(Urbi* v)
    {
      return v;
    }
  };

  // Conversion with int
  template<>
  struct CxxConvert<int>
  {
    static int
    to(const rObject& o, unsigned idx)
    {
      type_check<Float>(o, idx);
      return o->as<Float>()->to_int();
    }

    static rObject
    from(const int& v)
    {
      return new Float(v);
    }
  };

  // Conversion with unsigned int
  template<>
  struct CxxConvert<unsigned int>
  {
    static unsigned int
    to(const rObject& o, unsigned idx)
    {
      type_check<Float>(o, idx);
      return o->as<Float>()->to_unsigned_int();
    }

    static rObject
    from(const unsigned int& v)
    {
      return new Float(v);
    }
  };

  // Conversion with floating point
  template<>
  struct CxxConvert<Float::value_type>
  {
    static Float::value_type
    to(const rObject& o, unsigned idx)
    {
      type_check<Float>(o, idx);
      return o->as<Float>()->value_get();
    }

    static rObject
    from(const Float::value_type& v)
    {
      return new Float(v);
    }
  };

  // Conversion with std::strings
  template <>
  struct CxxConvert<std::string>
  {
    static std::string
    to(const rObject& o, unsigned idx)
    {
      type_check<String>(o, idx);
      return o->as<String>()->value_get();
    }

    static rObject
    from(const std::string& v)
    {
      return new String(v);
    }
  };

  // Conversion with libport::Symbols
  template <>
  struct CxxConvert<libport::Symbol>
  {
    static libport::Symbol
    to(const rObject& o, unsigned idx)
    {
      type_check<String>(o, idx);
      return libport::Symbol(o->as<String>()->value_get());
    }

    static rObject
    from(const libport::Symbol& v)
    {
      return new String(v.name_get());
    }
  };

  // Conversion with bools
  template <>
  struct CxxConvert<bool>
  {
    static bool
    to(const rObject& o, unsigned)
    {
      return is_true(o);
    }

    static rObject
    from(bool v)
    {
      return v ? true_class : false_class;
    }
  };

  // Conversion with containers
#define CONTAINER(Name, Method)                                         \
  template <typename T>                                                 \
  struct CxxConvert<Name<T> >                                           \
  {                                                                     \
    static Name<T>                                                      \
      to(const rObject& o, unsigned idx)                                \
    {                                                                   \
      type_check<List>(o);						\
      Name<T> res;                                                      \
      foreach (const rObject& elt, o->as<List>()->value_get())          \
        res.Method(CxxConvert<T>::to(elt, idx));                        \
      return res;                                                       \
    }                                                                   \
                                                                        \
    static rObject                                                      \
      from(const Name<T>& v)                                            \
    {                                                                   \
      objects_type res;                                                 \
      foreach (const T& elt, v)                                         \
        res.push_back(CxxConvert<T>::from(elt));                        \
      return new List(res);                                             \
    }                                                                   \
  };                                                                    \

  CONTAINER(std::set, insert);
  CONTAINER(std::vector, push_back);
  CONTAINER(std::deque, push_back);

#undef CONTAINER

  template <typename T>
  rObject to_urbi(const T& v)
  {
    return CxxConvert<T>::from(v);
  }
}

#endif
