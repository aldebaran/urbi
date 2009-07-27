#ifndef OBJECT_CXX_OBJECT_HH
# define OBJECT_CXX_OBJECT_HH

# include <vector>

# include <boost/optional.hpp>

# include <object/object.hh>


#define URBI_CXX_OBJECT(Name)                                           \
public:                                                                 \
  static void initialize(object::CxxObject::Binder<Name>& binder);      \
  static const std::string& type_name();                                \
  virtual std::string type_name_get() const;                            \
  static object::rObject proto;                                         \
  virtual bool valid_proto(const Object& o) const;                      \
private:                                                                \
  friend class TypeInitializer<Name>;                                   \
  static object::rObject proto_make();                                  \

#define URBI_CXX_OBJECT_REGISTER(Name)                                  \
  const std::string& Name::type_name()                                  \
  {                                                                     \
    static std::string res = #Name;                                     \
    return res;                                                         \
  }                                                                     \
                                                                        \
  object::rObject Name::proto;                                          \
                                                                        \
  std::string                                                           \
  Name::type_name_get() const                                           \
  {                                                                     \
    return type_name();                                                 \
  }                                                                     \
                                                                        \
  bool Name::valid_proto(const Object& o) const                         \
  {                                                                     \
    return dynamic_cast<const Name*>(&o);                               \
  }                                                                     \
  struct Name ## _register__                                            \
  {                                                                     \
    Name ## _register__() {object::CxxObject::add<Name>();}             \
  };                                                                    \
  static Name ## _register__ Name ## _registered__;                     \



namespace object
{
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

    class Initializer
    {
    public:
      Initializer(rObject& tgt);
      virtual ~Initializer();
      virtual rObject make_class() = 0;
      virtual void create() = 0;
      virtual libport::Symbol name() = 0;
    protected:
      rObject& res_;
    };

    template <typename T>
    class TypeInitializer: public Initializer
    {
    public:
      TypeInitializer();
      virtual rObject make_class();
      virtual void create();
      virtual libport::Symbol name();
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
  void type_check(const rObject& o, const rObject& exp,
                  boost::optional<unsigned> idx = boost::optional<unsigned>());

  /// Same as above, but check first with a dynamic_cast in order to handle
  /// atoms more efficiently.
  template<typename T>
  URBI_SDK_API
  void type_check(const rObject& o,
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

# if ! defined(OBJECT_EXECUTABLE_HH)                                    \
  && ! defined(OBJECT_FLOAT_CLASS_HH)                                   \
  && ! defined(OBJECT_LIST_CLASS_HH)                                    \
  && ! defined(OBJECT_STRING_CLASS_HH)                                  \
  && ! defined(OBJECT_TAG_CLASS_HH)
#  include <object/cxx-object.hxx>
# endif

#endif
