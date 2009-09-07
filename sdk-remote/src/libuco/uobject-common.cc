/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
#include <cstdarg>
#include <cstdio>

#include <boost/thread.hpp>

#include <libport/containers.hh>
#include <libport/foreach.hh>
#include <libport/utime.hh>

#include <urbi/uobject.hh>
#include <urbi/ustarter.hh>
#include <urbi/ucontext-factory.hh>

namespace urbi
{
  namespace impl
  {
    std::vector<std::string> listModules()
    {
      std::vector<std::string> res;
      foreach(baseURBIStarter* s, baseURBIStarter::list())
      {
        res.push_back(s->name);
      }
      return res;
    }
  }

  static void noop(impl::UContextImpl*)
  {
  }

  static boost::thread_specific_ptr<impl::UContextImpl> current_context(&noop);

  impl::UContextImpl* getCurrentContext()
  {
    return current_context.get();
  }

  void setCurrentContext(impl::UContextImpl* impl)
  {
    current_context.reset(impl);
  }


  /*-----------------------.
  | UGenericCallbackImpl.  |
  `-----------------------*/

  namespace impl
  {
    UGenericCallbackImpl::~UGenericCallbackImpl()
    {}
  }

  /*-------------------.
  | UGenericCallback.  |
  `-------------------*/

  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string& type,
				     const std::string& name,
				     int size, bool owned,
                                     impl::UContextImpl* ctx)
    : UContext(ctx)
    , nbparam(size)
    , objname(objname)
    , type(type)
    , name(name)
  {
    impl_ = ctx_->getGenericCallbackImpl();
    impl_->initialize(this, owned);
  }

  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string& type,
				     const std::string& name,
                                     impl::UContextImpl* ctx)
    : UContext(ctx)
    , objname(objname)
    , type(type)
    , name(name)
  {
    impl_ = ctx_->getGenericCallbackImpl();
    impl_->initialize(this);
  }

  UGenericCallback::~UGenericCallback()
  {
    impl_->clear();
  }

  void
  UGenericCallback::registerCallback()
  {
    impl_->registerCallback();
  }


  /*--------------.
  | UObjectImpl.  |
  `--------------*/

  namespace impl
  {
    UObjectImpl::~UObjectImpl()
    {
    }
  }

  /*----------.
  | UObject.  |
  `----------*/

  UObject::UObject(int, impl::UContextImpl* impl)
    : UContext(impl)
    , __name("_dummy")
    , classname("_dummy")
    , derived(false)
    , autogroup(false)
    , remote(true)
    , impl_(ctx_->getObjectImpl())
  {
    impl_->initialize(this);
    objecthub = 0;
  }

  //! UObject constructor.
  UObject::UObject(const std::string& s, impl::UContextImpl* impl)
    : UContext(impl)
    , __name(s)
    , classname(s)
    , derived(false)
    , autogroup(false)
    , remote(true)
    , impl_(ctx_->getObjectImpl())
  {
    impl_->initialize(this);
    objecthub = 0;
    autogroup = false;
    // Do not replace this call to init by a `, load(s, "load")' as
    // both result in "var <__name>.load = 1", which is not valid
    // until the two above lines actually create <__name>.
    load.init(__name, "load");
    // default
    load = 1;
  }

  void
  UObject::addAutoGroup()
  {
    UJoinGroup(classname + "s");
  }

  void
  UObject::UAutoGroup()
  {
    autogroup = true;
  }

  //! UObject destructor.
  UObject::~UObject()
  {
    clean();
    delete impl_;
  }

  void
  UObject::UJoinGroup(const std::string& gpname)
  {
    std::string groupregister = "addgroup " + gpname +" { "+__name+"};";
    send(groupregister);
  }


  /*---------------.
  | UContextImpl.  |
  `---------------*/
  namespace impl
  {
    // Declared pure virtual, but needs an implementation.  Known
    // idiom.
    UContextImpl::~UContextImpl()
    {}

    void
    UContextImpl::init()
    {
      setCurrentContext(this);
      foreach(baseURBIStarterHub* s, baseURBIStarterHub::list())
      {
        if (!libport::mhas(initialized, s))
        {
          newUObjectHubClass(s);
          initialized.insert(s);
        }
      }
      foreach(baseURBIStarter* s, baseURBIStarter::list())
      {
        if (!libport::mhas(initialized, s))
        {
          newUObjectClass(s);
          initialized.insert(s);
        }
      }
    }

    bool
    UContextImpl::bind(const std::string& n, std::string rename)
    {
      foreach(baseURBIStarter* s, baseURBIStarter::list())
      {
        if (s->name == n)
        {
          s->instanciate(this, rename);
          return true;
        }
      }
      return false;
    }

    void
    UContextImpl::registerObject(UObject*o)
    {
      objects[o->__name] = o;
    }

    void
    UContextImpl::registerHub(UObjectHub*u)
    {
      hubs[u->get_name()] = u;
    }

    UObjectHub*
    UContextImpl::getUObjectHub(const std::string& n)
    {
      return libport::find0(hubs, n);
    }

    UObject*
    UContextImpl::getUObject(const std::string& n)
    {
      return libport::find0(objects, n);
    }
  }

  /*--------------------------.
  | Free standing functions.  |
  `--------------------------*/

  //! echo method
  void
  echo(const char* format, ...)
  {
    if (format)
    {
      va_list arg;
      va_start(arg, format);
      vfprintf(stderr, format, arg);
      va_end(arg);
    }
  }

  UTimerCallback::UTimerCallback(const std::string& objname,
				 ufloat period,
                                 impl::UContextImpl* ctx)
    : period(period)
    , objname(objname)
    , ctx_(ctx)
  {
    lastTimeCalled = -9999999;

    ctx_->setTimer(this);
  }

  UTimerCallback::~UTimerCallback()
  {
  }
}
