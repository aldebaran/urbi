/// \file urbi/uvalue.hxx

// This file is part of UObject Component Architecture
// Copyright (c) 2009 Gostai S.A.S.
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
// For more information, comments, bug reports: http://www.urbiforge.com

#ifndef URBI_UVALUE_HXX
# define URBI_UVALUE_HXX

# include <urbi/uvalue.hh>

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

  ULIST_NTH(const)
  ULIST_NTH(/* const */)

# undef ULIST_NTH

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
    static UNamedValue instance;
    return instance;
  }



  /*----------------.
  | UObjectStruct.  |
  `----------------*/

# define UOBJECTSTRUCT_NTH(Const)               \
  inline                                        \
  Const UNamedValue&                            \
  UObjectStruct::operator[](size_t i) Const     \
  {                                             \
    if (i < size())                             \
      return array[i];                          \
    else                                        \
      return UNamedValue::error();              \
  }

  UOBJECTSTRUCT_NTH(const)
  UOBJECTSTRUCT_NTH(/* const */)

# undef UOBJECTSTRUCT_NTH

  inline
  size_t
  UObjectStruct::size() const
  {
    return array.size();
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

#define CTOR_AND_ASSIGN_AND_COMMA(Type)		\
  inline                                        \
  UValue& UValue::operator, (Type rhs)          \
  {						\
    return *this = rhs;                         \
  }

  // UFloats.
  CTOR_AND_ASSIGN_AND_COMMA(ufloat);
  CTOR_AND_ASSIGN_AND_COMMA(int);
  CTOR_AND_ASSIGN_AND_COMMA(long);
  CTOR_AND_ASSIGN_AND_COMMA(unsigned int);
  CTOR_AND_ASSIGN_AND_COMMA(unsigned long);

  // Strings.
  CTOR_AND_ASSIGN_AND_COMMA(const char*);
  CTOR_AND_ASSIGN_AND_COMMA(const void*);
  CTOR_AND_ASSIGN_AND_COMMA(const std::string&);

  // Others.
  CTOR_AND_ASSIGN_AND_COMMA(const UBinary&);
  CTOR_AND_ASSIGN_AND_COMMA(const UList&);
  CTOR_AND_ASSIGN_AND_COMMA(const UObjectStruct&);
  CTOR_AND_ASSIGN_AND_COMMA(const USound&);
  CTOR_AND_ASSIGN_AND_COMMA(const UImage&);

#undef CTOR_AND_ASSIGN_AND_COMMA

  inline
  UValue::operator int() const
  {
    return static_cast<int>(static_cast<ufloat>((*this)));
  }

  inline
  UValue::operator unsigned int() const
  {
    return static_cast<unsigned int>(static_cast<ufloat>((*this)));
  }

  inline
  UValue::operator long() const
  {
    return static_cast<long>(static_cast<ufloat>((*this)));
  }

  inline
  UValue::operator unsigned long() const
  {
    return static_cast<unsigned long>(static_cast<ufloat>((*this)));
  }

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
    assert(&v);
    return v.print(s);
  }



  /*----------.
  | Casters.  |
  `----------*/


  // Run the uvalue_caster<Type> on v.
  template <typename Type>
  typename uvar_ref_traits<typename libport::unref_traits<Type>::type>::type
  uvalue_cast(UValue& v)
  {
    return uvalue_caster<typename libport::unref_traits<Type>::type>()(v);
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

  UVALUE_CASTER_DEFINE(int);
  UVALUE_CASTER_DEFINE(unsigned int);
  UVALUE_CASTER_DEFINE(long);
  UVALUE_CASTER_DEFINE(unsigned long);
  UVALUE_CASTER_DEFINE(ufloat);
  UVALUE_CASTER_DEFINE(std::string);
  UVALUE_CASTER_DEFINE(const std::string);
  UVALUE_CASTER_DEFINE(bool);
  UVALUE_CASTER_DEFINE(UImage);
  UVALUE_CASTER_DEFINE(USound);

#undef UVALUE_CASTER_DEFINE

  template <>
  struct uvalue_caster<const UValue&>
  {
    const UValue& operator()(UValue& v)
      {
        return v;
      }
  };

  template <>
  struct uvalue_caster<UValue&>
  {
    UValue& operator()(UValue& v)
      {
        return v;
      }
  };

  template <>
  struct uvalue_caster<UValue>
  {
    UValue operator()(UValue& v)
      {
        return v;
      }
  };

  template <>
  struct uvalue_caster<const UValue>
  {
    const UValue operator()(UValue& v)
      {
        return v;
      }
  };

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
  UVALUE_CASTER_DECLARE(UObjectStruct);
  UVALUE_CASTER_DECLARE(const char*);

# undef UVALUE_CASTER_DECLARE


# ifndef UOBJECT_NO_LIST_CAST
  template<typename I>
  struct uvalue_caster <std::list<I> >
  {
    std::list<I> operator()(UValue& v)
      {
        std::list<I> res;
        if (v.type != DATA_LIST)
          // Cast just the element.
          res.push_back(uvalue_cast<I*>(v));
        else
          for (int i = 0; i < v.list->size(); ++i)
            res.push_back(uvalue_cast<I*>(*v.list->array[i]));
        return res;
      }
  };
# endif


} // namespace urbi

#endif // ! URBI_UVALUE_HXX
