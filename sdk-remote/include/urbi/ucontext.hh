#ifndef URBI_UCONTEXT_HH
# define URBI_UCONTEXT_HH

#include <libport/utime.hh>

#include <urbi/uvalue.hh>

namespace urbi
{
/// Possible UObject running modes.
  enum UObjectMode
  {
    MODE_PLUGIN=1,
    MODE_REMOTE
  };

/// UValue with implicit constructors.
  class UAutoValue: public UValue
  {
  public:
    UAutoValue() :UValue() {}
    template<typename T> UAutoValue(T v)
    :UValue(v) {}
  };
  namespace impl {
    class URBI_SDK_API UContextImpl;
  }

  /** Methods available in both UObject and UVar
   *  Wrapper around UContextImpl to enable access through inheritance.
   */
  class URBI_SDK_API UContext
  {
  public:
    UContext(impl::UContextImpl* ctx=0);
     UObjectHub* getUObjectHub(const std::string& n);
     UObject* getUObject(const std::string& n);
    /// Send Urbi code (ghost connection in plugin mode, default
    /// connection in remote mode).
     void uobject_unarmorAndSend(const char* str);

    /// Send the string to the connection hosting the UObject.
     void send(const char* str);
     void send(const std::string&s);
    /// Send buf to the connection hosting the UObject.
     void send(const void* buf, size_t size);
     void call(const std::string& object,
                         const std::string& method,
                         UAutoValue v1 = UAutoValue(),
                         UAutoValue v2 = UAutoValue(),
                         UAutoValue v3 = UAutoValue(),
                         UAutoValue v4 = UAutoValue(),
                         UAutoValue v5 = UAutoValue(),
                         UAutoValue v6 = UAutoValue(),
                         UAutoValue v7 = UAutoValue(),
                         UAutoValue v8 = UAutoValue());

    /// Return the mode in which the code is running.
    UObjectMode getRunningMode();

    /// Return true if the code is running in plugin mode.
    bool isPluginMode();
    /// Return true if the code is running in remote mode.
     bool isRemoteMode();

    /// Yield execution until next cycle. Process pending messages in remote mode.
     void yield();
    /// Yield execution until \b deadline is met (see libport::utime()).
     void yield_until(libport::utime_t deadline);
    /** Yield execution until something else is scheduled, or until a message is
    * received in remote mode.
    */
     void yield_until_things_changed();
    /** If \b s is true, mark the current task as having no side effect.
    * This call has no effect in remote mode.
    */
     void side_effect_free_set(bool s);
    /// Get the current side_effect_free state.
     bool side_effect_free_get();

     impl::UContextImpl* ctx_;
  };

  /// Will be used if no context is explicitly passed.
  URBI_SDK_API impl::UContextImpl* getCurrentContext();
  URBI_SDK_API void setCurrentContext(impl::UContextImpl*);

  URBI_SDK_API void echo(const char* format, ... )
  __attribute__((__format__(printf, 1, 2)));

}
#include "urbi/ucontext.hxx"
#endif
