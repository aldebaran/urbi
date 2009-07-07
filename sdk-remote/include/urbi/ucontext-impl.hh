#ifndef UCONTEXT_IMPL_HH
# define UCONTEXT_IMPL_HH

#include <urbi/fwd.hh>
#include <urbi/uprop.hh>

namespace urbi {
  /** The interfaces UContextImpl, UVarImpl, UObjectImpl and
  * UGenericCallbackImpl are what must be implemented to make an UObject
  * backend.
  */
  namespace impl {
  class URBI_SDK_API UContextImpl
  {
  public:
    /// Inject all loaded uobjects in the system by calling newUObjectClass.
    void init();
    /// Inject a specific uobject
    bool bind(const std::string& name);
    /// Called to instanciate the first Object of class.
    virtual void newUObjectClass(baseURBIStarter* s) = 0;
    virtual void newUObjectHubClass(baseURBIStarterHub* s) = 0;
    /// Retrieve a UObjectHub based on its name or return 0 if not found.
    virtual UObjectHub* getUObjectHub(const std::string& n);
    /// Retrieve a UObject based on its name or return 0 if not found.
    virtual UObject* getUObject(const std::string& n);
    /// Send Urbi code (ghost connection in plugin mode, default
    /// connection in remote mode).
    virtual void uobject_unarmorAndSend(const char* str) = 0;

    /// Send the string to the connection hosting the UObject.
    virtual void send(const char* str) = 0;

    /// Send buf to the connection hosting the UObject.
    virtual void send(const void* buf, size_t size) = 0;

    virtual void call(const std::string& object,
                         const std::string& method,
                         UAutoValue v1 = UAutoValue(),
                         UAutoValue v2 = UAutoValue(),
                         UAutoValue v3 = UAutoValue(),
                         UAutoValue v4 = UAutoValue(),
                         UAutoValue v5 = UAutoValue(),
                         UAutoValue v6 = UAutoValue(),
                         UAutoValue v7 = UAutoValue(),
                         UAutoValue v8 = UAutoValue()) = 0;

    /// Return the mode in which the code is running.
    virtual UObjectMode getRunningMode() = 0;
    /// Get the kernel major,minor version
    virtual std::pair<int, int> kernelVersion() = 0;
    /// Yield execution until next cycle. Process pending messages in remote mode.
    virtual void yield() = 0;
    /// Yield execution until \b deadline is met (see libport::utime()).
    virtual void yield_until(libport::utime_t deadline) = 0;
    /** Yield execution until something else is scheduled, or until a message is
    * received in remote mode.
    */
    virtual void yield_until_things_changed() = 0;
    /** If \b s is true, mark the current task as having no side effect.
    * This call has no effect in remote mode.
    */
    virtual void side_effect_free_set(bool s) = 0;
    /// Get the current side_effect_free state.
    virtual bool side_effect_free_get() = 0;
    virtual UVarImpl* getVarImpl() = 0;
    virtual UObjectImpl* getObjectImpl() = 0;
    virtual UGenericCallbackImpl* getGenericCallbackImpl() = 0;
    /// Default implementations appends to 'objects'.
    virtual void registerObject(UObject*o);
    virtual void setTimer(UTimerCallback* cb) = 0;
    virtual void registerHub(UObjectHub*);
    virtual void removeHub(UObjectHub*) = 0;
    virtual void setHubUpdate(UObjectHub*, ufloat) = 0;
    typedef libport::hash_map<std::string, UObject*>  objects_type;
    objects_type objects;
    typedef libport::hash_map<std::string, UObjectHub*> hubs_type;
    hubs_type hubs;
    std::set<void*> initialized;
  };
  class URBI_SDK_API UObjectImpl
  {
  public:
    // Called by uobject constructor
    virtual void initialize(UObject* owner) = 0;
    virtual void clean() = 0;
    virtual void setUpdate(ufloat period) = 0;
  };
  class URBI_SDK_API UVarImpl
  {
  public:
    virtual void initialize(UVar* owner) = 0;
    virtual void clean() = 0;
    virtual void setOwned() = 0;
    virtual void sync() = 0;
    virtual void request() = 0;
    virtual void keepSynchronized() = 0;
    virtual void set(const UValue& v) = 0;
    virtual const UValue& get() = 0;
    virtual UDataType type() = 0;
    virtual UValue getProp(UProperty prop) = 0;
    virtual void setProp(UProperty prop, const UValue& v) = 0;
    virtual bool setBypass(bool enable) = 0;
  };
  }
}

#endif
