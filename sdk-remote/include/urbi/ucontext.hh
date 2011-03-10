/*
 * Copyright (C) 2009, 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_UCONTEXT_HH
# define URBI_UCONTEXT_HH

# include <libport/fwd.hh>
# include <libport/utime.hh>

# include <urbi/uvalue.hh>

namespace urbi
{
  /// Possible UObject running modes.
  enum UObjectMode
  {
    MODE_PLUGIN = 1,
    MODE_REMOTE
  };

  /// UValue with implicit constructors.
  class UAutoValue: public UValue
  {
  public:
    UAutoValue();
    template<typename T>
    UAutoValue(T v);
    UAutoValue(const UValue& v);
  };

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
              UAutoValue v6 = UAutoValue());

    /// The underlying IO Service, from the UContextImpl.
    boost::asio::io_service& getIoService();

    /// The mode in which the code is running.
    UObjectMode getRunningMode() const;

    /// Whether running in plugin mode.
    bool isPluginMode() const;
    /// Whether running in remote mode.
    bool isRemoteMode() const;

    /// Yield execution until next cycle.
    /// Process pending messages in remote mode.
    void yield() const;

    /// Yield execution until \b deadline is met (see libport::utime()).
    void yield_until(libport::utime_t deadline) const;

    /// Yield execution for \b delay.
    void yield_for(libport::utime_t delay) const;

    /** Yield execution until something else is scheduled, or until a message is
     * received in remote mode.
     */
    void yield_until_things_changed() const;

    /** If \b s is true, mark the current task as having no side effect.
     * This call has no effect in remote mode.
     */
    void side_effect_free_set(bool s);

    /// Get the current side_effect_free state.
    bool side_effect_free_get() const;

    /// Get the version of the kernel that will receive send() messages.
    std::pair<int, int> kernelVersion();
    impl::UContextImpl* ctx_;
  };

  /// Will be used if no context is explicitly passed.
  URBI_SDK_API impl::UContextImpl* getCurrentContext();
  URBI_SDK_API void setCurrentContext(impl::UContextImpl*);

  ATTRIBUTE_DEPRECATED
  ATTRIBUTE_PRINTF(1, 2)
  URBI_SDK_API void echo(const char* format, ...);
}

# include "urbi/ucontext.hxx"
#endif
