/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/uvalue.hxx

#include <libport/cassert>
#include <libport/foreach.hh>
#include <libport/lexical-cast.hh>
#include <libport/preproc.hh>

namespace urbi
{

  /*--------.
  | UList.  |
  `--------*/

# define ULIST_NTH(Const)                       \
  inline                                        \
  Const UValue&                                 \
  UList::operator[](size_t i) Const             \
  {                                             \
    i += offset;                                \
    if (i < size())                             \
      return *array[i];                         \
    else                                        \
      return UValue::error();                   \
  }

  ULIST_NTH(__)
  ULIST_NTH(const)

# undef ULIST_NTH

  inline
  UList::iterator
  UList::begin()
  {
    return array.begin();
  }

  inline
  UList::iterator
  UList::end()
  {
    return array.end();
  }

  inline
  UList::const_iterator
  UList::begin() const
  {
    return array.begin();
  }

  inline
  UList::const_iterator
  UList::end() const
  {
    return array.end();
  }

  inline
  size_t
  UList::size() const
  {
    return array.size();
  }

  inline
  void
  UList::setOffset(size_t n)
  {
    offset = n;
  }

  template<typename T>
  inline
  UList&
  UList::push_back(const T& v)
  {
    array.push_back(new UValue(v));
    return *this;
  }

  inline
  UValue&
  UList::front()
  {
   return *array.front();
  }

  inline
  void
  UList::pop_back()
  {
    array.pop_back();
  }

  /*--------------.
  | UNamedValue.  |
  `--------------*/

  inline
  UNamedValue::UNamedValue(const std::string& n, UValue* v)
    : name(n)
    , val(v)
  {}

  inline
  UNamedValue&
  UNamedValue::error()
  {
    static UNamedValue instance("<<UNamedValue::error (denotes an error)>>");
    return instance;
  }


  /*---------.
  | UValue.  |
  `---------*/

  /// We use an operator , that behaves like an assignment.  The
  /// only difference is when the rhs is void, in which case it is
  /// the regular comma which is used.  This allows to write "uval,
  /// expr" to mean "compute expr and assign its result to uval,
  /// unless expr is void".
  inline
  UValue&
  UValue::operator, (const UValue &b)
  {
    return *this = b;
  }

#  define CONTAINER_TO_UVALUE_DECLARE(Type)                     \
  template <typename T>                                         \
  inline                                                        \
  UValue&                                                       \
  operator, (UValue &b, const Type<T> &v)                       \
  {                                                             \
    b.type = DATA_LIST;                                         \
    b.list = new UList();                                       \
    for (typename Type<T>::const_iterator i = v.begin(),        \
           i_end = v.end();                                     \
         i != i_end; ++i)                                       \
    {                                                           \
      UValue r;                                                 \
      r, *i;                                                    \
      b.list->array.push_back(new UValue(r));                   \
    }                                                           \
    return b;                                                   \
  }

  CONTAINER_TO_UVALUE_DECLARE(std::list)
  CONTAINER_TO_UVALUE_DECLARE(std::vector)

# undef CONTAINER_TO_UVALUE_DECLARE


# define OP_COMMA(Type)                         \
  inline                                        \
  UValue& UValue::operator, (Type rhs)          \
  {						\
    return *this = rhs;                         \
  }

  LIBPORT_LIST_APPLY(OP_COMMA, URBI_NUMERIC_TYPES)
  LIBPORT_LIST_APPLY(OP_COMMA, URBI_STRING_TYPES)
  LIBPORT_LIST_APPLY(OP_COMMA, URBI_MISC_TYPES)

# undef OP_COMMA



# define UVALUE_INTEGRAL_CAST(Type)                             \
  inline                                                        \
  UValue::operator Type() const                                 \
  {                                                             \
    return static_cast<Type>(static_cast<ufloat>((*this)));     \
  }

  LIBPORT_LIST_APPLY(UVALUE_INTEGRAL_CAST, URBI_DERIVED_NUMERIC_TYPES)
# undef UVALUE_INTEGRAL_CAST


  inline
  UValue::operator bool() const
  {
    return static_cast<int>(static_cast<ufloat>((*this))) != 0;
  }


  inline
  UValue&
  UValue::operator()()
  {
    return *this;
  }

  inline
  std::ostream&
  operator<<(std::ostream& s, const UValue& v)
  {
    // Looks bizarre, but might happen without have the "print" die
    // (it *has* happened).
    aver(&v);
    return v.print(s);
  }



  /*----------.
  | Casters.  |
  `----------*/


  // Run the uvalue_caster<Type> on v.
  template <typename Type>
  inline
  typename uvar_ref_traits<typename uvalue_cast_return_type<Type>::type>::type
  uvalue_cast(UValue& v)
  {
    typedef typename libport::traits::remove_reference<Type>::type res_type;
    return uvalue_caster<res_type>()(v);
  }


# define UVALUE_CASTER_DEFINE(Type)		\
  template <>					\
  struct uvalue_caster <Type>			\
  {						\
    Type operator() (UValue& v)			\
    {						\
      return v;					\
    }						\
  };

  LIBPORT_LIST_APPLY(UVALUE_CASTER_DEFINE, URBI_NUMERIC_TYPES)
  UVALUE_CASTER_DEFINE(std::string);
  UVALUE_CASTER_DEFINE(const std::string);
  UVALUE_CASTER_DEFINE(bool);
  UVALUE_CASTER_DEFINE(UImage);
  UVALUE_CASTER_DEFINE(USound);

#undef UVALUE_CASTER_DEFINE


