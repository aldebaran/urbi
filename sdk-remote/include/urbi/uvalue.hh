/*
 * Copyright (C) 2007, 2008, 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/uvalue.hh
#ifndef URBI_UVALUE_HH
# define URBI_UVALUE_HH

# include <libport/warning-push.hh>

# include <vector>

# include <libport/hash.hh>
# include <libport/preproc.hh>
# include <libport/traits.hh>
# include <libport/ufloat.h>

# include <urbi/fwd.hh>
# include <urbi/export.hh>
# include <urbi/ubinary.hh>

namespace urbi
{

  /// If a kernel version was associated to this stream, its value.
  URBI_SDK_API
  int& kernelMajor(std::ostream& o);

  // UValue and other related types

  /// Possible value types a UValue can contain.
  enum UDataType
  {
    DATA_BINARY,
    DATA_DICTIONARY,
    DATA_DOUBLE,
    DATA_LIST,
    DATA_OBJECT,
    DATA_STRING,
    DATA_SLOTNAME,
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

    // Assign a container to the UList
    template<typename T>
    UList& operator=(const T& container);

    UList& operator=(UVar& v);

    // Transform the UList to a container.
    template<typename T>
    T as();

    // Append an element to the end.
    template<typename T>
    UList&
    push_back(const T& v);

    void pop_back();

    UValue& front();

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
  | UDictionary.  |
  `--------------*/
  typedef boost::unordered_map<std::string, UValue> UDictionary;


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
      UDictionary* dictionary;  ///< value if of type DATA_DICTIONARY
      UObjectStruct* object;    ///< value if of type DATA_OBJ
      void* storage;            ///< internal
    };

    UValue();
    UValue(const UValue&);

    ~UValue();

    UValue& operator=(const UValue&);
    /// Setter. If copy is false, binary data if present is not copied.
    /// This is dangerous, as the user must ensure that the source UValue
    /// lives longer than this one.
    UValue& set(const UValue&, bool copy=true);
    /// Delete content and reset type to void.
    void clear();
    /// A specific UValue used when we want to return an error.
    /// For instance, out-of-bound access returns this object.
    static UValue& error();

    /// We use an operator , that behaves like an assignment.  The
    /// only difference is when the rhs is void, in which case it is
    /// the regular comma which is used.  This allows to write "uval,
    /// expr" to mean "compute expr and assign its result to uval,
    /// unless expr is void".
    UValue& operator, (const UValue &b);

    /// Return a legible definition of UDataType
    const char* format_string() const;

#define CTOR_AND_ASSIGN_AND_COMMA(Type)           \
    explicit UValue(Type, bool copy=true);	  \
    UValue& operator=(Type);			  \
    UValue& operator,(Type rhs);

// Types convertibles to DATA_DOUBLE
#define URBI_DERIVED_NUMERIC_TYPES LIBPORT_LIST( \
  int, long, unsigned int, unsigned long, \
  unsigned long long, long long,)
#define URBI_NUMERIC_TYPES LIBPORT_LIST( \
  ufloat, int, long, unsigned int, unsigned long, \
  unsigned long long, long long,)

// Types convertibles to DATA_STRING
#define URBI_STRING_TYPES LIBPORT_LIST( \
  const char*, const void*, const std::string&,)

#define URBI_MISC_TYPES LIBPORT_LIST( \
  const UBinary&, const UList&, const UDictionary&, const UObjectStruct&, \
  const USound&, const UImage&,)

    LIBPORT_LIST_APPLY(CTOR_AND_ASSIGN_AND_COMMA, URBI_NUMERIC_TYPES)
    LIBPORT_LIST_APPLY(CTOR_AND_ASSIGN_AND_COMMA, URBI_STRING_TYPES)
    LIBPORT_LIST_APPLY(CTOR_AND_ASSIGN_AND_COMMA, URBI_MISC_TYPES)

#undef CTOR_AND_ASSIGN_AND_COMMA

#define CAST_OPERATOR(Type) \
    operator Type() const;

    LIBPORT_LIST_APPLY(CAST_OPERATOR, URBI_NUMERIC_TYPES)
#undef CAST_OPERATOR

    operator std::string() const;
    operator bool() const;

    /// Accessor. Gives us an implicit operator UBinary() const
    operator const UBinary&() const;

    /// Deep copy.
    operator UList() const;

    /// Deep copy.
    operator UDictionary() const;

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
	      const binaries_type& bins,
	      binaries_type::const_iterator& binpos);

    /// Print itself on \c s, and return it.
    std::ostream& print(std::ostream& s) const;

    // Huge hack.
    static const bool copy = true;
  };

  inline
  std::ostream&
  operator<<(std::ostream& s, const UValue& v);

  inline
  std::ostream&
  operator<<(std::ostream& s, const UDictionary& d);


  /*----------.
  | Casters.  |
  `----------*/

  // For each Type, define an operator() that casts its UValue&
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

  /* NM: Why exactly are we moving the const away?
   * I'm dropping it for UValue as it makes unnecessary copy.
   */

   template<typename T> struct uvalue_cast_return_type
   {
     typedef typename libport::traits::remove_reference<T>::type type;
   };

   template<> struct uvalue_cast_return_type<const UValue&>
   {
     typedef const UValue& type;
   };

  template <typename Type>
  typename uvar_ref_traits<typename uvalue_cast_return_type<Type>::type>::type
  uvalue_cast (UValue& v);

} // namespace urbi

#define SYNCLINE_PUSH()                                         \
  "//#push " BOOST_PP_STRINGIZE(__LINE__) " \"" __FILE__ "\"\n"

#define SYNCLINE_POP()                          \
  "//#pop\n"

#define SYNCLINE_WRAP(...)                      \
  SYNCLINE_PUSH()                               \
  __VA_ARGS__                                   \
  "\n"                                          \
  SYNCLINE_POP()



# include <urbi/uvalue.hxx>

# include <libport/warning-pop.hh>

#endif // ! URBI_UVALUE_HH
