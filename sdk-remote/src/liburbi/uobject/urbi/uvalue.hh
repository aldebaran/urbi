/// \file urbi/uvalue.hh

// This file is part of UObject Component Architecture
// Copyright (c) 2007, 2008 Gostai S.A.S.
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
// For more information, comments, bug reports: http://www.urbiforge.com

#ifndef URBI_UVALUE_HH
# define URBI_UVALUE_HH

# include <vector>
# include "libport/ufloat.h"

# include "urbi/fwd.hh"
# include "urbi/ubinary.hh"

namespace urbi
{

  // UValue and other related types

  /// Possible value types a UValue can contain.
  enum UDataType
  {
    DATA_DOUBLE,
    DATA_STRING,
    DATA_BINARY,
    DATA_LIST,
    DATA_OBJECT,
    DATA_VOID
  };

  /// Class storing URBI List type
  class UList
  {
  public:
    std::vector<UValue *> array;
    UList();
    UList(const UList &b);
    UList & operator = (const UList &b);
    ~UList();
    UValue & operator [](int i)
    {
      return *array[i+offset];
    }
    const UValue & operator [](int i) const
    {
      return *array[i+offset];
    }
    int size() const
    {
      return array.size();
    }
    void setOffset(int n)
    {
      offset = n;
    }

  private:
    void clear();
    int offset;
  };

  class UNamedValue
  {
  public:
    UNamedValue(const std::string& n, UValue* v)
      : val(v), name(n)
    {}
    UNamedValue()
      : val(0), name()
    {}
    UValue *val;
    std::string name;
  };

  class UObjectStruct
  {
  public:
    UObjectStruct();
    UObjectStruct(const UObjectStruct &b);
    UObjectStruct& operator = (const UObjectStruct &b);
    ~UObjectStruct();

    UValue& operator [](const std::string& s);
    UNamedValue& operator [](int i)
    {
      return array[i];
    }

    int size() const
    {
      return array.size();
    }

    std::string refName;
    std::vector<UNamedValue> array;
  };

  /** Container for a value that handles all types known to URBI.
   */
  class UValue
  {
  public:
    UDataType       type;
    ufloat          val;  ///< value if of type DATA_DOUBLE
    union
    {
      std::string	*stringValue;	///< value if of type DATA_STRING
      UBinary		*binary;	///< value if of type DATA_BINARY
      UList		*list;		///< value if of type DATA_LIST
      UObjectStruct	*object;	///< value if of type DATA_OBJ
      void           *storage;		///< internal
    };

    UValue();
    UValue(const UValue&);

    /// We use an operator , that behaves like an assignment.  The
    /// only difference is when the rhs is void, in which case it is
    /// the regular comma which is used.  This allows to write "uval,
    /// expr" to mean "compute expr and assign its result to uval,
    /// unless expr is void".
#define CTOR_AND_ASSIGN_AND_COMMA(Type)		\
    explicit UValue(Type);			\
    UValue& operator=(Type);			\
    UValue& operator, (Type rhs)		\
    {						\
      return *this = rhs;			\
    }

    UValue & operator, (const UValue &b)
    {
      return *this = b;
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

    operator ufloat() const;
    operator std::string() const;
    operator int() const {return (int)(ufloat)(*this);}
    operator unsigned int() const {return (unsigned int)(ufloat)(*this);}
    operator long() const {return (int)(ufloat)(*this);}
    operator unsigned long() const {return (unsigned int)(ufloat)(*this);}
    operator bool() const {return (bool)(int)(ufloat)(*this);}

    /// Deep copy.
    operator UBinary() const;

    /// Deep copy.
    operator UList() const;

    /// Shallow copy.
    operator UImage() const;

    /// Shallow copy.
    operator USound() const;

    UValue& operator=(const UValue&);


    /// This operator does nothing, but helps with the previous operator,.
    /// Indeed, when writing "uval, void_expr", the compiler complains
    /// about uval being evaluated for nothing.  Let's have it believe
    /// we're doing something...
    UValue& operator()() { return *this; }

    ~UValue();

    /// Parse an uvalue in current message+pos, returns pos of end of
    /// match -pos of error if error.
    int parse(const char* message,
	      int pos,
	      std::list<BinaryData> &bins,
	      std::list<BinaryData>::iterator &binpos);

    /// Print itself on \c s, and return it.
    std::ostream& print (std::ostream& s) const;
  };

  inline
  std::ostream&
  operator<<(std::ostream &s, const UValue &v)
  {
    return v.print (s);
  }

  /*----------.
  | Casters.  |
  `----------*/

  // For each Type, define an operator() that cast its UValue&
  // argument into Type.  We need partial specialization.
  template <typename Type>
  struct uvalue_caster
  {
  };

  // T -> UVar&  if T = UVar
  // T -> T      otherwise.
  template <typename T>
  struct uvar_ref_traits
  {
    typedef T type;
  };

  template <>
  struct uvar_ref_traits<UVar>
  {
    typedef UVar& type;
  };

  // Run the uvalue_caster<Type> on v.
  template <typename Type>
  typename uvar_ref_traits<Type>::type
  uvalue_cast (UValue& v)
  {
    return uvalue_caster<Type>()(v);
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
  struct uvalue_caster<UValue>
  {
    UValue operator()(UValue& v)
    {
      return v;
    }
  };

  // The following ones are defined in uvalue-common.cc.

  template <>
  struct uvalue_caster<UVar>
  {
    UVar& operator () (UValue& v);
  };


# define UVALUE_CASTER_DECLARE(Type)		\
  template <>					\
  struct uvalue_caster<Type>			\
  {						\
    Type operator () (UValue& v);		\
  };

  UVALUE_CASTER_DECLARE(UBinary)
  UVALUE_CASTER_DECLARE(UList)
  UVALUE_CASTER_DECLARE(UObjectStruct)
  UVALUE_CASTER_DECLARE(const char*)

# undef UVALUE_CASTER_DECLARE


# ifndef UOBJECT_NO_LIST_CAST
  template<typename I>
  struct uvalue_caster <std::list<I> >
  {
    std::list<I> operator()(UValue& v)
    {
      std::list<I> res;
      if (v.type != DATA_LIST)
	//cast just the element
	res.push_back(uvalue_cast<I*>(v));
      else
	for (int i = 0; i < v.list->size(); ++i)
	  res.push_back(uvalue_cast<I*>(*v.list->array[i]));
      return res;
    }
  };
# endif


} // namespace urbi

#endif // ! URBI_UVALUE_HH
