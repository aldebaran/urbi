/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/uobject.hh

#ifndef URBI_UOBJECT_HH
# define URBI_UOBJECT_HH

# include <libport/warning-push.hh>

# include <string>

# include <libport/cmath>
# include <libport/compiler.hh>
# include <libport/fwd.hh>
# include <libport/preproc.hh>
# include <libport/thread-pool.hh>
# include <libport/ufloat.h>
# include <libport/utime.hh>

# include <urbi/export.hh>
# include <urbi/fwd.hh>
# include <urbi/kernel-version.hh>
# include <urbi/input-port.hh>
# include <urbi/ucallbacks.hh>
# include <urbi/uevent.hh>
# include <urbi/utimer-callback.hh>
# include <urbi/uvar.hh>
# include <urbi/uobject-hub.hh>
# include <urbi/ucontext.hh>

# ifndef URBI_SDK_DEPRECATED
#  ifdef BUILDING_URBI_SDK
#   define URBI_SDK_DEPRECATED
#  else
#   define URBI_SDK_DEPRECATED ATTRIBUTE_DEPRECATED
#  endif
# endif

// Tell our users that it is fine to use void returning functions.
# define USE_VOID 1

# define URBI_UOBJECT_VERSION 2

/** Bind a variable to an object.

 These macros can only be called from within a class inheriting from
 UObject.  They bind the UVar X within the object to a variable
 with Uname as variable name in the corresponding Urbi object.  */
# define UBindVarRename(Obj, X, Uname)          \
  (X).init(__name, Uname, ctx_)

