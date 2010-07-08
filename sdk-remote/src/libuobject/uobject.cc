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
#include <libport/containers.hh>
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

GD_ADD_CATEGORY(Libuobject);

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
    {
      client_->setCallback(callback(*this, &RemoteUContextImpl::dispatcher),
                           externalModuleTag.c_str());
      client_->setClientErrorCallback(callback(*this,
                 &RemoteUContextImpl::clientError));
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

    void  RemoteUContextImpl::send(const char* a)
    {
      (*client_) << a;
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
      URBI_SEND_PIPED_COMMAND_C((*client),
                                "class " << owner_->__name << "{}");
      URBI_SEND_PIPED_COMMAND_C((*client),
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
        RemoteUContextImpl& ctx = dynamic_cast<RemoteUContextImpl&>
        (*(owner_->ctx_));
        ctx.getClient()->notifyCallbacks(UMessage(*ctx.getClient(), 0,
                                          externalModuleTag.c_str(),
          ("["
          + string_cast(UEM_EVALFUNCTION) + ","
          + '"' + owner_->__name + ".update__0" + '"' + ","
          + "\"\""
          +"]").c_str()));
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
        bool processed = dynamic_cast<USyncClient*>(client_)
          ->processEvents(0);
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
    call_result(UAbstractClient * client, std::string var,
                const UValue& retval, const std::exception* e)
    {
      // var is empty for internally generated messages (such as update())
      if (var.empty())
        return;
      // This method can be called by a thread from the Thread Pool because
      // it is used as a callback function.  Thus we have to declare the
      // category for the debugger used by the current thread.
      GD_CATEGORY(Libuobject);
      GD_FINFO_DUMP("...dispatch of %s done", var);
      if (e)
      {
         URBI_SEND_COMMA_COMMAND_C((*client), "var " << var << "|"
             << var << "=" << "Exception.new(\""
             << "Exception while calling remote bound method: "
             << libport::escape(e->what()) << "\")");
      }
      switch (retval.type)
      {
      case DATA_BINARY:
        // Send it
        // URBI_SEND_COMMAND does not now how to send binary since it
        // depends on the kernel version.
        // Careful, 'var x=1,' has no effect as ',' scopes.
        client->startPack();
        *client << " var  " << var << "|" << var << "=";
        client->send(retval);
        *client << ",";
        client->endPack();
        break;

      case DATA_VOID:
        URBI_SEND_COMMAND_C((*client), "var " << var);
        break;

      default:
        URBI_SEND_COMMA_COMMAND_C((*client), "var " << var << "|"
                                  << var << "=" << retval);
        break;
      }
    }

    UCallbackAction
    RemoteUContextImpl::dispatcher(const UMessage& msg)
    {
      if (closed_)
        return URBI_CONTINUE;
      GD_CATEGORY(Libuobject);
      setCurrentContext(this);
      typedef UTable::callbacks_type callbacks_type;
      //check message type
      if (msg.type != MESSAGE_DATA || msg.value->type != DATA_LIST)
      {
        msg.client.printf("Component Error: "
                          "unknown message content, type %d\n",
                          msg.type);
        return URBI_CONTINUE;
      }

      UList& array = *msg.value->list;

      if (array[0].type != DATA_DOUBLE)
      {
        msg.client.printf("Component Error: "
                          "invalid server message type %d\n",
                          array[0].type);
        return URBI_CONTINUE;
      }

      switch ((USystemExternalMessage)(int)array[0])
      {
      case UEM_ASSIGNVALUE:
      {
        if (array.size() != 3)
        {
          msg.client.printf("Component Error: Invalid number "
                            "of arguments in the server message: %lu"
                            " (expected 3)\n",
                            static_cast<unsigned long>(array.size()));
          return URBI_CONTINUE;
        }
        if (std::list<UVar*> *us = varmap().find0(array[1]))
        {
          foreach (UVar* u, *us)
          {
            if (RemoteUVarImpl* impl = dynamic_cast<RemoteUVarImpl*>(u->impl_))
              impl->update(array[2]);
            else
            {
              GD_FERROR("Unable to cast %x to a RemoteUVarImpl.", u->impl_);
              std::abort();
            }
          }
        }
        if (callbacks_type* cs = monitormap().find0(array[1]))
        {
          foreach (UGenericCallback *c, *cs)
          {
            // test of return value here
            UList u;
            u.array.push_back(new UValue());
            u[0].storage = c->target;
            c->eval(u);
          }
        }
      }
      break;

      case UEM_EVALFUNCTION:
      {
        if (array.size() < 2)
        {
          msg.client.printf("Component Error: Invalid number "
                            "of arguments in the server message: %lu\n",
                            static_cast<unsigned long>(array.size()));
          return URBI_CONTINUE;
        }
        GD_FINFO_DUMP("dispatching call of %s...", array[1]);
	callbacks_type tmpfun = functionmap()[array[1]];
        const std::string var = array[2];
	callbacks_type::iterator tmpfunit = tmpfun.begin();
        if (tmpfunit == tmpfun.end())
          throw std::runtime_error("no callback found");
	array.setOffset(3);
        (*tmpfunit)->eval(array,
                          boost::bind(&call_result, client_, var, _1,_2));
        break;
      }
      break;

      case UEM_EMITEVENT:
        eval_call(eventmap(), array);
        break;

      case UEM_ENDEVENT:
        eval_call(eventendmap(), array);
        break;

      case UEM_NEW:
      {
        objects_type::iterator i = objects.find(std::string(array[2]));
        if (i == objects.end())
          msg.client.printf("No such objects %s\n",
                            std::string(array[2]).c_str());
        else
        {
          baseURBIStarter* bsa = i->second->cloner;
          std::cerr << "instantiating from " << bsa << std::endl;
          std::cerr << "name: " << (std::string) array[1] << std::endl;
          bsa->instanciate(this, (std::string) array[1]);
        }
      }
      break;

      case UEM_DELETE:
      {
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
      default:
        msg.client.printf("Component Error: "
                          "unknown server message type number %d\n",
                          (int)array[0]);
        return URBI_CONTINUE;
      }
      // Send a terminating ';' since code send by the UObject API uses '|'.
      URBI_SEND_COMMAND_C((*client_), "");
      return URBI_CONTINUE;
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
      URBI_SEND_COMMA_COMMAND_C((*client_), r);
    }

    void
    RemoteUContextImpl::declare_event(const UEvent* owner)
    {
      std::string r = "var " + owner->get_name() + " = Event.new()|;";
      URBI_SEND_COMMAND_C((*client_), r);
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
      URBI_SEND_COMMAND_C((*client_), r);
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
