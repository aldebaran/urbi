/*
 * Copyright (C) 2005-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuobject/uobject.cc

#include <iostream>
#include <sstream>
#include <list>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <libport/containers.hh>
#include <libport/debug.hh>
#include <libport/escape.hh>
#include <libport/foreach.hh>
#include <libport/lexical-cast.hh>
#include <libport/unistd.h>

#include <libport/debug.hh>
#include <libport/format.hh>

#include <liburbi/compatibility.hh>

#include <urbi/uexternal.hh>
#include <urbi/umessage.hh>
#include <urbi/uobject.hh>
#include <urbi/ustarter.hh>
#include <urbi/usyncclient.hh>
#include <urbi/uvar.hh>
#include <urbi/ucontext-factory.hh>
#include <libuobject/remote-ucontext-impl.hh>

GD_CATEGORY(LibUObject);

#define REQUIRE(Cond, ...)                      \
  do {                                          \
    if (!(Cond))                                \
    {                                           \
      msg.client.printf(__VA_ARGS__);           \
      return URBI_CONTINUE;                     \
    }                                           \
  } while (false)

namespace urbi
{
  UObjectMode running_mode()
  {
    return MODE_REMOTE;
  }

  namespace impl
  {
    namespace
    {
      /// Look for the function args[1] in \a t, and make a call to the
      /// associated callback with arguments (args[2], args[3], etc.).
      static
      void
      eval_call(UTable& t, UList& args)
      {
        if (UTable::callbacks_type* cs = t.find0(args[1]))
        {
          args.setOffset(2);
          foreach (UGenericCallback* c, *cs)
            c->eval(args);
          args.setOffset(0);
        }
      }
    }

    typedef boost::unordered_map<std::string, impl::UContextImpl*>
            contexts_type;
    static contexts_type contexts;

    impl::UContextImpl*
    makeRemoteContext(const std::string& host, const std::string& port)
    {
      impl::UContextImpl* c = new impl::RemoteUContextImpl(
        new USyncClient(host, lexical_cast<unsigned>(port)));
      return c;
    }

    impl::UContextImpl*
    getRemoteContext(const std::string& host, const std::string& port)
    {
      std::string key = host + ':' + port;
      contexts_type::iterator i = contexts.find(key);
      if (i != contexts.end())
        return i->second;
      impl::UContextImpl* c = new impl::RemoteUContextImpl(
        new USyncClient(host, lexical_cast<unsigned>(port)));
      contexts[key] = c;
      return c;
    }

    /*---------------------.
    | RemoteUContextImpl.  |
    `---------------------*/

    RemoteUContextImpl::RemoteUContextImpl(USyncClient* client)
      : client_(client)
      , closed_(false)
      , dummyUObject(0)
      , enableRTP(true)
      , dispatchDepth(0)
      , dataSent(false)
    {
      client_->setCallback(callback(*this, &RemoteUContextImpl::dispatcher),
                           externalModuleTag.c_str());
      client_->setClientErrorCallback(callback(*this,
                 &RemoteUContextImpl::clientError));

      typedef long long int us_t;
#define GET(Type, Part)                                                 \
      Type Part =                                                       \
        Type(client_->syncGet("System.timeReference." #Part)->value->val)
      GET(int, year);
      GET(int, month);
      GET(int, day);
      GET(us_t, us);
#undef GET

      libport::utime_reference_set
        (boost::posix_time::ptime(boost::gregorian::date(year, month, day),
                                  boost::posix_time::microseconds(us)));
      GD_FINFO_DEBUG("Remote kernel reference timestamp: %s.",
                     to_simple_string(libport::utime_reference()));
    }

    RemoteUContextImpl::~RemoteUContextImpl()
    {}

    USyncClient*
    RemoteUContextImpl::getClient()
    {
      return client_;
    }

    UObject*
    RemoteUContextImpl::getDummyUObject()
    {
      if (!dummyUObject)
        dummyUObject = new UObject(0, this);
      return dummyUObject;
    }

    void RemoteUContextImpl::uobject_unarmorAndSend(const char* a)
    {
      unarmorAndSend(a, client_);
    }

    void RemoteUContextImpl::send(const char* a)
    {
      *client_ << a;
    }

    void RemoteUContextImpl::send(const void* buf, size_t size)
    {
      client_->rdbuf()->sputn(static_cast<const char*> (buf), size);
    }

    UObjectMode RemoteUContextImpl::getRunningMode() const
    {
      return MODE_REMOTE;
    }

    UTable&
    RemoteUContextImpl::tableByName(const std::string& n)
    {
#define CHECK(v) if (n == #v) return v##map##_
      CHECK(access);
      CHECK(event);
      CHECK(eventend);
      CHECK(function);
      CHECK(monitor);
#undef CHECK
      if (n == "var" || n =="var_onrequest")
        return monitormap_;
      if (n == "varaccess")
        return accessmap_;
      throw std::runtime_error("Unexpected table name. " + n);
    }

    std::pair<int, int>
    RemoteUContextImpl::kernelVersion() const
    {
      client_->waitForKernelVersion(true);
      return std::make_pair(client_->kernelMajor(),
                            client_->kernelMinor());
    }

    void
    RemoteUContextImpl::instanciated(UObject*)
    {
      // Protect our initialization code against rescoping by ','.
      send(";");
    }


    void
    RemoteUContextImpl::lock()
    {
    }

    void
    RemoteUContextImpl::unlock()
    {
    }

    boost::asio::io_service&
    RemoteUContextImpl::getIoService()
    {
      return client_->get_io_service();
    }

    /*--------------------.
    | RemoteUObjectImpl.  |
    `--------------------*/

    RemoteUObjectImpl::~RemoteUObjectImpl()
    {}

    //! UObject constructor.
    void RemoteUObjectImpl::initialize(UObject* owner)
    {
      //We were called by UObject base constructor.
      period = -1;
      this->owner_ = owner;
      if (owner->__name == "_dummy")
        return;
      owner_->ctx_->registerObject(owner);
      UClient* client =
        dynamic_cast<RemoteUContextImpl*>(owner_->ctx_)->client_;
      URBI_SEND_PIPED_COMMAND_C(*client,
                                "class " << owner_->__name << "{}");
      URBI_SEND_PIPED_COMMAND_C(*client,
                                "external object " << owner_->__name);
      // Bind update, we need it since we use a dummy message locally generated
      // to trigger the periodic call.
      createUCallback(*owner, 0, "function", owner, &UObject::update,
                      owner->__name + ".update");

      // At this point the child class constructor is called, and will
      // also send piped commands.
      // Then the starter will call instanciated() which will send a semicolon.
    }

    //! UObject cleaner
    void
    RemoteUObjectImpl::clean()
    {
      RemoteUContextImpl& ctx = dynamic_cast<RemoteUContextImpl&>
        (*(owner_->ctx_));
      ctx.monitormap().clean(owner_->__name);
      ctx.accessmap().clean(owner_->__name);
      ctx.functionmap().clean(owner_->__name);
      ctx.eventmap().clean(owner_->__name);
      ctx.eventendmap().clean(owner_->__name);

      if (owner_->objecthub)
        owner_->objecthub->members.remove(owner_);
    }

    void
    RemoteUObjectImpl::setUpdate(ufloat t)
    {
      if (updateHandler)
      {
        updateHandler->cancel();
        updateHandler.reset();
      }
      period = t;
      onUpdate();
    }

    void
    RemoteUObjectImpl::onUpdate()
    {
      if (0 < period)
      {
        RemoteUContextImpl& ctx =
          dynamic_cast<RemoteUContextImpl&>(*(owner_->ctx_));
        UMessage m(*ctx.client_);
        m.type = MESSAGE_DATA;
        m.tag = externalModuleTag;
        m.value = new UValue(UList());
        m.value->list->push_back(UEM_EVALFUNCTION);
        m.value->list->push_back(owner_->__name + ".update__0");
        m.value->list->push_back("");
        // This is potentialy not the worker thread, cannot call dispatcher()
        // synchronously.
        ctx.getClient()->notifyCallbacks(m);
        updateHandler =
          libport::asyncCall(boost::bind(&RemoteUObjectImpl::onUpdate, this),
                             useconds_t(period * 1000));
      }
    }

    void RemoteUContextImpl::yield() const
    {
      yield_until(libport::utime());
    }

    void RemoteUContextImpl::yield_until(libport::utime_t deadline) const
    {
      // Ensure processEvents is called at least once.
      while (true)
      {
        bool processed =
          dynamic_cast<USyncClient*>(client_)->processEvents(0);
        if (deadline < libport::utime())
          return;
        if (!processed)
          usleep(0);
      }
    }

    void RemoteUContextImpl::yield_until_things_changed() const
    {
      while (true)
      {
        if (dynamic_cast<USyncClient*>(client_)->processEvents(0))
          return;
        usleep(0);
      }
    }

    void RemoteUContextImpl::side_effect_free_set(bool)
    {}

    bool RemoteUContextImpl::side_effect_free_get() const
    {
      return false;
    }

    static void
    call_result(UAbstractClient* client, std::string var,
                const UValue& retval, const std::exception* e)
    {
      // var is empty for internally generated messages (such as update())
      if (var.empty())
        return;
      // This method can be called by a thread from the Thread Pool because
      // it is used as a callback function.  Thus we have to declare the
      // category for the debugger used by the current thread.
      GD_FINFO_DUMP("... dispatch %s done", var);
      if (e)
      {
         URBI_SEND_COMMA_COMMAND_C
           (*client,
            "Global.UObject.funCall(\"" << var << "\", "
            << "Exception.new(\""
            << "Exception while calling remote bound method: "
            << libport::escape(e->what())
            << "\"))");
      }
      else
        switch (retval.type)
        {
        case DATA_BINARY:
          // Send it
          // URBI_SEND_COMMAND does not now how to send binary since it
          // depends on the kernel version.
          client->startPack();
          *client << "Global.UObject.funCall(\"" << var << "\", ";
          client->send(retval);
          *client << "),";
          client->endPack();
          break;

        case DATA_VOID:
          URBI_SEND_COMMAND_C
            (*client,
             "Global.UObject.funCall(\"" << var << "\")");
          break;

        default:
          URBI_SEND_COMMA_COMMAND_C
            (*client,
             "Global.UObject.funCall(\"" << var << "\", " << retval << ")");
          break;
        }
    }

    UCallbackAction
    RemoteUContextImpl::dispatcher(const UMessage& msg)
    {
      if (closed_)
        return URBI_CONTINUE;

      //check message type
      REQUIRE(msg.type == MESSAGE_DATA,
              "Component Error: unknown message content, type %d\n",
              msg.type);
      REQUIRE(msg.value->type == DATA_LIST,
              "Component Error: unknown message content, value type %d\n",
              msg.value->type);

      UList& array = *msg.value->list;

      REQUIRE(array[0].type == DATA_DOUBLE,
              "Component Error: invalid server message type %d\n",
              array[0].type);

      FINALLY(((unsigned int& , dispatchDepth)), dispatchDepth--);
      dispatchDepth++;
      setCurrentContext(this);
      switch ((USystemExternalMessage)(int)array[0])
      {
      case UEM_ASSIGNVALUE:
        REQUIRE(array.size() == 4,
                "Component Error: Invalid number "
                "of arguments in the server message: %lu (expected 4)\n",
                static_cast<unsigned long>(array.size()));
        assignMessage(array[1], array[2], array[3]);
        break;

      case UEM_EVALFUNCTION:
        REQUIRE(3 <= array.size(),
                "Component Error: Invalid number "
                "of arguments in the server message: %lu\n",
                static_cast<unsigned long>(array.size()));
        evalFunctionMessage(array[1], array[2], array);
        break;

      case UEM_EMITEVENT:
        eval_call(eventmap(), array);
        break;

      case UEM_ENDEVENT:
        eval_call(eventendmap(), array);
        break;

      case UEM_NEW:
      {
        impl::UContextImpl::CleanupStack s_(*this);
        objects_type::iterator i = objects.find(std::string(array[2]));
        REQUIRE(i != objects.end(),
                "No such objects %s\n", std::string(array[2]).c_str());
        baseURBIStarter* bsa = i->second->cloner;
        GD_FINFO_DEBUG("instantiating from %s, name: %s", bsa, array[1]);
        bsa->instanciate(this, (std::string) array[1]);
      }
      break;

      case UEM_DELETE:
      {
        impl::UContextImpl::CleanupStack s_(*this);
        objects_type::iterator i = objects.find(std::string(array[1]));
        if (i == objects.end())
          break;
        if (objects.size() == 1)
          exit(0);
        else
          delete i->second;
        objects.erase(i);
      }
      break;
      case UEM_INIT:
        init();
        break;
      case UEM_TIMER:
        {
        std::string cbname = array[1];
        TimerMap::iterator i = timerMap.find(cbname);
        if (i != timerMap.end())
          i->second.second->call();
        }
        break;
      case UEM_NORTP:
        GD_WARN("Disabling RTP as requested by engine");
        enableRTP = false;
        break;
      default:
        REQUIRE(false,
                "Component Error: unknown server message type number %d\n",
                (int)array[0]);
      }

      // Send a terminating ';' since code send by the UObject API uses '|'.
      // But only in outermost dispatch call
      if (dispatchDepth == 1 && dataSent)
      {
        URBI_SEND_COMMAND_C(*client_, "");
        dataSent = false;
      }
      return URBI_CONTINUE;
    }

    void
    RemoteUContextImpl::assignMessage(const std::string& name,
                                      const UValue& v, time_t ts)
    {
      int nv = 0, nc = 0;
      if (std::list<UVar*> *us = varmap().find0(name))
      {
        foreach (UVar* u, *us)
        {
          nv++;
          if (RemoteUVarImpl* impl = dynamic_cast<RemoteUVarImpl*>(u->impl_))
            impl->update(v, ts);
          else
          {
            GD_FERROR("Unable to cast %x to a RemoteUVarImpl.", u->impl_);
            std::abort();
          }
        }
      }
      if (UTable::callbacks_type* cs = monitormap().find0(name))
      {
        foreach (UGenericCallback *c, *cs)
        {
          nc++;
          // test of return value here
          UList u;
          u.array.push_back(new UValue());
          u[0].storage = c->target;
          c->eval(u);
        }
      }
    }

    void
    RemoteUContextImpl::evalFunctionMessage(const std::string& name,
                                            const std::string& var,
                                            UList& args)
    {
      GD_FINFO_DUMP("dispatch call %s = %s...", var, name);
      UTable::callbacks_type funs = functionmap()[name];
      UTable::callbacks_type::iterator i = funs.begin();
      if (i == funs.end())
        throw std::runtime_error("no callback found");
      args.setOffset(3);
      (*i)->eval(args,
                 boost::bind(&call_result, client_, var, _1, _2));
    }

    void
    RemoteUContextImpl::newUObjectClass(baseURBIStarter* s)
    {
      s->instanciate(this);
    }
    void
    RemoteUContextImpl::newUObjectHubClass(baseURBIStarterHub* s)
    {
      s->instanciate(this);
    }

    /*---------------------.
    | UObjects accessors.  |
    `---------------------*/

    TimerHandle RemoteUContextImpl::setTimer(UTimerCallback* cb)
    {
      cb->call();
      libport::AsyncCallHandler h =
        libport::asyncCall(
                           boost::bind(&RemoteUContextImpl::onTimer, this, cb),
                           useconds_t(cb->period * 1000));
      libport::BlockLock bl(mapLock);
      std::string cbname = "timer" + string_cast(cb);
      timerMap[cbname] = std::make_pair(h, cb);
      return TimerHandle(new std::string(cbname));
    }

    void
    RemoteUContextImpl::onTimer(UTimerCallback* cb)
    {
      std::string cbname = "timer" + string_cast(cb);
      {
        libport::BlockLock bl(mapLock);
        if (!libport::mhas(timerMap, cbname))
          return;
      }
      client_->notifyCallbacks(UMessage(*client_, 0,
                                        externalModuleTag.c_str(),
          ("["
          + string_cast(UEM_TIMER) + ","
          + '"' + cbname + '"'
          +"]").c_str()));

      libport::BlockLock bl(mapLock);
      libport::AsyncCallHandler h =
        libport::asyncCall(
                           boost::bind(&RemoteUContextImpl::onTimer, this, cb),
                           useconds_t(cb->period * 1000));
      timerMap[cbname] = std::make_pair(h, cb);
    }

    bool
    RemoteUObjectImpl::removeTimer(TimerHandle h)
    {
      if (!h)
        return false;
      RemoteUContextImpl* ctx = dynamic_cast<RemoteUContextImpl*>(owner_->ctx_);
      libport::BlockLock bl(ctx->mapLock);
      // Should not happen, but you never know...
      if (!libport::mhas(ctx->timerMap, *h))
        return false;
      ctx->timerMap[*h].first->cancel();
      ctx->timerMap.erase(*h);
      h.reset();
      return true;
    }

    void
    RemoteUContextImpl::call(const std::string& object,
                             const std::string& method,
                             UAutoValue v1,
                             UAutoValue v2,
                             UAutoValue v3,
                             UAutoValue v4,
                             UAutoValue v5,
                             UAutoValue v6,
                             UAutoValue v7,
                             UAutoValue v8)
    {
      std::stringstream s;
      s << object << "." << method <<"(";
#define CHECK(v) if (v.type != DATA_VOID) s << v << ","
      CHECK(v1); CHECK(v2); CHECK(v3); CHECK(v4);
      CHECK(v5); CHECK(v6); CHECK(v7); CHECK(v8);
#undef CHECK
      std::string r = s.str();
      if (v1.type != DATA_VOID)
        r = r.substr(0, r.length() - 1);
      r += ')';
      URBI_SEND_COMMA_COMMAND_C(*client_, r);
      dataSent = true;
    }

    void
    RemoteUContextImpl::declare_event(const UEvent* owner)
    {
      // Event may or may not already exist.
      std::string r = "try{var " + owner->get_name() + " = Event.new()}"
      " catch(var e) {}";
      URBI_SEND_PIPED_COMMAND_C(*client_, r);
      dataSent = true;
    }

    void
    RemoteUContextImpl::emit(const std::string& object,
                             UAutoValue& v1,
                             UAutoValue& v2,
                             UAutoValue& v3,
                             UAutoValue& v4,
                             UAutoValue& v5,
                             UAutoValue& v6,
                             UAutoValue& v7,
                             UAutoValue& v8
                             )
    {
      std::stringstream s;
      s << object << "!(";
#define CHECK(v) if (v.type != DATA_VOID) s << v << ","
      CHECK(v1); CHECK(v2); CHECK(v3); CHECK(v4);
      CHECK(v5); CHECK(v6); CHECK(v7); CHECK(v8);
#undef CHECK
      std::string r = s.str();
      if (v1.type != DATA_VOID)
        r = r.substr(0, r.length() - 1);
      r += ')';
      URBI_SEND_COMMAND_C(*client_, r);
      dataSent = true;
    }

    UValue
    RemoteUContextImpl::localCall(const std::string& object,
                                  const std::string& method,
                                  UAutoValue v1,
                                  UAutoValue v2,
                                  UAutoValue v3,
                                  UAutoValue v4,
                                  UAutoValue v5,
                                  UAutoValue v6,
                                  UAutoValue v7,
                                  UAutoValue v8)
    {
      UAutoValue* vals[] = {&v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8};
      int nargs = 0;
      while (nargs<8 && vals[nargs]->type != DATA_VOID)
        ++nargs;
      std::string name = object + "." + method +"__" + string_cast(nargs);
      UList l;
      {
        // We do not copy the UValues, so do not let the UList destroy them.
        FINALLY(((UList&, l)), l.array.clear());
        for (int i=0; i<nargs; ++i)
          l.array.push_back(vals[i]);
        UTable::callbacks_type tmpfun = functionmap()[name];
        UTable::callbacks_type::iterator tmpfunit = tmpfun.begin();
        if (tmpfunit == tmpfun.end())
        throw std::runtime_error("no callback found for " + object +"::"
                                 + method + " with " + string_cast(nargs)
                                 +" arguments");
        return (*tmpfunit)->__evalcall(l);
      }
    }

    UVarImpl*
    RemoteUContextImpl::getVarImpl()
    {
      return new RemoteUVarImpl();
    }

    UObjectImpl*
    RemoteUContextImpl::getObjectImpl()
    {
      return new RemoteUObjectImpl();
    }

    UGenericCallbackImpl*
    RemoteUContextImpl::getGenericCallbackImpl()
    {
      return new RemoteUGenericCallbackImpl();
    }

    /*-------------.
    | UObjectHub.  |
    `-------------*/

    void
    RemoteUContextImpl::setHubUpdate(UObjectHub*, ufloat)
    {
      // nothing happend in remote mode...
    }
    void
    RemoteUContextImpl::removeHub(UObjectHub*)
    {
    }
    void
    RemoteUContextImpl::registerHub(UObjectHub*)
    {
    }

    UCallbackAction
    RemoteUContextImpl::clientError(const UMessage&)
    {
      closed_ = true;
      /* Destroy everything
       *  We must remove each object from the hash right after deleting it
       * to prevent getUObject requests on deleted items from dtor of other
       * uobjects.
       * Clearing first then deleting might make some UObject fail, since
       * getUObject would return 0 for perfectly valid and accessible UObjects.
       */
      while (!objects.empty())
      {
        objects_type::iterator i = objects.begin();
        delete i->second;
        objects.erase(i);
      }

      while (!hubs.empty())
      {
        hubs_type::iterator i = hubs.begin();
        delete i->second;
        hubs.erase(i);
      }
      return URBI_CONTINUE;
    }
  } // namespace urbi::impl

  /*
    FIXME: find out where it is used
    std::string
    baseURBIStarter::getFullName(const std::string& name) const
    {
    if (local)
    return name + "_" + getClientConnectionID(client_);
    else
    return name;
    }*/
} // namespace urbi
