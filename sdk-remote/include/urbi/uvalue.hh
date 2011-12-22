/*
 * Copyright (C) 2007, 2008, 2009, 2010, 2011, Gostai S.A.S.
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

  /*--------------------------.
  | Free standing functions.  |
  `--------------------------*/

  /// If a kernel version was associated to this stream, its value.
  URBI_SDK_API
  int& kernelMajor(std::ostream& o);

  /// Issue a syncline.
  /// \param srcdir   the path from here to the top-srcdir.
  ///                 Will be removed from file.
  /// \param file     __FILE__
  /// \param line     __LINE__
  URBI_SDK_API
  std::string
  syncline_push(const std::string& srcdir, std::string file, unsigned line);

#define SYNCLINE_PUSH()                                         \
  "//#push " BOOST_PP_STRINGIZE(__LINE__) " \"" __FILE__ "\"\n"

#define SYNCLINE_POP()                          \
  "//#pop\n"

#ifndef __SRCDIR__
# define __SRCDIR__ ""
#endif

#define SYNCLINE_WRAP(...)                                      \
  (::urbi::syncline_push(__SRCDIR__, __FILE__, __LINE__)        \
   + libport::format(__VA_ARGS__)                               \
   + "\n"                                                       \
   SYNCLINE_POP())



  // UValue and other related types

  /// Possible value types a UValue can contain.
  enum UDataType
  {
    DATA_BINARY = 0,
    DATA_DICTIONARY = 1,
    DATA_DOUBLE = 2,
    DATA_LIST = 3,
    // No longer supported. DATA_OBJECT = 4,
    DATA_STRING = 5,
    DATA_SLOTNAME = 6,
    DATA_VOID = 7,
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

    /// Iteration.
    typedef std::vector<UValue*> list_type;
    typedef list_type::iterator iterator;
    typedef list_type::const_iterator const_iterator;
    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;

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
    list_type array;

  private:
    void clear();
    size_t offset;
    friend class UValue;
  };

  URBI_SDK_API
  std::ostream& operator<< (std::ostream& o, const UList& t);


  /*--------------.
  | UDictionary.  |
  `--------------*/

  typedef boost::unordered_map<std::string, UValue> UDictionary;

  URBI_SDK_API
  std::ostream& operator<<(std::ostream& s, const UDictionary& d);


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


  /*---------.
  | UValue.  |
  `---------*/

  /** Container for a value that handles all types known to URBI.
   */
  class URBI_SDK_API UValue
  {
  public:
    UDataType type;

    ufloat val;                 ///< value if of type DATA_DOUBLE
    union
    {
      std::string* stringValue; ///< value if of type DATA_STRING
      UBinary* binary;          ///< value if of type DATA_BINARY
      UList* list;              ///< value if of type DATA_LIST
      UDictionary* dictionary;  ///< value if of type DATA_DICTIONARY
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
#define URBI_DERIVED_NUMERIC_TYPES                     \
  LIBPORT_LIST(int, long, unsigned int, unsigned long, \
               unsigned long long, long long,)

#define URBI_NUMERIC_TYPES                                     \
  LIBPORT_LIST(ufloat, int, long, unsigned int, unsigned long, \
               unsigned long long, long long,)

// Types convertibles to DATA_STRING
#define URBI_STRING_TYPES \
  LIBPORT_LIST(const char*, const void*, const std::string&,)

#define URBI_MISC_TYPES                                          \
  LIBPORT_LIST(const UBinary&, const UList&, const UDictionary&, \
               const USound&, const UImage&,)

# ifndef SWIG
    LIBPORT_LIST_APPLY(CTOR_AND_ASSIGN_AND_COMMA, URBI_NUMERIC_TYPES)
    LIBPORT_LIST_APPLY(CTOR_AND_ASSIGN_AND_COMMA, URBI_STRING_TYPES)
    LIBPORT_LIST_APPLY(CTOR_AND_ASSIGN_AND_COMMA, URBI_MISC_TYPES)
# else
    // UFloats.
    CTOR_AND_ASSIGN_AND_COMMA(ufloat);
    CTOR_AND_ASSIGN_AND_COMMA(int);
    CTOR_AND_ASSIGN_AND_COMMA(long);
    CTOR_AND_ASSIGN_AND_COMMA(unsigned int);
    CTOR_AND_ASSIGN_AND_COMMA(unsigned long);
    CTOR_AND_ASSIGN_AND_COMMA(unsigned long long);
    CTOR_AND_ASSIGN_AND_COMMA(long long);

    // Strings.
    CTOR_AND_ASSIGN_AND_COMMA(const char*);
    CTOR_AND_ASSIGN_AND_COMMA(const void*);
    CTOR_AND_ASSIGN_AND_COMMA(const std::string&);

    // Others.
    CTOR_AND_ASSIGN_AND_COMMA(const UBinary&);
    CTOR_AND_ASSIGN_AND_COMMA(const UList&);
    CTOR_AND_ASSIGN_AND_COMMA(const UDictionary&);
    CTOR_AND_ASSIGN_AND_COMMA(const USound&);
    CTOR_AND_ASSIGN_AND_COMMA(const UImage&);
# endif

#undef CTOR_AND_ASSIGN_AND_COMMA

#define CAST_OPERATOR(Type) \
    operator Type() const;

# ifndef SWIG
    LIBPORT_LIST_APPLY(CAST_OPERATOR, URBI_NUMERIC_TYPES)
# else
    operator ufloat() const;
    operator std::string() const;
    operator int() const;
    operator unsigned int() const;
    operator long() const;
    operator unsigned long() const;
    operator bool() const;
# endif
#undef CAST_OPERATOR

#ifdef DOXYGEN
    // Doxygens needs a prototype for ufloat.
    operator ufloat() const;
#endif

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

  std::ostream&
  operator<<(std::ostream& s, const UValue& v);


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

#define URBI_STRUCT_CAST_FIELD(_, Cname, Field)                         \
  if (libport::mhas(dict, BOOST_PP_STRINGIZE(Field)))                   \
    uvalue_cast_bounce(res.Field, dict[BOOST_PP_STRINGIZE(Field)]);     \
  else                                                                  \
    GD_WARN("Serialized data for " #Cname "is missing field"            \
            BOOST_PP_STRINGIZE(Field));                                 \

#define URBI_STRUCT_BCAST_FIELD(_, Cname, Field)        \
  dict[BOOST_PP_STRINGIZE(Field)], c.Field;


#define URBI_REGISTER_STRUCT(Cname, ...)                                \
  namespace urbi                                                        \
  {                                                                     \
    template<>                                                          \
      struct uvalue_caster<Cname>                                       \
    {                                                                   \
      Cname operator()(UValue& v)                                       \
      {                                                                 \
        if (v.type != DATA_DICTIONARY)                                  \
          throw std::runtime_error("invalid cast to " #Cname "from "    \
                                   + string_cast(v));                   \
        UDictionary& dict = *v.dictionary;                              \
        Cname res;                                                      \
        LIBPORT_VAARGS_APPLY(URBI_STRUCT_CAST_FIELD, Cname, __VA_ARGS__); \
        return res;                                                     \
      }                                                                 \
    };                                                                  \
                                                                        \
    static UValue& operator,(UValue& v, const Cname &c)                 \
    {                                                                   \
      v = UDictionary();                                                \
      UDictionary& dict = *v.dictionary;                                \
      dict["$sn"] = BOOST_PP_STRINGIZE(Cname);                          \
      LIBPORT_VAARGS_APPLY(URBI_STRUCT_BCAST_FIELD, Cname, __VA_ARGS__); \
      return v;                                                         \
    }                                                                   \
  }

/** Packed data structure.
 *  This template class behaves exactly as a std::vector, except it gets
 *  serialized efficiently in a Binary, whereas std::vector is converted in
 *  an urbiscript list.
 */
template<typename T>
class UPackedData: public std::vector<T>
{
public:
  UPackedData()
    {};
  UPackedData(const std::vector<T>& src)
    : std::vector<T>(src)
    {};
  template<typename I>
  UPackedData(I begin, I end)
    : std::vector<T>(begin, end)
  {};
};

# include <urbi/uvalue.hxx>

# include <libport/warning-pop.hh>

#endif // ! URBI_UVALUE_HH
