/// \file urbi/uvalue.hh

// This file is part of UObject Component Architecture
// Copyright (c) 2007, 2008, 2009 Gostai S.A.S.
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

# include <libport/select-ref.hh>
# include <libport/ufloat.h>

# include <urbi/fwd.hh>
# include <urbi/export.hh>
# include <urbi/ubinary.hh>

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

  /*--------.
  | UList.  |
  `--------*/

  /// Class storing URBI List type
  class URBI_SDK_API UList
  {
  public:
    UList();
    UList(const UList &b);
    ~UList();

    UList& operator=(const UList &b);

    UValue& operator[](size_t i);
    const UValue& operator[](size_t i) const;

    size_t size() const;
    void setOffset(size_t n);

    std::ostream& print(std::ostream& o) const;

    // The actual contents.
    std::vector<UValue*> array;

  private:
    void clear();
    size_t offset;
  };

  URBI_SDK_API
  std::ostream& operator<< (std::ostream& o, const UList& t);


  /*--------------.
  | UNamedValue.  |
  `--------------*/

  class URBI_SDK_API UNamedValue
  {
  public:
    UNamedValue(const std::string& n = "", UValue* v = 0);
    // Used on errors.
    static UNamedValue& error();
    std::string name;
    UValue* val;
  };


  /*----------------.
  | UObjectStruct.  |
  `----------------*/

  class URBI_SDK_API UObjectStruct
  {
  public:
    UObjectStruct();
    UObjectStruct(const UObjectStruct &b);
    UObjectStruct& operator=(const UObjectStruct &b);
    ~UObjectStruct();

    /// Return UValue::error() on errors.
    UValue& operator[](const std::string& s);

    /// Return UNamedValue::error() on errors.
    const UNamedValue& operator[](size_t i) const;
    UNamedValue& operator [](size_t i);

    size_t size() const;

    std::ostream& print(std::ostream& o) const;

    std::string refName;
    std::vector<UNamedValue> array;
  };

  URBI_SDK_API
  std::ostream& operator<< (std::ostream& o, const UObjectStruct& t);


  /*---------.
  | UValue.  |
  `---------*/

  /** Container for a value that handles all types known to URBI.
   */
  class URBI_SDK_API UValue
  {
  public:
    UDataType       type;
    ufloat          val;  ///< value if of type DATA_DOUBLE
    union
    {
      std::string* stringValue; ///< value if of type DATA_STRING
      UBinary* binary;          ///< value if of type DATA_BINARY
      UList* list;              ///< value if of type DATA_LIST
      UObjectStruct* object;    ///< value if of type DATA_OBJ
      void* storage;            ///< internal
    };

    UValue();
    UValue(const UValue&);

    ~UValue();

    UValue& operator=(const UValue&);

    /// A specific UValue used when we want to return an error.
    /// For instance, out-of-bound access returns this object.
    static UValue& error();

    /// We use an operator , that behaves like an assignment.  The
    /// only difference is when the rhs is void, in which case it is
    /// the regular comma which is used.  This allows to write "uval,
    /// expr" to mean "compute expr and assign its result to uval,
    /// unless expr is void".
    UValue& operator, (const UValue &b);

#define CTOR_AND_ASSIGN_AND_COMMA(Type)		\
    explicit UValue(Type);			\
    UValue& operator=(Type);			\
    UValue& operator,(Type rhs)

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
    operator int() const;
    operator unsigned int() const;
    operator long() const;
    operator unsigned long() const;
    operator bool() const;

    /// Deep copy.
    operator UBinary() const;

    /// Deep copy.
    operator UList() const;

    /// Shallow copy.
    operator UImage() const;

    /// Shallow copy.
    operator USound() const;

    /// This operator does nothing, but helps with the previous operator,.
    /// Indeed, when writing "uval, void_expr", the compiler complains
    /// about uval being evaluated for nothing.  Let's have it believe
    /// we're doing something...
    UValue& operator()();

    /// Parse an uvalue in current message+pos, returns pos of end of
    /// match -pos of error if error.
    int parse(const char* message,
	      int pos,
	      const std::list<BinaryData>& bins,
	      std::list<BinaryData>::const_iterator& binpos);

    /// Print itself on \c s, and return it.
    std::ostream& print(std::ostream& s) const;
  };

  inline
  std::ostream&
  operator<<(std::ostream& s, const UValue& v);



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
  typename uvar_ref_traits<typename libport::unref_traits<Type>::type>::type
  uvalue_cast (UValue& v);

} // namespace urbi

# include <urbi/uvalue.hxx>

#endif // ! URBI_UVALUE_HH
