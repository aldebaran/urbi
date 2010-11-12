/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef CXX_CONVERSIONS_HXX
# define CXX_CONVERSIONS_HXX

# include <boost/optional.hpp>

# include <libport/cassert>
# include <libport/format.hh>
# include <libport/path.hh>
# include <libport/symbol.hh>

# include <urbi/object/float.hh>
# include <urbi/object/list.hh>
# include <urbi/object/path.hh>
# include <urbi/object/string.hh>
# include <urbi/runner/raise.hh>

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {
    inline
    TypeError::TypeError(const rObject& expected)
      : expected_(expected)
    {}

    inline
    TypeError::~TypeError() throw()
    {}

    inline const char*
    TypeError::what() const throw()
    {
      return "TypeError";
    };

    inline const rObject&
    TypeError::expected() const
    {
      return expected_;
    }

    /*--------.
    | Helpers |
    `--------*/

    /// Convert an rFloat to an integral value.
    template <typename T>
    T
    to_integer(const rObject& o)
    {
      libport::ufloat value = o->as<Float>()->value_get();
      try
      {
        return libport::numeric_cast<T>(value);
      }
      catch (libport::bad_numeric_cast& e)
      {
        std::string format = e.what();
        format += ": %s";
        runner::raise_bad_integer_error(value, format);
      }
    }


    /*----------.
    | Objects.  |
    `----------*/

    template <typename T>
    typename CxxConvert<T>::target_type
    CxxConvert<T>::to(rObject o)
    {
      type_check<T>(o);
      return *o->as<T>();
    }

    template <typename T>
    rObject
    CxxConvert<T>::from(source_type v)
    {
      return &v;
    }

    template <>
    struct CxxConvert<libport::intrusive_ptr<Object> >
    {
      typedef libport::intrusive_ptr<Object> target_type;

      static rObject
      to(const rObject& o)
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


    /*-------------.
    | Urbi types.  |
    `-------------*/

    template <typename Urbi>
    struct CxxConvert<libport::intrusive_ptr<Urbi> >
    {
      typedef libport::intrusive_ptr<Urbi> target_type;
      static target_type
      to(const rObject& o)
      {
        type_check<Urbi>(o);
        return o->as<Urbi>();
      }

      static rObject
      from(const target_type& v)
      {
        return v;
      }
    };


    /*----------------------.
    | Urbi types pointers.  |
    `----------------------*/

    template <typename Urbi>
    struct CxxConvert<Urbi*>
    {
      typedef Urbi* target_type;
      static target_type
      to(const rObject& o)
      {
        type_check<Urbi>(o);
        return o->as<Urbi>().get();
      }

      static rObject
      from(target_type v)
      {
        return v;
      }
    };

    /*------.
    | int.  |
    `------*/

    template<>
    struct CxxConvert<int>
    {
      typedef Float::int_type target_type;
      static target_type
      to(const rObject& o)
      {
        return type_check<Float>(o)->to_int_type();
      }

      static rObject
      from(target_type v)
      {
        return new Float(v);
      }
    };


    /*-----------------.
    | Integral types.  |
    `-----------------*/

#define CONVERT(Type)                           \
    template<>                                  \
    struct CxxConvert<Type>                     \
    {                                           \
      typedef Type target_type;                 \
      static target_type                        \
        to(const rObject& o)                    \
      {                                         \
        type_check<Float>(o);                   \
        return to_integer<target_type>(o);      \
      }                                         \
                                                \
      static rObject                            \
        from(target_type v)                     \
      {                                         \
        return new Float(v);                    \
      }                                         \
    };

    CONVERT(unsigned char);
    CONVERT(unsigned short);
    CONVERT(unsigned long);
    CONVERT(short);
    CONVERT(long);
    CONVERT(long long);
#undef CONVERT


    /*--------.
    | float.  |
    `--------*/

    template<>
    struct CxxConvert<float>
    {
      typedef float target_type;
      static target_type
      to(const rObject& o)
      {
        type_check(o, Float::proto);
        return o->as<Float>()->value_get();
      }

      static rObject
      from(target_type v)
      {
        return new Float(v);
      }
    };


    /*----------------.
    | unsigned_type.  |
    `----------------*/

    template<>
    struct CxxConvert<Float::unsigned_type>
    {
      typedef Float::unsigned_type target_type;
      static target_type
      to(const rObject& o)
      {
        type_check<Float>(o);
        return o->as<Float>()->to_unsigned_type();
      }

      static rObject
      from(target_type v)
      {
        return new Float(v);
      }
    };


    /*-----------------.
    | floating point.  |
    `-----------------*/

    template<>
    struct CxxConvert<Float::value_type>
    {
      typedef Float::value_type target_type;
      static target_type
      to(const rObject& o)
      {
        type_check<Float>(o);
        return o->as<Float>()->value_get();
      }

      static rObject
      from(target_type v)
      {
        return new Float(v);
      }
    };


    /*--------------.
    | std::string.  |
    `--------------*/

    template <>
    struct CxxConvert<std::string>
    {
      typedef       std::string  target_type;
      typedef const std::string& source_type;
      static target_type
      to(const rObject& o)
      {
        if (rPath p = o->as<Path>())
          return p->value_get();
        type_check<String>(o);
        return o->as<String>()->value_get();
      }

      static rObject
      from(source_type v)
      {
        return new String(v);
      }
    };


    /*-------.
    | char.  |
    `-------*/

    template <>
    struct CxxConvert<char>
    {
      typedef char target_type;
      static target_type
      to(const rObject& o)
      {
        type_check<String>(o);
        std::string str = o->as<String>()->value_get();
        if (str.size() != 1)
          runner::raise_primitive_error
            ("expected one character string for argument");
            //FIXME: Would be good to give argument number.
        return str[0];
      }

      static rObject
      from(target_type v)
      {
        return new String(std::string(1, v));
      }
    };


    /*------------------------.
    | char* and const char*.  |
    `------------------------*/

#define CONVERT(Type)                                                   \
    template <>                                                         \
    struct CxxConvert<Type>                                             \
    {                                                                   \
      typedef Type target_type;                                         \
      static target_type                                                \
        to(const rObject& o)                                            \
      {                                                                 \
        return strdup(CxxConvert<std::string>::to(o).c_str());          \
      }                                                                 \
                                                                        \
      static rObject                                                    \
        from(target_type v)                                             \
      {                                                                 \
        return CxxConvert<std::string>::from(std::string(v));           \
      }                                                                 \
    };

    CONVERT(char*);
    CONVERT(const char*);

#undef CONVERT


    /*------------------.
    | libport::Symbol.  |
    `------------------*/

    template <>
    struct CxxConvert<libport::Symbol>
    {
      typedef libport::Symbol target_type;
      static target_type
      to(const rObject& o)
      {
        type_check<String>(o);
        return libport::Symbol(o->as<String>()->value_get());
      }

      static rObject
      from(target_type v)
      {
        return new String(v.name_get());
      }
    };


    /*-------.
    | bool.  |
    `-------*/

    template <>
    struct CxxConvert<bool>
    {
      typedef bool target_type;
      static target_type
      to(const rObject& o)
      {
        return o->as_bool();
      }

      static rObject
      from(target_type v)
      {
        return v ? true_class : false_class;
      }
    };


    /*----------------.
    | libport::path.  |
    `----------------*/

    template <>
    struct CxxConvert<libport::path>
    {
      typedef libport::path target_type;
      static target_type
      to(const rObject& o)
      {
        if (rString str = o->as<String>())
          return str->value_get();
        type_check<Path>(o);
        return o->as<String>()->value_get();
      }

      static rObject
      from(const target_type& v)
      {
        return new Path(v);
      }
    };


    /*-------------------------------------------------------------.
    | std::set, std::vector, std::deque, libport::ReservedVector.  |
    `-------------------------------------------------------------*/

#define CONTAINER(Name, Method, ExtraT, ExtraTDecl)                     \
    template <typename T ExtraTDecl>                                    \
    struct CxxConvert<Name<T ExtraT> >                                  \
    {                                                                   \
      typedef Name<T ExtraT> target_type;                               \
                                                                        \
      static target_type                                                \
        to(const rObject& o)                                            \
      {                                                                 \
        type_check<List>(o);                                            \
        Name<T ExtraT> res;                                             \
        foreach (const rObject& elt, o->as<List>()->value_get())        \
          res.Method(CxxConvert<T>::to(elt));                           \
        return res;                                                     \
      }                                                                 \
                                                                        \
      static rObject                                                    \
        from(const target_type& v)                                      \
      {                                                                 \
        objects_type res;                                               \
        foreach (const T& elt, v)                                       \
          res.push_back(CxxConvert<T>::from(elt));                      \
        return new List(res);                                           \
      }                                                                 \
    };

#define comma ,
    CONTAINER(std::set, insert, /**/, /**/);
    CONTAINER(std::vector, push_back, /**/, /**/);
    CONTAINER(std::deque, push_back, /**/, /**/);
    CONTAINER(libport::ReservedVector, push_back, comma R, comma int R);
#undef comma
#undef CONTAINER


    /*------------------.
    | boost::optional.  |
    `------------------*/

    template <typename T>
    struct CxxConvert<boost::optional<T> >
    {
      typedef boost::optional<T> target_type;
      static target_type
      to(const rObject& o)
      {
        if (o == void_class)
          return target_type();
        return CxxConvert<T>::to(o);
      }

      static rObject
      from(const target_type& v)
      {
        if (!v)
          return void_class;
        return CxxConvert<T>::from(v.get());
      }
    };


    /*------------.
    | std::pair.  |
    `------------*/

    template <typename T1, typename T2>
    struct CxxConvert<std::pair<T1, T2> >
    {
      typedef std::pair<T1, T2> target_type;
      static target_type
      to(const rObject& o)
      {
        type_check<List>(o);
        const List::value_type& list = o->as<List>()->value_get();
        if (list.size() != 2)
          runner::raise_primitive_error("Expected a list of size 2");
        return std::make_pair(CxxConvert<T1>::to(list[0]),
                              CxxConvert<T2>::to(list[1]));
      }

      static rObject
      from(const target_type& v)
      {
        List::value_type content;
        content.push_back(CxxConvert<T1>::from(v.first ));
        content.push_back(CxxConvert<T2>::from(v.second));
        return new List(content);
      }
    };

    /*--------------------.
    | to_urbi/from_urbi.  |
    `--------------------*/

    template <typename T>
    rObject to_urbi(const T& v)
    {
      return CxxConvert<T>::from(v);
    }

    template <typename T>
    typename CxxConvert<T>::target_type
    from_urbi(rObject v)
    {
      try
      {
        return CxxConvert<T>::to(v);
      }
      catch (TypeError& e)
      {
        runner::raise_type_error(v, e.expected());
      }
    }

    template<typename T>
    typename CxxConvert<T>::target_type
    from_urbi(rObject o, unsigned idx)
    {
      try
      {
        return CxxConvert<T>::to(o);
      }
      catch (TypeError& e)
      {
        runner::raise_argument_type_error(idx, o, e.expected());
      }
    }
  }
}

#endif