# define UBindVar(Obj, X)                       \
  UBindVarRename(Obj, X, #X)

/// Bind multiple variables in one call
# define UBindVars(Obj, ...)                            \
  LIBPORT_VAARGS_APPLY(URBI_BINDVARS, Obj, __VA_ARGS__)


/** Bind an event to an object.

 These macros can only be called from within a class inheriting from
 UObject.  They bind the UEvent X within the object to a variable
 with uName as variable name in the corresponding Urbi object.  */
# define UBindEventRename(Obj, X, Uname)        \
  (X).init(__name, Uname, ctx_)

# define UBindEvent(Obj,X)                      \
  UBindEventRename(Obj, X, #X)

# define UBindEvents(Obj, ...)                                  \
  LIBPORT_VAARGS_APPLY(URBI_BINDEVENTS, Obj, __VA_ARGS__)

/** This macro inverts a UVar in/out accesses.

 After this call is made, writes by this module affect the sensed
 value, and reads read the target value. Writes by other modules
 and Urbi code affect the target value, and reads get the sensed
 value. Without this call, all operations affect the same
 underlying variable.  */
# define UOwned(X)                              \
  (X).setOwned()

/// Backward compatibility.
# define USensor(X)                             \
  UOwned(X)

/// Call me inside your class declaration if you need a LOCK_CLASS task lock.
# define CREATE_CLASS_LOCK                      \
  virtual                                       \
  libport::ThreadPool::rTaskLock                \
  getClassTaskLock()                            \
  {                                             \
    static libport::ThreadPool::rTaskLock       \
      tl(new libport::ThreadPool::TaskLock);    \
    return tl;                                  \
  }

/// Internal, might change, do not use it directly.
# define URBI_CREATE_CALLBACK(Kind, ...)                        \
  ::urbi::createUCallback(*this, 0, #Kind, this, __VA_ARGS__)

/** Bind the function X in current Urbi object to the C++ member
 function of same name.  The return value and arguments must be of
 a basic integral or floating types, char *, std::string, UValue,
 UBinary, USound or UImage, or any type that can cast to/from
 UValue.  */
# define UBindFunctionRename(Obj, X, Uname)                     \
  URBI_CREATE_CALLBACK(function, (&Obj::X), __name + "." Uname)

# define UBindFunction(Obj, X)                  \
  UBindFunctionRename(Obj, X, #X)

# define UBindFunctions(Obj, ...)                               \
  LIBPORT_VAARGS_APPLY(URBI_BINDFUNCTIONS, Obj, __VA_ARGS__)

/** Bind the function so that it gets executed in a separate thread.
 *  @param Obj the UObject class name
 *  @param X the unquoted function name
 *  @param Uname the urbiscript name of the method
 *  @param LockMode (LockMode or LockSpec) which lock to use. This lock can be
 *         used to prevent multiple parallel execution of functions.
 *         If you use only a LockMode, operations that cannot execute in
 *         parallel will be queued. A LockSpec can be used to specify the
 *         maximum queue size.
 */
# define UBindThreadedFunctionRename(Obj, X, Uname, LockMode)           \
  UBindFunctionRename(Obj, X, Uname)->setAsync(getTaskLock(LockMode, Uname))

# define UBindThreadedFunction(Obj, X, LockMode)        \
  UBindThreadedFunctionRename(Obj, X, #X, LockMode)


/** Registers a function X in current object that will be called each
 time the event of same name is triggered. The function will be
 called only if the number of arguments match between the function
 prototype and the Urbi event.  */
# define UAt(Obj, X)                                            \
  URBI_CREATE_CALLBACK(event, (&Obj::X), __name + "." #X)

/// Same as UAt() but executes the code in a separate thread.
# define UThreadedAt(Obj, X, LockMode)                  \
  UAt(Obj, X)->setAsync(getTaskLock(LockMode, #X))


/** Registers a function \a X in current object that will be called each
 * time the event of same name is triggered, and a function fun called
 * when the event ends. The function will be called only if the number
 * of arguments match between the function prototype and the Urbi
 * event.
 */
# define UAtEnd(Obj, X, Fun)					        \
  URBI_CREATE_CALLBACK(eventend, (&Obj::X), (&Obj::Fun), __name + "." #X)

# define UThreadedAtEnd(Obj, X, Fun, LockMode)                  \
  UAtEnd(Obj, X, Fun)->setAsync(getTaskLock(LockMode, #X))

/// Register current object to the UObjectHub named \a Hub.
# define URegister(Hub)						\
  do {								\
    objecthub = ::urbi::baseURBIStarterHub::find(#Hub);         \
    if (objecthub)						\
      objecthub->addMember(this);				\
    else                                                        \
    {                                                           \
      GD_CATEGORY(Urbi.UObject);                                \
      GD_FERROR("Error: hub name '%s' is unknown",  #Hub);      \
    }                                                           \
  } while (0)


//macro to send urbi commands
# ifndef URBI
/// Send unquoted Urbi commands to the server.
/// Add an extra layer of parenthesis for safety.
#   define URBI(A)                              \
  uobject_unarmorAndSend(# A)
# endif


/// Send \a Args (which is given to a stream and therefore can use <<)
/// to \a C.
# define URBI_SEND_C(C, Args)			\
  do {						\
    libport::BlockLock bl((C).sendBufferLock);  \
    (C) << Args;                                \
  } while (false)

/// Send \a Args (which is given to a stream and therefore can use <<)
/// to \a C, then flush.
# define URBI_SEND_C_FLUSH(C, Args)		\
  do {						\
    libport::BlockLock bl((C).sendBufferLock);  \
    (C) << Args << std::endl;                   \
  } while (false)

/// Send \a Args (which is given to a stream and therefore can use <<)
/// to the server.
# define URBI_SEND(Args)			\
  URBI_SEND_C(URBI(()), Args)

/// Send "\a Args ; \n" to \a C.
# define URBI_SEND_COMMAND_C(C, Args)		\
  URBI_SEND_C_FLUSH(C, Args << ';')

# define URBI_SEND_COMMAND(Args)		\
  URBI_SEND_COMMAND_C(URBI(()), Args)

/** Send "\a Args | \n" to \a C.
  * \b Warning: nothing is executed until a ';' or ',' is sent.
  * Do not endl, we do not want to flush.
  */
# define URBI_SEND_PIPED_COMMAND_C(C, Args)     \
  URBI_SEND_C(C, Args << "|\n")

# define URBI_SEND_PIPED_COMMAND(Args)          \
  URBI_SEND_PIPED_COMMAND_C(URBI(()), Args)

# define URBI_SEND_COMMA_COMMAND_C(C, Args)     \
  URBI_SEND_C_FLUSH(C, Args << ',')

# define URBI_SEND_COMMA_COMMAND(Args)          \
  URBI_SEND_COMMA_COMMAND_C(URBI(()), Args)

namespace urbi
{

  /** Locking model.
   * This enum is used in UBindThreadedFunction to tell what locking model
   * should be used.
   */
  enum LockMode
  {
    LOCK_NONE,      ///< No locking is performed
    /** Prevent parallel call to the same function. For notifies, it prevent
     * multiple parallel notifies on the same variable
     */
    LOCK_FUNCTION,
    LOCK_INSTANCE,  ///< Prevent parallel call to any function of this object
    LOCK_CLASS,     ///< Prevent parallel call to any function of this class
    LOCK_MODULE     ///< Prevent parallel call to any function of this module
  };

  /** Extended locking model specifications.
   * Permits to set the maximum queue size which is infinite by default.
   */
  class URBI_SDK_API LockSpec
  {
  public:
    LockSpec(LockMode l, unsigned int maxQueueSize = 0)
      : lockMode(l)
      , maxQueueSize(maxQueueSize)
    {}
    LockMode lockMode;
    unsigned int maxQueueSize;
  };

  /** LockSpec that prevents parallel calls to the function, and drops all
   * subsequent calls while one is running.
   */
  static const LockSpec LOCK_FUNCTION_DROP = LockSpec(LOCK_FUNCTION, 1);

   /** LockSpec that prevents parallel calls to the function, and drops all
   * subsequent calls while one is running but one. This mode will ensure
   * maximum CPU usage.
   */
  static const LockSpec LOCK_FUNCTION_KEEP_ONE = LockSpec(LOCK_FUNCTION, 2);

  UObjectHub* getUObjectHub(const std::string& n);
  UObject* getUObject(const std::string& n);
  void uobject_unarmorAndSend(const char* str);
  void send(const char* str);
  void send(const std::string&s);
  void send(const void* buf, size_t size);
  UObjectMode getRunningMode();
  bool isPluginMode();
  bool isRemoteMode();

  /// Set maximum number of threads to use for threaded calls (0=unlimited).
  URBI_SDK_API void setThreadLimit(size_t nThreads);
  /// Return current hostname mangled to fit into an urbiscript variable name.
  URBI_SDK_API std::string getFilteredHostname();

  typedef int UReturn;

  /** Main UObject class definition
      Each UObject instance corresponds to an URBI object.
      It provides mechanisms to bind variables and functions between
      C++ and Urbi.
  */
  class URBI_SDK_API UObject: public UContext
  {
  public:
    /// Call this constructor to create an UObject from C++.
    UObject(impl::UContextImpl* impl = 0);
    /// Must be called by your constructor when called with a string.
    UObject(const std::string&, impl::UContextImpl* impl = 0);
    /// Reserved for internal use
    UObject(int, impl::UContextImpl* impl = 0);
    virtual ~UObject();


    // This call registers both an UObject (say of type
    // UObjectDerived), and a callback working on it (named here
    // fun).

    // These macros provide the following callbacks :
    // Notify
    // Access    | const std::string& | F        | const
    // Change    | urbi::UVar&        | F        | non-const

# ifdef DOXYGEN
    // Doxygen does not handle macros very well so feed it simplified code.

    /** @defgroup notifies Change/Access callback registration.
     *
     * All the registered callback functions can take no argument, a
     * reference to an UVar, or any type convertible from UValue. The last
     * kind will be called with the current value contained in the UVar.
     *  @{
     */
    /*!
    \brief Call a function each time a variable is modified.
    \param v the variable to monitor.
    \param fun the function to call each time the variable \b v is modified.
    The function is called rigth after the variable v is modified.
    */
    void UNotifyChange(UVar& v, void (UObject::*fun)(UVar&));

    /*!
    \brief UnotifyChange() variant that passes the UVar value as argument to
    your callback function.
    */
    template<typename T>
    void UNotifyChange(UVar& v, void (UObject::*fun)(T));

    /*!
    \brief Similar to UNotifyChange(), but run function in a thread.
    \param v the variable to monitor.
    \param fun the function to call.
    \param m the locking mode to use.
    */
    void
    UNotifyThreadedChange(UVar& v, void (UObject::*fun)(UVar&), LockMode m);

    /*!
    \brief Similar to UNotifyChange(), but run function in a thread.
    \param v the variable to monitor.
    \param fun the function to call.
    \param s the locking specifications to use.
    */
    void
    UNotifyThreadedChange(UVar& v, void (UObject::*fun)(UVar&), LockSpec s);

    /*!
    \brief Call a function each time a variable is accessed.
    \param v the variable to monitor.
    \param fun the function to call each time the variable \b v is accessed.
    The function is called right \b before the variable \b v is accessed, giving
    \b fun the opportunity to modify it.
    */
    void
    UNotifyAccess(UVar& v, void (UObject::*fun)(UVar&));
    void
    UNotifyThreadedAccess(UVar& v, void (UObject::*fun)(UVar&), LockMode m);
    void
    UNotifyThreadedAccess(UVar& v, void (UObject::*fun)(UVar&), LockSpec s);
    /** @} */

    /// Call \a fun every \a t milliseconds.
    template <class T>
    TimerHandle
    USetTimer(ufloat t, void (T::*fun)());

# else // !DOXYGEN

    /// \internal
#  define MakeNotify(Type, Notified,			\
                     TypeString, Owned, Name,		\
                     StoreArg)				\
    template <typename F>				\
    void UNotify##Type(Notified, F fun)			\
    {							\
	createUCallback(*this, StoreArg, TypeString,	\
                        this, fun, Name);		\
    }							\
    template <typename F>				\
    void UNotifyThreaded##Type(Notified, F fun,		\
                               LockMode lockMode)	\
    {							\
	createUCallback(*this, StoreArg, TypeString,	\
                        this, fun, Name)		\
          ->setAsync(getTaskLock(lockMode, Name));	\
    }                                                   \
    template <typename F>				\
    void UNotifyThreaded##Type(Notified, F fun,		\
                               LockSpec lockMode)	\
    {							\
	createUCallback(*this, StoreArg, TypeString,	\
                        this, fun, Name)		\
          ->setAsync(getTaskLock(lockMode, Name));	\
    }



    /// \internal Define notify by name or by passing an UVar.
#  define MakeMetaNotify(Type, TypeString)				\
    MakeNotify(Type, UVar& v, TypeString,				\
               v.owned, v.get_name (),                                  \
               v.get_temp()?new UVar(v.get_name(), ctx_):&v);           \
    MakeNotify(Type, InputPort& v, TypeString,				\
               v.owned, v.get_name (),                                  \
               &v);                                                     \
    MakeNotify(Type, const std::string& name, TypeString,		\
               false, name, new UVar(name, ctx_));

    /// \internal
    MakeMetaNotify(Access, "varaccess");

    /// \internal
    MakeMetaNotify(Change, "var");

#  undef MakeNotify
#  undef MakeMEtaNotify

#  ifndef SWIG
    /// \internal
#   define MKUSetTimer(Const, Useless)                                  \
    template <class T, class R>						\
    TimerHandle USetTimer(ufloat t, R (T::*fun) () Const)	        \
    {									\
      return (new UTimerCallbackobj<T>                                  \
              (__name, t,                                               \
               dynamic_cast<T*>(this),                                  \
               boost::bind(fun, dynamic_cast<T*>(this)),                \
               ctx_))                                                   \
        ->handle_get();                                                 \
    }

    MKUSetTimer (/**/, /**/);
    MKUSetTimer (const, /**/);

#   undef MKUSetTimer
#  endif // !SWIG
# endif // DOXYGEN

    /// Remove a timer registered with USetTimer.
    bool removeTimer(TimerHandle h);

    /// Request permanent synchronization for v.
    void USync(UVar &v);

    /// Name of the object as seen in Urbi.
    std::string __name;
    /// Name of the class the object is derived from.
    std::string classname;
    /// True when the object has been newed by an urbi command.
    bool derived;

    UObjectList members;

    /// The hub, if it exists.
    UObjectHub* objecthub;

    /// Set a timer that will call the update function every 'period'
    /// milliseconds.
    void USetUpdate(ufloat period);
    virtual int update();


    /// \name Autogroup.
    /// \{
    /// These functions are obsoleted, they are not supported
    /// in Urbi SDK 2.0.
    /// Set autogrouping facility for each new subclass created.
    URBI_SDK_DEPRECATED
    void UAutoGroup();
    /// Called when a subclass is created if autogroup is true.
    URBI_SDK_DEPRECATED
    virtual void addAutoGroup();

    /// Join the uobject to the 'gpname' group.
    URBI_SDK_DEPRECATED
    virtual void UJoinGroup(const std::string& gpname);

    /// Add a group with a 's' after the base class name.
    URBI_SDK_DEPRECATED
    bool autogroup;
    /// \}

    /// Void function used in USync callbacks.
    int voidfun();

    /// Flag to know whether the UObject is in remote mode or not
    bool remote;

    /// Remove all bindings, this method is called by the destructor.
    void clean();

    /// Find the TaskLock associated with lock mode \b m.
    libport::ThreadPool::rTaskLock getTaskLock(LockMode m,
                                               const std::string& what);
    /// Find the TaskLock associated with lock specs \b s.
    libport::ThreadPool::rTaskLock getTaskLock(LockSpec s,
                                               const std::string& what);
    /// The load attribute is standard and can be used to control the
    /// activity of the object.
    UVar load;

    baseURBIStarter* cloner;

    impl::UObjectImpl* impl_get();

    // Override me to have your own LOCK_CLASS task lock.
    virtual libport::ThreadPool::rTaskLock getClassTaskLock();
  private:
    /// Pointer to a globalData structure specific to the
    /// remote/plugin architectures who defines it.
    UObjectData* objectData;

    impl::UObjectImpl* impl_;
    boost::unordered_map<std::string, libport::ThreadPool::rTaskLock>
    taskLocks_;
    /// Instance task lock.
    libport::ThreadPool::rTaskLock taskLock_;
  };

#ifndef NO_UOBJECT_CASTER
  // Provide cast support to UObject*
  template<> struct uvalue_caster<UObject*>
  {
    UObject* operator()(urbi::UValue& v);
  };
  UValue& operator,(UValue&a, const UObject* b);
#endif
} // end namespace urbi

// This file needs the definition of UObject, so included last.
// To be cleaned later.
# include <urbi/ustarter.hh>

# include <urbi/uobject.hxx>

# include <libport/warning-pop.hh>

#endif // ! URBI_UOBJECT_HH
