/*
 * Copyright (C) 2009, 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/hash.hh>
#include <urbi/fwd.hh>
#include <urbi/ucontext-impl.hh>

namespace urbi
{

  /*-------------.
  | UAutoValue.  |
  `-------------*/

  inline
  UAutoValue::UAutoValue()
    : UValue()
  {}

  template<typename T>
  inline
  UAutoValue::UAutoValue(T v)
  {
    *this, v;
  }

  inline
  UAutoValue::UAutoValue(const UValue& v)
  {
    set(v, false);
  }

  /*-----------.
  | UContext.  |
  `-----------*/

  inline
  UContext::UContext(impl::UContextImpl* impl)
    : ctx_(impl ? impl : getCurrentContext())
  {
  }

  inline
  boost::asio::io_service&
  UContext::getIoService()
  {
    return ctx_->getIoService();
  }

  inline UObjectHub*
  getUObjectHub(const std::string& n)
  {
    return getCurrentContext()->getUObjectHub(n);
  }

  inline UObjectHub*
  UContext::getUObjectHub(const std::string& n)
  {
    return ctx_->getUObjectHub(n);
  }

  inline UObject* getUObject(const std::string& n)
  {
    return getCurrentContext()->getUObject(n);
  }

  inline UObject* UContext::
  getUObject(const std::string& n)
  {
    return ctx_->getUObject(n);
  }

  /// Send Urbi code (ghost connection in plugin mode, default
  /// connection in remote mode).
  inline void
  uobject_unarmorAndSend(const char* str)
  {
    return getCurrentContext()->uobject_unarmorAndSend(str);
  }

  inline void
  UContext::uobject_unarmorAndSend(const char* str)
  {
    return ctx_->uobject_unarmorAndSend(str);
  }

  inline void
  send(const char* str)
  {
    getCurrentContext()->send(str);
  }

  /// Send the string to the connection hosting the UObject.
  inline void
  UContext::send(const char* str)
  {
    ctx_->send(str);
  }

  inline void
  send(const std::string& str)
  {
    getCurrentContext()->send(str.c_str(), str.length());
  }

  inline void
  UContext::send(const std::string& s)
  {
    ctx_->send(s.c_str(), s.length());
  }

  /// Send buf to the connection hosting the UObject.
  inline void
  send(const void* buf, size_t size)
  {
    getCurrentContext()->send(buf, size);
  }

  inline void
  UContext::send(const void* buf, size_t size)
  {
    ctx_->send(buf, size);
  }

  inline void
  UContext::call(const std::string& object,
                 const std::string& method,
                 UAutoValue v1,
                 UAutoValue v2,
                 UAutoValue v3,
                 UAutoValue v4,
                 UAutoValue v5,
                 UAutoValue v6)
  {
    ctx_->call(object, method, v1, v2, v3, v4, v5, v6);
  }

  URBI_SDK_API UObjectMode running_mode();
  inline UObjectMode
  getRunningMode()
  {
    if (impl::UContextImpl* ctx = getCurrentContext())
      return ctx->getRunningMode();
    else
      // Hack to get a valid mode if one of the uobjects is loaded
      return running_mode();
  }

  /// Return the mode in which the code is running.
  inline UObjectMode
  UContext::getRunningMode() const
  {
    return ctx_->getRunningMode();
  }

  inline bool
  isPluginMode()
  {
    return getRunningMode() == MODE_PLUGIN;
  }

  /// Return true if the code is running in plugin mode.
  inline bool
  UContext::isPluginMode() const
  {
    return getRunningMode() == MODE_PLUGIN;
  }

  inline bool
  isRemoteMode()
  {
    return getRunningMode() == MODE_REMOTE;
  }
  /// Return true if the code is running in remote mode.
  inline bool
  UContext::isRemoteMode() const
  {
    return getRunningMode() == MODE_REMOTE;
  }

  /// Yield execution until next cycle. Process pending messages in remote mode.
  inline void
  UContext::yield() const
  {
    return ctx_->yield();
  }

  /// Yield execution until \b deadline is met (see libport::utime()).
  inline void
  UContext::yield_until(libport::utime_t deadline) const
  {
    return ctx_->yield_until(deadline);
  }

  inline void
  UContext::yield_for(libport::utime_t delay) const
  {
    return ctx_->yield_for(delay);
  }

  /** Yield execution until something else is scheduled, or until a message is
  * received in remote mode.
  */
  inline void
  UContext::yield_until_things_changed() const
  {
    return ctx_->yield_until_things_changed();
  }

  /** If \b s is true, mark the current task as having no side effect.
  * This call has no effect in remote mode.
  */
  inline void
  UContext::side_effect_free_set(bool s)
  {
    return ctx_->side_effect_free_set(s);
  }

  /// Get the current side_effect_free state.
  inline bool
  UContext::side_effect_free_get() const
  {
    return ctx_->side_effect_free_get();
  }

  inline std::pair<int, int>
  UContext::kernelVersion()
  {
    return ctx_->kernelVersion();
  }
}