  /*-----------------------------------.
  | Casting an UValue into an UValue.  |
  `-----------------------------------*/

  // Always return a const UValue&, a copy will be made if required.
# define UVALUE_CASTER_DEFINE(Type)              \
  template <>                                   \
  struct uvalue_caster<Type>                    \
  {                                             \
    const UValue& operator()(UValue& v)         \
    {                                           \
      return v;                                 \
    }                                           \
  }

  UVALUE_CASTER_DEFINE(const UValue&);
  UVALUE_CASTER_DEFINE(UValue&);
  UVALUE_CASTER_DEFINE(const UValue);
  UVALUE_CASTER_DEFINE(UValue);
# undef UVALUE_CASTER_DEFINE



  // The following ones are defined in uvalue-common.cc.
  template <>
  struct URBI_SDK_API uvalue_caster<UVar>
  {
    UVar& operator () (UValue& v);
  };


# define UVALUE_CASTER_DECLARE(Type)		\
  template <>					\
  struct URBI_SDK_API uvalue_caster<Type>       \
  {                                             \
    Type operator () (UValue& v);		\
  }

  UVALUE_CASTER_DECLARE(UBinary);
  UVALUE_CASTER_DECLARE(UList);
  UVALUE_CASTER_DECLARE(UDictionary);
  UVALUE_CASTER_DECLARE(const char*);

# undef UVALUE_CASTER_DECLARE


# ifndef UOBJECT_NO_LIST_CAST

#  define UVALUE_CONTAINER_CASTER_DECLARE(Type)                 \
  template <typename T>                                         \
  struct uvalue_caster< Type<T> >                               \
  {                                                             \
    Type<T>                                                     \
    operator()(UValue& v)                                       \
    {                                                           \
      Type<T> res;                                              \
      if (v.type != DATA_LIST)                                  \
        /* Cast just the element.  */                           \
        res.push_back(uvalue_cast<T>(v));                       \
      else                                                      \
        for (size_t i = 0; i < v.list->size(); ++i)             \
          res.push_back(uvalue_cast<T>(*v.list->array[i]));     \
      return res;                                               \
    }                                                           \
  }

  UVALUE_CONTAINER_CASTER_DECLARE(std::list);
  UVALUE_CONTAINER_CASTER_DECLARE(std::vector);

#  undef UVALUE_CONTAINER_CASTER_DECLARE

# endif

  // Dictionary casters.
  template <typename V>
  struct uvalue_caster<boost::unordered_map<std::string, V> >
  {
    boost::unordered_map<std::string, V> operator()(UValue& v)
    {
      boost::unordered_map<std::string, V> res;
      if (v.type != DATA_DICTIONARY)
        throw std::runtime_error("UValue is not a dictionary.");
      typedef UDictionary::value_type DictVal;
      foreach (UDictionary::value_type& val, *v.dictionary)
        res[val.first] = uvalue_cast<V>(val.second);
      return res;
    }
  };

  template <typename V>
  inline
  UValue&
  operator,(UValue& v, const boost::unordered_map<std::string, V> & d)
  {
    typedef typename boost::unordered_map<std::string, V>::value_type DictVal;
    v.clear();
    v.type = DATA_DICTIONARY;
    v.dictionary = new UDictionary;
    foreach (const DictVal & val, d)
    {
      UValue nv;
      nv, val.second;
      (*v.dictionary)[val.first] = nv;
     }
     return v;
  }

  // Uses casters, must be at the end
  template <typename T>
  inline
  UList&
  UList::operator=(const T& container)
  {
    array.clear();
    typedef const typename T::value_type constv;
    foreach (constv& v, container)
    {
      UValue val;
      val,v;
      array.push_back(new UValue(val));
    }
    return *this;
  }

  template <typename T>
  inline
  T
  UList::as()
  {
    T res;
    foreach (UValue* v, array)
      res.push_back(uvalue_caster<typename T::value_type>()(*v));
    return res;
  }

  /** Bounce to uvalue_cast(). Useful when the type of the argument is not
   * directly available.
   */
  template <typename T>
  inline
  void uvalue_cast_bounce(T& t, UValue& v)
  {
    t = uvalue_cast<T>(v);
  }

  template <typename T>
  struct uvalue_caster<UPackedData<T> >
  {
    UPackedData<T> operator() (UValue& v)
    {
      if (v.type != DATA_BINARY)
        throw std::runtime_error("invalid cast to UPackedData: not a Binary");
      if (v.binary->common.size % sizeof(T))
        throw std::runtime_error("invalid cast to UPackedData: incorrect binary"
                                 "size");
      return UPackedData<T>((T*)v.binary->common.data,
                            (T*)((char*)v.binary->common.data
                                 + v.binary->common.size));
    }
  };

  template <typename T>
  inline
  UValue& operator, (UValue&v, const UPackedData<T>& d)
  {
    v = UBinary();
    UBinary& b = *v.binary;
    b.common.size = sizeof(T)*d.size();
    b.common.data = malloc(b.common.size);
    b.message = "packed " + string_cast(sizeof(T)) + " " + typeid(T).name();
    memcpy(b.common.data, &d.front(), b.common.size);
    return v;
  }

} // namespace urbi
