#ifndef CXX_CONVERSIONS_HXX
# define CXX_CONVERSIONS_HXX

# include <boost/optional.hpp>

# include <libport/assert.hh>
# include <libport/path.hh>
# include <libport/symbol.hh>

# include <object/cxx-object.hh>
# include <object/float.hh>
# include <object/list.hh>
# include <object/path.hh>
# include <object/string.hh>
# include <runner/raise.hh>

namespace object
{
  // Nothing to do for objects
  template <>
  struct CxxConvert<libport::intrusive_ptr<Object> >
  {
    typedef libport::intrusive_ptr<Object> target_type;

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
    typedef libport::intrusive_ptr<Urbi> target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      type_check<Urbi>(o, idx);
      return o->as<Urbi>();
    }

    static rObject
    from(const target_type& v)
    {
      return v;
    }
  };

  // Convert between Urbi types pointers
  template <typename Urbi>
  struct CxxConvert<Urbi*>
  {
    typedef Urbi* target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      type_check<Urbi>(o, idx);
      return o->as<Urbi>().get();
    }

    static rObject
    from(target_type v)
    {
      return v;
    }
  };

  // Conversion with int
  template<>
  struct CxxConvert<int>
  {
    typedef int target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      type_check<Float>(o, idx);
      return o->as<Float>()->to_int();
    }

    static rObject
    from(target_type v)
    {
      return new Float(v);
    }
  };

  // Conversion with unsigned chars
  template<>
  struct CxxConvert<unsigned char>
  {
    typedef unsigned char target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      type_check<Float>(o, idx);
      int res = o->as<Float>()->to_int();
      if (res < 0 || res > 255)
        runner::raise_bad_integer_error(res, "expected a number between 0 and 255, got %s");
      return res;
    }

    static rObject
    from(target_type v)
    {
      return new Float(v);
    }
  };

  // Conversion with float
  template<>
  struct CxxConvert<float>
  {
    typedef float target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      type_check(o, Float::proto, idx);
      return o->as<Float>()->value_get();
    }

    static rObject
    from(target_type v)
    {
      return new Float(v);
    }
  };

  // Conversion with unsigned int
  template<>
  struct CxxConvert<unsigned int>
  {
    typedef unsigned int target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      type_check<Float>(o, idx);
      return o->as<Float>()->to_unsigned_int();
    }

    static rObject
    from(target_type v)
    {
      return new Float(v);
    }
  };

  // Conversion with unsigned_type
  template<>
  struct CxxConvert<Float::unsigned_type>
  {
    typedef Float::unsigned_type target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      type_check<Float>(o, idx);
      return o->as<Float>()->to_unsigned_type();
    }

    static rObject
    from(target_type v)
    {
      return new Float(v);
    }
  };

  // Conversion with floating point
  template<>
  struct CxxConvert<Float::value_type>
  {
    typedef Float::value_type target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      type_check<Float>(o, idx);
      return o->as<Float>()->value_get();
    }

    static rObject
    from(target_type v)
    {
      return new Float(v);
    }
  };

  // Conversion with std::strings
  template <>
  struct CxxConvert<std::string>
  {
    typedef std::string target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      type_check<String>(o, idx);
      return o->as<String>()->value_get();
    }

    static rObject
    from(const target_type& v)
    {
      return new String(v);
    }
  };

  // Conversion with libport::Symbols
  template <>
  struct CxxConvert<libport::Symbol>
  {
    typedef libport::Symbol target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      type_check<String>(o, idx);
      return libport::Symbol(o->as<String>()->value_get());
    }

    static rObject
    from(target_type v)
    {
      return new String(v.name_get());
    }
  };

  // Conversion with bools
  template <>
  struct CxxConvert<bool>
  {
    typedef bool target_type;
    static target_type
    to(const rObject& o, unsigned)
    {
      return is_true(o);
    }

    static rObject
    from(target_type v)
    {
      return v ? true_class : false_class;
    }
  };

  // Conversion with libport::path
  template <>
  struct CxxConvert<libport::path>
  {
    typedef libport::path target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      if (rString str = o->as<String>())
        return str->value_get();
      type_check<Path>(o, idx);
      return o->as<String>()->value_get();
    }

    static rObject
    from(const target_type& v)
    {
      return new Path(v);
    }
  };

  // Conversion with containers
#define CONTAINER(Name, Method, ExtraT, ExtraTDecl)                     \
  template <typename T ExtraTDecl>                                      \
  struct CxxConvert<Name<T ExtraT> >                                           \
  {                                                                     \
    typedef Name<T ExtraT> target_type;                                        \
                                                                        \
    static target_type                                                  \
      to(const rObject& o, unsigned idx)                                \
    {                                                                   \
      type_check<List>(o);						\
      Name<T ExtraT> res;                                                      \
      foreach (const rObject& elt, o->as<List>()->value_get())          \
        res.Method(CxxConvert<T>::to(elt, idx));                        \
      return res;                                                       \
    }                                                                   \
                                                                        \
    static rObject                                                      \
      from(const target_type& v)                                        \
    {                                                                   \
      objects_type res;                                                 \
      foreach (const T& elt, v)                                         \
        res.push_back(CxxConvert<T>::from(elt));                        \
      return new List(res);                                             \
    }                                                                   \
  };
#define comma ,
  CONTAINER(std::set, insert, /**/, /**/);
  CONTAINER(std::vector, push_back, /**/, /**/);
  CONTAINER(std::deque, push_back, /**/, /**/);
  CONTAINER(libport::ReservedVector, push_back, comma R, comma int R);
#undef comma
#undef CONTAINER


  // Conversion with boost::optional
  template <typename T>
  struct CxxConvert<boost::optional<T> >
  {
    typedef boost::optional<T> target_type;
    static target_type
    to(const rObject& o, unsigned idx)
    {
      if (o == void_class)
        return target_type();
      return CxxConvert<T>::to(o, idx);
    }

    static rObject
    from(const target_type& v)
    {
      if (!v)
        return void_class;
      return CxxConvert<T>::from(v.get());
    }
  };


  template <typename T>
  rObject to_urbi(const T& v)
  {
    return CxxConvert<T>::from(v);
  }

  template <typename T>
  T from_urbi(rObject v)
  {
    return CxxConvert<T>::to(v, 0);
  }
}

#endif
