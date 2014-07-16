/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <cstdarg>
#include <typeinfo>

#include <libport/cstdio>
#include <libport/debug.hh>

#include <boost/thread.hpp>

#include <libport/containers.hh>
#include <libport/foreach.hh>
#include <libport/utime.hh>

#include <urbi/uobject.hh>
#include <urbi/ustarter.hh>
#include <urbi/ucontext-factory.hh>

GD_CATEGORY(Urbi.UObject);

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
    if (target)
      target->check();
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
    if (target)
      target->check();
    impl_ = ctx_->getGenericCallbackImpl();
    impl_->initialize(this);
  }

  UGenericCallback::~UGenericCallback()
  {
    impl_->clear();
    delete impl_;
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
        std::runtime_error e("invalid exception");
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
      libport::ThreadPool::rTaskHandle h
       = threadPool().queueTask(
                             boost::function0<void>(
        boost::bind(&UGenericCallback::syncEval, this, params, onDone)),
        taskLock);
       GD_FINFO_TRACE("Queued async op: with lock %s: %s", taskLock.get(),
                      h->getState());
      if (h->getState() == libport::ThreadPool::TaskHandle::DROPPED && onDone)
      {
        UValue res;
        onDone(res, 0);
      }
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

  UObject::UObject(impl::UContextImpl* impl)
   : UContext(impl)
   , derived(false)
   , autogroup(false)
   , remote(true)
   , cloner(0)
   , impl_(ctx_->getObjectImpl())
   , taskLock_(new libport::ThreadPool::TaskLock)
 {
   impl_->initialize(this);
    objecthub = 0;
  }

  UObject::UObject(int, impl::UContextImpl* impl)
    : UContext(impl)
    , __name("_dummy")
    , classname("_dummy")
    , derived(false)
    , autogroup(false)
    , remote(true)
    , cloner(0)
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
    , cloner(0)
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


  /*---------------.
  | UContextImpl.  |
  `---------------*/
  namespace impl
  {
# define GENERIC_TRY(Desc, Code)                                \
    do {                                                        \
      std::string estr_;                                        \
      try                                                       \
      {                                                         \
        Code;                                                   \
      }                                                         \
      catch(const std::exception& e)                            \
      {                                                         \
        estr_ = e.what();                                       \
      }                                                         \
      catch(...)                                                \
      {                                                         \
        estr_ = "unknown exception";                            \
      }                                                         \
      if (!estr_.empty())                                       \
        GD_SERROR("Exception " << Desc << ": " << estr_);       \
    } while(0)


    void
    UContextImpl::init()
    {
      setCurrentContext(this);
      foreach (baseURBIStarterHub* s, baseURBIStarterHub::list())
      {
        if (!libport::has(initialized, s))
        {
          GD_FINFO_TRACE("initializing UObject hub: %s", s->name);
          GENERIC_TRY("Instanciating hub" << s,
                      newUObjectHubClass(s);
                      initialized.insert(s);
                      );
        }
      }
      foreach (baseURBIStarter* s, baseURBIStarter::list())
      {
        if (!libport::has(initialized, s))
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
      const std::type_info& ti = typeid(*o);
      if (!o->cloner)
      {
        // Object was instanciated from C++ and has no cloner, try to find one.
        foreach(objects_type::value_type& v, objects)
        {
          GD_FINFO_TRACE("    Scanning %s (%s)",  v.second->__name,
                         typeid(*v.second).name());
          if (typeid(*v.second) == ti && o != v.second)
          {
            GD_FINFO_TRACE("Found parent of %s: %s", o->__name,
                           v.second->__name);
            o->cloner = v.second->cloner;
            return;
          }
        }
        GD_FINFO_TRACE("No parent fonud for %s", o->__name);
      }
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



    void
    UContextImpl::addCleanup(boost::function0<void> op)
    {
      aver(cleanup_list_.get());
      aver(!cleanup_list_->empty());
      cleanup_list_->back().push_back(op);
    }

    void
    UContextImpl::pushCleanupStack()
    {
      CleanupList* cl = cleanup_list_.get();
      if (!cl)
      {
        cl = new CleanupList;
        cleanup_list_.reset(cl);
      }
      cl->resize(cl->size()+1);
    }

    void
    UContextImpl::popCleanupStack()
    {
      foreach (boost::function0<void>& f, cleanup_list_->back())
        f();
      cleanup_list_->pop_back();
    }

    UContextImpl::CleanupStack::CleanupStack(UContextImpl& owner)
      : owner_(owner)
    {
      owner_.pushCleanupStack();
    }

    UContextImpl::CleanupStack::~CleanupStack()
    {
      owner_.popCleanupStack();
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
      // This method is deprecated, there's no good reason to
      // make it perfect.
      char buf[BUFSIZ];
      va_list arg;
      va_start(arg, format);
      // Don't print if we overflow the buffer.  It would be nice to
      // rely on the behavior of the GNU LibC which accepts 0 as
      // destination buffer to query the space needed.  But it is not
      // portable (e.g., segv on OS X).  So rather, try to vsnprintf,
      // and upon failure, revert the buffer in its previous state.
      int r = vsnprintf(buf, sizeof buf, format, arg);
      va_end(arg);
      // vsnprintf returns the number of characters to write.  Check
      // that it fits.  Don't forget the ending '\0' that it does not
      // count, but wants to add.
      if (r < 0 || static_cast<int>(sizeof buf) <= r)
        // Don't produce partial input.
        buf[sizeof buf - 1] = 0;
      GD_INFO_TRACE(buf);
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
    std::string res = libport::gethostname();
    if (!isalpha(res[0]))
      res = "_" + res;
    foreach (char& c, res)
      if (!isalnum(c) && c != '_')
        c = '_';
    return res;
  }
}
