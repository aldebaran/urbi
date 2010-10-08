/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/utimer-callback.hh

#ifndef URBI_UTIMER_CALLBACK_HH
# define URBI_UTIMER_CALLBACK_HH

# include <string>

# include <boost/function.hpp>

# include <libport/ufloat.h>

# include <urbi/export.hh>
# include <urbi/utable.hh>

namespace urbi
{

  //! Timer mechanism
  /*! This class stores a callback as a class method.
   */

  class URBI_SDK_API UTimerCallback
  {
  public:
    /**
     * \param objname object name.
     * \param period in milliseconds.
     * \param ctx uobject context.
     */
    UTimerCallback(const std::string& objname,
                   ufloat period,
                   impl::UContextImpl* ctx);
    virtual ~UTimerCallback();

    virtual void call() = 0;
    void registerCallback();
    TimerHandle handle_get() { return handle_;}
    ufloat period;
    ufloat lastTimeCalled;
    std::string objname;
  private:
    impl::UContextImpl* ctx_;
    TimerHandle handle_;
  };

  // UTimerCallback subclasses

  template <class T>
  class UTimerCallbackobj : public UTimerCallback
  {
  public:
    UTimerCallbackobj(const std::string& objname,
		      ufloat period, T* obj,
		      boost::function0<void> fun, impl::UContextImpl* ctx)
      : UTimerCallback(objname, period, ctx)
      , obj(obj)
      , fun(fun)
    {
      registerCallback();
    }

    virtual void call()
    {
      fun();
    }
  private:
    T* obj;
    boost::function0<void> fun;
  };


} // end namespace urbi

#endif // ! URBI_UTIMER_CALLBACK_HH
