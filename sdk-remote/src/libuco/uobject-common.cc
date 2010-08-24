/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <cstdarg>
#include <libport/cstdio>
#include <libport/debug.hh>

#include <boost/thread.hpp>

#include <libport/containers.hh>
#include <libport/foreach.hh>
#include <libport/utime.hh>

#include <urbi/uobject.hh>
#include <urbi/ustarter.hh>
#include <urbi/ucontext-factory.hh>

GD_CATEGORY(UObject);

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
  static impl::UContextImpl* default_context = 0;
  impl::UContextImpl* getCurrentContext()
  {
    impl::UContextImpl* res = current_context.get();
    return res ? res : default_context;
  }

  void setCurrentContext(impl::UContextImpl* impl)
  {
    if (!default_context)
      default_context = impl;
    current_context.reset(impl);
  }


  /*-------------------.
  | UGenericCallback.  |
  `-------------------*/

  UGenericCallback::UGenericCallback(UObject& owner,
                                     UVar* target,
				     const std::string& type,
				     const std::string& name,
				     int size,
                                     impl::UContextImpl* ctx)
    : UContext(ctx)
    , nbparam(size)
    , objname(&owner?owner.__name:"dummy")
    , type(type)
    , name(name)
    , target(target)
    , owner(owner)
    , synchronous_(true)
  {
    impl_ = ctx_->getGenericCallbackImpl();
    impl_->initialize(this, target?target->owned:false);
  }

  UGenericCallback::UGenericCallback(UObject& owner,
                                     UVar* target,
				     const std::string& type,
				     const std::string& name,
                                     impl::UContextImpl* ctx)
    : UContext(ctx)
    , objname(&owner?owner.__name:"dummy")
    , type(type)
    , name(name)
    , target(target)
    , owner(owner)
    , synchronous_(true)
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

  bool
  UGenericCallback::isSynchronous() const
  {
    return synchronous_;
  }

  void
  UGenericCallback::syncEval(UList& params, OnDone onDone)
  {
    UValue res;
    try {
      res = __evalcall(params);
      if (onDone)
        onDone(res, 0);
    }
    catch(const std::exception& e)
    {
      if (onDone)
        onDone(res, &e);
    }
    catch(...)
    {
      if (onDone)
      {
        std::runtime_error e("Invalid exception");
        onDone(res, &e);
      }
    }
  }

  void
  UGenericCallback::eval(UList& params, OnDone onDone)
  {
    if (synchronous_)
    {
      syncEval(params, onDone);
    }
    else
    {
      threadPool().queueTask(
                             boost::function0<void>(
        boost::bind(&UGenericCallback::syncEval, this, params, onDone)),
        taskLock);
    }
  }

  /* Note: to implement the LOCK_MODULE mode, we must delegate search of the
   * TaskLock to use to a function within the calling module.
   * That is why we take the TaskLock here and we do not find it ourselve.
   */
  void
  UGenericCallback::setAsync(libport::ThreadPool::rTaskLock l)
  {
    synchronous_ = false;
    taskLock = l;
  }

  libport::ThreadPool&
  UGenericCallback::threadPool()
  {
    static libport::ThreadPool tp;
    return tp;
  }

  void
  setThreadLimit(size_t nThreads)
  {
    UGenericCallback::threadPool().resize(nThreads);
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
    , taskLock_(new libport::ThreadPool::TaskLock)
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
    , taskLock_(new libport::ThreadPool::TaskLock)
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

  libport::ThreadPool::rTaskLock
  UObject::getClassTaskLock()
  {
    throw std::runtime_error("You must redefine getClassTaskLock to use"
                             " asynchronous calls with LOCK_CLASS locking");
  }


  #define GENERIC_TRY(desc, code)                         \
  do {                                                    \
    std::string estr_;                                    \
    try {                                                 \
      code                                                \
    }                                                     \
    catch(const std::exception& e)                        \
    {                                                     \
      estr_ = e.what();                                   \
    }                                                     \
    catch(...)                                            \
    {                                                     \
      estr_ = "unknown exception";                        \
    }                                                     \
    if (!estr_.empty())                                   \
      GD_SERROR("Exception " << desc << " : " << estr_);  \
  } while(0)


  /*---------------.
  | UContextImpl.  |
  `---------------*/
  namespace impl
  {
    void
    UContextImpl::init()
    {
      setCurrentContext(this);
      foreach(baseURBIStarterHub* s, baseURBIStarterHub::list())
      {
        if (!libport::mhas(initialized, s))
        {
          GD_FINFO_TRACE("initializing UObject hub: %s", s->name);
          GENERIC_TRY("Instanciating hub" << s,
                      newUObjectHubClass(s);
                      initialized.insert(s);
                      );
        }
      }
      foreach(baseURBIStarter* s, baseURBIStarter::list())
      {
        if (!libport::mhas(initialized, s))
        {
          GD_FINFO_TRACE("initializing UObject: %s", s->name);
          GENERIC_TRY("Instanciating object" << s,
                      newUObjectClass(s);
                      initialized.insert(s);
                      );
        }
      }
    }
#undef GENERIC_TRY
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
  }

  void
  UTimerCallback::registerCallback()
  {
     handle_ = ctx_->setTimer(this);
  }

  UTimerCallback::~UTimerCallback()
  {
  }

  std::string getFilteredHostname()
  {
    char hn[1024];
    gethostname(hn, 1024);
    std::string res = hn;
    if (!isalpha(res[0]))
      res = "_" + res;
    foreach(char& c, res)
      if (!isalnum(c) && c != '_')
        c = '_';
    return res;
  }
}
