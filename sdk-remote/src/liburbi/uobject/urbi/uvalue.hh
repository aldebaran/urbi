/// \file urbi/uvalue.hh

// This file is part of UObject Component Architecture
// Copyright (c) 2007 Gostai S.A.S.
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
    int offset;
  };

  class UNamedValue
  {
  public:
    UNamedValue(const std::string& n, UValue* v)
      : val(v),name(n)
    {}
    UNamedValue()
      : val(0),name()
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
    operator UBinary() const; ///< deep copy
    operator UList() const; ///< deep copy
    operator UImage() const; ///< ptr copy
    operator USound() const; ///< ptr copy
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


} // end namespace urbi

#endif // ! URBI_UVALUE_HH
