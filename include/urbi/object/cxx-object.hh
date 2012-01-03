/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_CXX_OBJECT_HH
# define OBJECT_CXX_OBJECT_HH

# include <vector>

# include <boost/functional/hash.hpp>

# include <libport/preproc.hh>

# include <urbi/kernel/userver.hh>
# include <urbi/object/enumeration.hh>
# include <urbi/object/object.hh>
# include <urbi/version-check.hh>

#define URBI_CXX_OBJECT(Name, Parent)                                   \
  private:                                                              \
    Name(const ::urbi::object::FirstPrototypeFlag&);                    \
    friend class CxxObject;                                             \
  public:                                                               \
    static const ::std::string& type_name();                            \
    virtual ::std::string type_name_get() const;                        \
    static ::libport::intrusive_ptr<Name> proto;                        \
    virtual bool valid_proto(const ::urbi::object::Object& o) const;    \
    virtual void* as_dispatch_(const std::type_info* requested);        \
    ATTRIBUTE_ALWAYS_INLINE                                             \
    bool                                                                \
    as_check_(const std::type_info* req)                                \
    {                                                                   \
      return typeid(Name) == *req || Parent::as_check_(req);            \
    }                                                                   \

#define URBI_CXX_OBJECT_REGISTER(Name, ...)                   \
  URBI_CXX_OBJECT_REGISTER_(Name, LIBPORT_LIST(__VA_ARGS__,))

#define URBI_CXX_OBJECT_REGISTER_(Name, Attrs)                \
  URBI_CXX_OBJECT_REGISTER__(Name, LIBPORT_LIST_NTH(0, Attrs))

#define URBI_CXX_OBJECT_REGISTER__(Name, Ns)                            \
  static void                                                           \
  LIBPORT_CAT(urbi_cxx_object_register_##Name##_, __LINE__)()           \
  {                                                                     \
    URBI_CHECK_SDK_VERSION(#Name);                                      \
    ::urbi::object::CxxObject::add<Name>(BOOST_PP_STRINGIZE(Ns));       \
  }                                                                     \
  URBI_INITIALIZATION_REGISTER                                          \
  (LIBPORT_CAT(urbi_cxx_object_register_##Name##_, __LINE__))

#define URBI_CXX_OBJECT_INIT(Name)                                      \
  const std::string& Name::type_name()                                  \
  {                                                                     \
    static std::string res = #Name;                                     \
    return res;                                                         \
  }                                                                     \
                                                                        \
  ::libport::intrusive_ptr<Name> Name::proto;                           \
                                                                        \
  std::string                                                           \
  Name::type_name_get() const                                           \
  {                                                                     \
    return type_name();                                                 \
  }                                                                     \
                                                                        \
  bool                                                                  \
  Name::valid_proto(const ::urbi::object::Object& o) const              \
  {                                                                     \
    return o.as<Name>();                                                \
  }                                                                     \
                                                                        \
  void*                                                                 \
  Name::as_dispatch_(const std::type_info* requested)                   \
  {                                                                     \
    return this->as_check_(requested) ? this : 0;                       \
  }                                                                     \
  Name::Name(const ::urbi::object::FirstPrototypeFlag&)

#define URBI_CXX_OBJECT_REGISTER_INIT(Name, ...)                   \
  URBI_CXX_OBJECT_REGISTER_INIT_(Name, LIBPORT_LIST(__VA_ARGS__,))

#define URBI_CXX_OBJECT_REGISTER_INIT_(Name, Attrs) \
  URBI_CXX_OBJECT_REGISTER_(Name, Attrs);           \
  URBI_CXX_OBJECT_INIT(Name)

namespace urbi
{
  namespace object
  {
    struct FirstPrototypeFlag {};

    /// Base class for Urbi bound C++ classes.
    class URBI_SDK_API CxxObject: public Object
    {
    public:

      /// Build a CxxObject.
      CxxObject();

      /// Register a C++ class to be bound on the urbi side.
      /** \param T       The class to bind.
       *  \param ns      Where to store the class.
       */
      template<typename T>
      static void add(const std::string& ns = "");

      virtual std::string type_name_get() const = 0;

    protected:

      public:
      /// Functor to bind methods on the urbi side.
      /** An instance of this class is given to the static initialize
       *  method of the bound classes. It can then be used to bind
       *  method and/or attributes on the Urbi side.
       */
      template <typename T>
        class Binder
      {
      public:
        /// Bind \a method with \a name
        template <typename M>
        void operator()(libport::Symbol name, M method);
        template <typename A>
        void var(libport::Symbol name, A (T::*attr));
        template <typename A>
        void var(libport::Symbol name, A* (T::*ref)());
        rObject proto() { return tgt_; }

      private:
        friend class CxxObject;
        Binder(rObject tgt);
        rObject tgt_;
      };

    };

    /// Raise an exception if \a o is not a \a exp. If \a idx is given,
    /// report the type error for argument \a idx.
    URBI_SDK_API
    void
    type_check(const rObject& o, const rObject& exp);

    /// Same as above, but check first with a dynamic_cast in order to handle
    /// atoms more efficiently.
    template<typename T>
    URBI_SDK_API
    libport::intrusive_ptr<T>
    type_check(const rObject& o);

    /// Throw an exception if formal != effective.
    /// \note: \c self is included in the count.
    URBI_SDK_API
    void check_arg_count(unsigned effective, unsigned formal);

    /// Same as above, with a minimum and maximum number of
    /// formal parameters.
    URBI_SDK_API
    void check_arg_count(unsigned effective,
                         unsigned minformal, unsigned maxformal);

    /// Resolve the namespace part of an Urbi object name.
    /* The namespace component is stripped from named, created and
     * returned.  I.e., given "Foo.Bar.Baz", Foo.Bar will be created
     * and returned, and name will equal "Baz". Namespaces are rooted
     * in Global.
     */
    URBI_SDK_API
    rObject resolve_namespace(std::string& path);
  }

  typedef boost::function0<void> Initialization;
  URBI_SDK_API
  int initialization_register(const Initialization& action);

# define URBI_INITIALIZATION_REGISTER(Action)                           \
                                                                        \
  static                                                                \
  int LIBPORT_CAT(_urbi_initialization, __LINE__)()                     \
  {                                                                     \
    ::urbi::initialization_register(Action);                            \
    return 0xbadf00d;                                                   \
  }                                                                     \
                                                                        \
  static inline                                                         \
  int LIBPORT_CAT(_urbi_initialization_bouncer, __LINE__)();            \
                                                                        \
  static int LIBPORT_CAT(_urbi_initialization_register_, __LINE__) =    \
    LIBPORT_CAT(_urbi_initialization_bouncer, __LINE__)();              \
                                                                        \
  static inline                                                         \
  int LIBPORT_CAT(_urbi_initialization_bouncer, __LINE__)()             \
  {                                                                     \
    static int dummy = LIBPORT_CAT(_urbi_initialization, __LINE__)();   \
    /* Avoid unused warnings */                                         \
    LIBPORT_CAT(_urbi_initialization_register_, __LINE__) =             \
      LIBPORT_CAT(_urbi_initialization_register_, __LINE__);            \
    return dummy;                                                       \
  }                                                                     \


}

# if ! defined OBJECT_EXECUTABLE_HH             \
      && ! defined OBJECT_FLOAT_HH              \
      && ! defined OBJECT_LIST_HH               \
      && ! defined OBJECT_STRING_HH             \
      && ! defined OBJECT_TAG_HH
#  include <urbi/object/cxx-object.hxx>
# endif

#endif // ! OBJECT_CXX_OBJECT_HH
