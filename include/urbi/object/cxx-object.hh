/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
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

# include <boost/optional.hpp>

# include <urbi/object/object.hh>


#define URBI_CXX_OBJECT_(Name)                                          \
public:                                                                 \
  static const ::std::string& type_name();                              \
  virtual ::std::string type_name_get() const;                          \
  static ::libport::intrusive_ptr<Name> proto;                          \
  virtual bool valid_proto(const ::urbi::object::Object& o) const;      \
private:                                                                \
  friend class ::urbi::object::CxxObject::TypeInitializer<Name>;        \
  Name(const ::urbi::object::FirstPrototypeFlag&);                      \
public:                                                                 \
  static void initialize(::urbi::object::CxxObject::Binder<Name>&)      \

#define URBI_CXX_OBJECT(Name)                   \
  URBI_CXX_OBJECT_(Name)                        \
  {}                                            \

#define URBI_CXX_OBJECT_REGISTER(Name)                          \
  const std::string& Name::type_name()                          \
  {                                                             \
    static std::string res = #Name;                             \
    return res;                                                 \
  }                                                             \
                                                                \
  ::libport::intrusive_ptr<Name> Name::proto;                   \
                                                                \
  std::string                                                   \
  Name::type_name_get() const                                   \
  {                                                             \
    return type_name();                                         \
  }                                                             \
                                                                \
  bool Name::valid_proto(const ::urbi::object::Object& o) const \
  {                                                             \
    return dynamic_cast<const Name*>(&o);                       \
  }                                                             \
                                                                \
  struct Name ## _register__                                    \
  {                                                             \
    Name ## _register__()                                       \
    {                                                           \
      ::urbi::object::CxxObject::add<Name>();                   \
    }                                                           \
  };                                                            \
                                                                \
  static Name ## _register__ Name ## _registered__;             \
                                                                \
  Name::Name(const ::urbi::object::FirstPrototypeFlag&)


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

      /// Bind all registered objects.
      /** This function should be called once to bind C++ objects.
       *  \param global Where to store the new classes.
       */
      static void initialize(rObject global);
      static void create();
      static void cleanup();

      /// Push initializer for class T to the back of the initialization list.
      template<typename T>
        static void push_initializer_to_back();

      /// Register a C++ class to be bound on the urbi side.
      /** \param T      The class to bind.
       *  \param name   Name of the class on the Urbi side.
       *  \param tgt    Where to store the result.
       */
      template<typename T>
        static bool add();

      virtual std::string type_name_get() const = 0;

      protected:

      class URBI_SDK_API Initializer
      {
      public:
        Initializer();
        virtual ~Initializer();
        virtual rObject make_class() = 0;
        virtual void create() = 0;
        virtual libport::Symbol name() = 0;
      };

      template <typename T>
        class TypeInitializer: public Initializer
      {
      public:
        TypeInitializer();
        virtual rObject make_class();
        virtual void create();
        virtual libport::Symbol name();
      protected:
        libport::intrusive_ptr<T>& res_;
      };

      typedef std::list<Initializer*> initializers_type;
      static initializers_type& initializers_get();

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
        void operator()(const libport::Symbol& name, M method);
        template <typename A>
        void var(const libport::Symbol& name, A (T::*attr));
        template <typename A>
        void var(const libport::Symbol& name, A* (T::*ref)());
        rObject proto() { return tgt_; }

      private:
        Binder(rObject tgt);
        // This method is allowed to construct a Binder.
        // VC++ is not a friend of friend method templates, so friend the class.
        friend class TypeInitializer<T>;
        rObject tgt_;
      };

    };

    /// Raise an exception if \a o is not a \a exp. If \a idx is given,
    /// report the type error for argument \a idx.
    URBI_SDK_API
    void
    type_check(const rObject& o, const rObject& exp,
               boost::optional<unsigned> idx = boost::optional<unsigned>());

    /// Same as above, but check first with a dynamic_cast in order to handle
    /// atoms more efficiently.
    template<typename T>
    URBI_SDK_API
    libport::intrusive_ptr<T>
    type_check(const rObject& o,
               boost::optional<unsigned> idx = boost::optional<unsigned>());

    /// Throw an exception if formal != effective.
    /// \note: \c self is included in the count.
    URBI_SDK_API
    void check_arg_count(unsigned effective, unsigned formal);

    /// Same as above, with a minimum and maximum number of
    /// formal parameters.
    URBI_SDK_API
    void check_arg_count(unsigned effective,
                         unsigned minformal, unsigned maxformal);

  }
}

# if ! defined OBJECT_EXECUTABLE_HH             \
      && ! defined OBJECT_FLOAT_HH              \
      && ! defined OBJECT_LIST_HH               \
      && ! defined OBJECT_STRING_HH             \
      && ! defined OBJECT_TAG_HH
#  include <urbi/object/cxx-object.hxx>
# endif

#endif // ! OBJECT_CXX_OBJECT_HH
