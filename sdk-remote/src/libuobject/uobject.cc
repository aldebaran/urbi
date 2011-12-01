/*
 * Copyright (C) 2005-2011, Gostai S.A.S.
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
#include <libport/io-stream.hh>
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
#include <urbi/uvalue-serialize.hh>
#include <urbi/uvar.hh>
#include <urbi/ucontext-factory.hh>
#include <libuobject/remote-ucontext-impl.hh>

GD_CATEGORY(Urbi.LibUObject);

#define REQUIRE(Cond, ...)                              \
  do {                                                  \
    if (!(Cond))                                        \
    {                                                   \
      msg.client.printf(__VA_ARGS__);                   \
      GD_FERROR("Message content: %s", *msg.value);     \
      return URBI_CONTINUE;                             \
    }                                                   \
  } while (false)

class HookPoint: public urbi::UObject
{
public:
  HookPoint(const std::string&n, urbi::impl::UContextImpl* impl)
  : UObject(n, impl)
  {
    UBindFunction(HookPoint, init);
  }
  int init()
  {
    return 1;
  }
};
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
      return new impl::RemoteUContextImpl(
        new USyncClient(host, lexical_cast<unsigned>(port)));
    }

    impl::UContextImpl*
    getRemoteContext(const std::string& host, const std::string& port)
    {
      std::string key = host + ':' + port;
      contexts_type::iterator i = contexts.find(key);
      if (i != contexts.end())
        return i->second;
      return contexts[key] = makeRemoteContext(host, port);
    }

    class SerializedUrbiscriptStreamBuffer: public libport::StreamBuffer
    {
    public:
      SerializedUrbiscriptStreamBuffer(RemoteUContextImpl* backend)
        : backend_(backend)
      {
      }
    protected:
      virtual size_t read(char*, size_t)
      {
        return 0;
      }
      virtual void write(char* buffer, size_t size)
      {
        int p = size-1;
        while (p && (buffer[p]==' ' || buffer[p] == '\n')) --p;
        GD_FINFO_TRACE(
          "Flushing %s bytes of serialized urbiscript ending with '%s'", size,
                       buffer[p]);
        if (!strchr(",;|&", buffer[p]))
          GD_FWARN("Serialized message may be incomplete: %s",
                 std::string(buffer, size));
        char code = UEM_EVAL;
        std::string msg(buffer, size);
        backend_->backend_->startPack();
        *backend_->oarchive << code << msg;
        backend_->backend_->flush();
        backend_->backend_->endPack();
      }
    private:
      RemoteUContextImpl* backend_;
    };
    /*---------------------.
    | RemoteUContextImpl.  |
    `---------------------*/

    RemoteUContextImpl::RemoteUContextImpl(USyncClient* client)
      : backend_(client)
      , closed_(false)
      , dummyUObject(0)
      , enableRTP(true)
      , dispatchDepth(0)
      , outputStream(client)
      , dataSent(false)
      , serializationMode(false)
      , oarchive(0)
      , sharedRTP_(0)
    {
      rtpSend = 0;
      rtpSendGrouped = 0;
      hookPointName_ = libport::format("hookPoint_%s_%s",
                                       getFilteredHostname(),
 #ifdef __UCLIBC__
   "default"
#else
   getpid()
#endif
      );
      backend_->setCallback(callback(*this, &RemoteUContextImpl::dispatcher),
                           externalModuleTag.c_str());
      backend_->setClientErrorCallback(callback(*this,
                 &RemoteUContextImpl::clientError));

      typedef long long int us_t;
      UMessage* m = backend_->syncGet(
        "[System.timeReference.year,"
         "System.timeReference.month,"
         "System.timeReference.day,"
         "System.timeReference.us,"
        // New versions of Urbi register "Urbi" as a component name,
        // but keep backward compatibility on "Urbi SDK".  So use the
        // latter for a good while.
         "PackageInfo.components[\"Urbi SDK\"].major,"
         "PackageInfo.components[\"Urbi SDK\"].minor,"
         "PackageInfo.components[\"Urbi SDK\"].subMinor,"
         "PackageInfo.components[\"Urbi SDK\"].patch,"
         "]"
         );
      int year  = (*m->value->list)[0];
      int month = (*m->value->list)[1];
      int day   = (*m->value->list)[2];
      int us    = (*m->value->list)[3];
      version = libport::PackageInfo::Version((*m->value->list)[4],
                                              (*m->value->list)[5],
                                              (*m->value->list)[6],
                                              (*m->value->list)[7]);
      // Compatibility for wire protocol 2.3-2.4.
      URBI_SEND_COMMAND_C
        (*outputStream,
         "if (!Object.hasSlot(\"uvalueSerialize\"))\n"
         "  function Object.uvalueSerialize() { this }");
      // Compatibility for versions < 2.7
      URBI_SEND_COMMAND_C
        (*outputStream,
         "if (!UObject.hasSlot(\"syncGet\"))\n"
         "  function UObject.syncGet(exp, tag)\n"
         "  {\n"
         "    try { Channel.new(tag) << eval(exp) }\n"
         "    catch (var e) { lobby.send(\"!!! \" + e, tag) }\n"
         "  }");

      boost::posix_time::ptime now
        (boost::posix_time::microsec_clock::local_time());
      boost::posix_time::ptime ref(boost::gregorian::date(year, month, day),
                                   boost::posix_time::microseconds(us));
      libport::utime_reference_set
        (libport::utime() - (now - ref).total_microseconds());
      GD_FINFO_DEBUG("Remote kernel reference timestamp: %s.",
                     to_simple_string(ref));
      GD_FINFO_DEBUG("Remote kernel version: %s", version);
      // Connect hookPoint
      setCurrentContext(this);
      new HookPoint(hookPointName_, const_cast<RemoteUContextImpl*>(this));
      URBI_SEND_COMMAND_C(*outputStream, "var hookPoint = "
                          + hookPointName_+"|");
    }

    RemoteUContextImpl::~RemoteUContextImpl()
    {}

    std::string RemoteUContextImpl::hookPointName()
    {
      return hookPointName_;
    }

    USyncClient*
    RemoteUContextImpl::getClient()
    {
      return backend_;
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
      if (!serializationMode)
        unarmorAndSend(a, backend_);
      else
      {
        backend_->startPack();
        size_t len = strlen(a);
        if (2 <= len && a[0] == '(')
          *outputStream << std::string(a+1, len-2);
        else
          *outputStream << std::string(a, len);
        markDataSent();
        backend_->endPack();
      }
    }

    void RemoteUContextImpl::send(const char* a)
    {
      if (closed_)
        GD_FWARN("Write on closed remote context: \"%s\"", libport::escape(a));
      else
      {
        backend_->startPack();
        *outputStream << a;
        outputStream->flush();
        backend_->endPack();
      }
    }

    void RemoteUContextImpl::send(const void* buf, size_t size)
    {
      if (closed_)
        GD_FWARN("Write on closed remote context: \"%s\"",
                 libport::escape(std::string((const char*) buf, size)));
      else
      {
        backend_->startPack();
        outputStream->rdbuf()->sputn(static_cast<const char*> (buf), size);
        outputStream->flush();
        backend_->endPack();
      }
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
      throw std::runtime_error("unexpected table name: " + n);
    }

    std::pair<int, int>
    RemoteUContextImpl::kernelVersion() const
    {
      backend_->waitForKernelVersion(true);
      return std::make_pair(backend_->kernelMajor(),
                            backend_->kernelMinor());
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
      return backend_->get_io_service();
    }

    /*--------------------.
    | RemoteUObjectImpl.  |
    `--------------------*/

    RemoteUObjectImpl::~RemoteUObjectImpl()
    {}

    //! UObject constructor.
    void RemoteUObjectImpl::initialize(UObject* owner)
    {
      static int uid = 0;
      this->owner_ = owner;
      RemoteUContextImpl* ctx = dynamic_cast<RemoteUContextImpl*>(owner_->ctx_);
      //We were called by UObject base constructor.
      period = -1;
      if (owner->__name == "_dummy")
        return;
      bool fromcxx = owner_->__name.empty();
      if (fromcxx)
        owner_->__name = "uob_" +  getFilteredHostname() + string_cast(++uid);
      LockableOstream* client = ctx->outputStream;
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
      // ...unless instanciation was made from c++.
      if (fromcxx)
      { // Delay calls to register functions until UObject constructor finishes,
        // othewrise typeid is wrong.
        ctx->addCleanup(boost::bind(&RemoteUContextImpl::instanciated,
                               ctx, owner));
        ctx->addCleanup(boost::bind(&UContextImpl::registerObject, ctx, owner));
      }
      else
        owner_->ctx_->registerObject(owner);
    }

    //! UObject cleaner
    void
    RemoteUObjectImpl::clean()
    {
      RemoteUContextImpl& ctx = dynamic_cast<RemoteUContextImpl&>
        (*(owner_->ctx_));

      if (updateHandler)
      {
        updateHandler->cancel();
        updateHandler.reset();
      }

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
        UMessage m(*ctx.backend_);
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
          dynamic_cast<USyncClient*>(backend_)->processEvents(0);
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
        if (dynamic_cast<USyncClient*>(backend_)->processEvents(0))
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
    call_result(RemoteUContextImpl* ctx, std::string var,
                const UValue& retval, const std::exception* e)
    {
      GD_FINFO_DUMP("... dispatch %s done", var);
      // var is empty for internally generated messages (such as update())
      if (var.empty())
        return;
      // This method can be called by a thread from the Thread Pool because
      // it is used as a callback function.  Thus we have to declare the
      // category for the debugger used by the current thread.
      if (e)
      {
         URBI_SEND_COMMA_COMMAND_C
           (*ctx->outputStream,
            "Global.UObject.funCall(\"" << var << "\", "
            << "Exception.new(\""
            << "Exception while calling remote bound method: "
            << libport::escape(e->what())
            << "\"))");
      }
      else
      {
        if (ctx->serializationMode)
        {
          char type = UEM_REPLY;
          ctx->outputStream->flush();
          *ctx->oarchive << type << var << retval;
          ctx->backend_->flush();
        }
        else
        switch (retval.type)
        {
        case DATA_BINARY:
          // Send it
          // URBI_SEND_COMMAND does not now how to send binary since it
          // depends on the kernel version.
          ctx->backend_->startPack();
          *ctx->backend_ << "Global.UObject.funCall(\"" << var << "\", ";
          ctx->backend_->send(retval);
          *ctx->backend_ << "),\n";
          ctx->backend_->endPack();
          ctx->backend_->flush();
          break;

        case DATA_VOID:
          URBI_SEND_COMMAND_C
            (*ctx->outputStream,
             "Global.UObject.funCall(\"" << var << "\")");
          break;

        default:
          URBI_SEND_COMMA_COMMAND_C
            (*ctx->outputStream,
             "Global.UObject.funCall(\"" << var << "\", " << retval << ")");
          break;
        }
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
      GD_FINFO_DUMP("Dispatching %s, first %s", array, array[0]);
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
                "Component Error: invalid number "
                "of arguments in the server message: %lu (expected 4)\n",
                static_cast<unsigned long>(array.size()));
        assignMessage(array[1], array[2], array[3]);
        break;

      case UEM_EVALFUNCTION:
        REQUIRE(3 <= array.size(),
                "Component Error: invalid number "
                "of arguments in the server message: %lu\n",
                static_cast<unsigned long>(array.size()));
        REQUIRE(array[2].type == DATA_STRING,
                "Component Error, argument 2 to function call is not a"
                "stirng\n");
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
        REQUIRE(bsa, "Object %s has no cloner", std::string(array[2]).c_str());
        GD_FINFO_DEBUG("instantiating from %s (%s), name: %s", bsa, array[2],
                       array[1]);
        bsa->instanciate(this, (std::string) array[1]);
      }
      break;

      case UEM_DELETE:
      {
        impl::UContextImpl::CleanupStack s_(*this);
        objects_type::iterator i = objects.find(std::string(array[1]));
        if (i == objects.end())
          GD_FWARN("delete: no such object: %s", array[1]);
        else
        {
          delete i->second;
          objects.erase(i);
          if (objects.size() == 1 )
          {
            // All the instances have been deleted, except the hookpoint we
            // created automaticaly, we're done with this.
            // remote.
            GD_INFO_TRACE("Last instance deleted, quit");
            exit(0);
          }
        }
      }
      break;

      case UEM_INIT:
        init();
        // switch to binary mode
        if (!getenv("URBI_TEXT_MODE")
            && version >= libport::PackageInfo::Version(2, 6, 0, 13))
        {
          GD_INFO_TRACE("Switching to binary mode");
          setSerializationMode(true);
        }
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

      case UEM_SETRTP:
        REQUIRE(array.size() == 3,
                "Component Error: invalid number "
                "of arguments in the server message: %lu (expected 3)\n",
                static_cast<unsigned long>(array.size()));
        setRTPMessage(array[1], array[2]);
        break;

      case UEM_SETLOCAL:
      {
        REQUIRE(array.size() == 3,
                "Component Error: invalid number "
                "of arguments in the server message: %lu (expected 3)\n",
                static_cast<unsigned long>(array.size()));
        std::string name = array[1];
        bool state = array[2];
        GD_FINFO_LOG("Set local mode to %s on %s", state, name);
        {
          libport::BlockLock bl(tableLock);
          if (std::list<UVar*> *us = varmap().find0(name))
          {
            foreach(UVar* v, *us)
              v->set_local(state);
          }
        }
      }
      break;

      default:
        REQUIRE(false,
                "Component Error: unknown server message type number %d\n",
                (int)array[0]);
      }

      // Send a terminating ';' since code send by the UObject API uses '|'.
      // But only in outermost dispatch call
      GD_FINFO_DUMP("Flush check: depth %s, data sent %s", dispatchDepth,
                    dataSent);
      if (dispatchDepth == 1 && dataSent)
      {
        URBI_SEND_COMMAND_C(*outputStream, "");
        dataSent = false;
      }
      return URBI_CONTINUE;
    }

    void
    RemoteUContextImpl::assignMessage(const std::string& name,
                                      const UValue& v, time_t ts,
                                      bool bypass,
                                      UValue* val, time_t* timestamp)
    {
      libport::BlockLock bl(tableLock);
      std::list<UVar*> *us = 0;
      bool cachedVal = val;
      // Fetch storage UValue if it was not given to us
      if (!val)
      {
        us = varmap().find0(name); // Do not make this call if cachedVal
        if (us && !us->empty())
        {
          // Get first UVarImpl to get pointers to val and timestamp
          RemoteUVarImpl* vimpl =
          static_cast<RemoteUVarImpl*>(us->front()->impl_);
          val = vimpl->value_;
          timestamp = vimpl->timestamp_;
        }
      }
      if (val)
      {
        val->set(v, bypass);
        *timestamp = ts;
      }
      // Process notifyChange
      if (UTable::callbacks_type* cs = monitormap().find0(name))
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
      /* Reset val to empty uvalue in bypass mode
       * if val was not given to us as argument, maybe it was destroyed since
       * we calculated it. So check that at least one UVar is still present.
       */
      if (bypass && (cachedVal || (us && !us->empty())))
      { // Reset to void
        val->set(UValue());
      }
    }

    void
    RemoteUContextImpl::evalFunctionMessage(const std::string& name,
                                            const std::string& var,
                                            UList& args)
    {
      GD_FINFO_DUMP("dispatch call %s = %s...", var, name);
      UGenericCallback* cb = 0;
      {
        libport::BlockLock bl(tableLock);
        UTable::callbacks_type funs = functionmap()[name];
        UTable::callbacks_type::iterator i = funs.begin();
        if (i == funs.end())
          throw std::runtime_error("no callback found");
        cb = *i;
      }
      args.setOffset(3);
      cb->eval(args,
                 boost::bind(&call_result, this, var, _1, _2));
      GD_INFO_DUMP("dispatch call over, async call_result");
    }

    void
    RemoteUContextImpl::setRTPMessage(const std::string& varname,
                                      int state)
    {
      libport::BlockLock bl(tableLock);
      if (std::list<UVar*> *us = varmap().find0(varname))
      {
        foreach (UVar* u, *us)
        {
          u->useRTP(state?UVar::RTP_YES: UVar::RTP_NO);
        }
      }
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
        libport::asyncCall(boost::bind(&RemoteUContextImpl::onTimer, this, cb),
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
      backend_->notifyCallbacks
        (UMessage(*backend_, 0, externalModuleTag,
                  libport::format("[%s,\"%s\"]", UEM_TIMER, cbname)));

      libport::BlockLock bl(mapLock);
      libport::AsyncCallHandler h =
        libport::asyncCall(boost::bind(&RemoteUContextImpl::onTimer, this, cb),
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
                             UAutoValue v6)
    {
      std::stringstream s;
      s << object << "." << method <<"(";
#define CHECK(v) if (v.type != DATA_VOID) s << v << ","
      CHECK(v1); CHECK(v2); CHECK(v3); CHECK(v4);
      CHECK(v5); CHECK(v6);
#undef CHECK
      std::string r = s.str();
      if (v1.type != DATA_VOID)
        r = r.substr(0, r.length() - 1);
      r += ')';
      URBI_SEND_COMMA_COMMAND_C(*outputStream, r);
      markDataSent();
    }

    void
    RemoteUContextImpl::declare_event(const UEvent* owner)
    {
      // Event may or may not already exist.
      std::string r = "try{var " + owner->get_name() + " = Event.new()}"
      " catch(var e) {}";
      URBI_SEND_PIPED_COMMAND_C(*outputStream, r);
      markDataSent();
    }

    void
    RemoteUContextImpl::emit(const std::string& object,
                             UAutoValue& v1,
                             UAutoValue& v2,
                             UAutoValue& v3,
                             UAutoValue& v4,
                             UAutoValue& v5,
                             UAutoValue& v6,
                             UAutoValue& v7)
    {
      if (serializationMode)
      {
        UAutoValue* vals[] = {&v1, &v2, &v3, &v4, &v5, &v6, &v7};
        int i = 0;
        while (i<7 && vals[i]->type != DATA_VOID)
          ++i;
        outputStream->flush();
        char code = UEM_EMITEVENT;
        *oarchive << code << object << i;
        for (int t=0; t<i; ++t)
          *oarchive << *(UValue*)vals[t];
        backend_->flush();
        return;
      }
      std::stringstream s;
      s << object << "!(";
#define CHECK(v) if (v.type != DATA_VOID) s << v << ","
      CHECK(v1); CHECK(v2); CHECK(v3); CHECK(v4);
      CHECK(v5); CHECK(v6); CHECK(v7);
#undef CHECK
      std::string r = s.str();
      if (v1.type != DATA_VOID)
        r = r.substr(0, r.length() - 1);
      r += ')';
      URBI_SEND_COMMAND_C(*outputStream, r);
      markDataSent();
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
        UGenericCallback* cb;
        {
          libport::BlockLock bl(tableLock);
          UTable::callbacks_type tmpfun = functionmap()[name];
          UTable::callbacks_type::iterator tmpfunit = tmpfun.begin();
          if (tmpfunit == tmpfun.end())
          throw std::runtime_error("no callback found for " + object +"::"
                                   + method + " with " + string_cast(nargs)
                                   +" arguments");
          cb = *tmpfunit;
        }
        return cb->__evalcall(l);
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
      GD_INFO_TRACE("clientError on remote context");
      if (closed_)
      {
        GD_WARN("ClientError already processed");
        return URBI_CONTINUE;
      }
      impl::UContextImpl::CleanupStack s_(*this);
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
        GD_FINFO_TRACE("Destroying object %s", i->second);
        delete i->second;
        objects.erase(i);
      }

      while (!hubs.empty())
      {
        hubs_type::iterator i = hubs.begin();
        GD_FINFO_TRACE("Destroying hub %s", i->second);
        delete i->second;
        hubs.erase(i);
      }
      return URBI_CONTINUE;
    }

    void RemoteUContextImpl::setSerializationMode(bool mode)
    {
      if (mode == serializationMode)
        return;
      if (!mode)
        throw std::runtime_error("Serialization mode can not be undone");
      serializationMode = mode;
      // Notify the kernel, in the current mode.
      // Do not use call, we must be foreground.
      // Do not send any other message until we get a reply.
      backend_->lockQueue();
      const char* tag = "remotecontext_setmode";
      send(libport::format("binaryMode(%s, \"%s\");\n", mode, tag));
      delete backend_->waitForTag(tag, 0);
      // Change the urbiscript outputstream to one that encapsulates
      // in UValue and serializes.
      if (mode)
      {
        if (!oarchive)
          oarchive = new libport::serialize::BinaryOSerializer(*backend_);
        outputStream =
          new LockableOstream(new SerializedUrbiscriptStreamBuffer(this));
      }
      else
      {
        // Reset outputstream to the USyncClient.
        delete outputStream->rdbuf();
        delete outputStream;
        outputStream = backend_;
      }
    }

    UMessage*
    RemoteUContextImpl::syncGet(const std::string& exp,
                                libport::utime_t timeout)
    {
      static int counter = 0;
      counter++;
      std::string tag = "remotecontext_" + string_cast(counter);
      backend_->lockQueue();
      call("UObject", "syncGet", exp, tag);
      return backend_->waitForTag(tag, timeout);
    }

    void
    RemoteUContextImpl::markDataSent()
    {
      if (backend_->isCallbackThread() && dispatchDepth)
        dataSent = true;
      else // we were not called by dispatch: send the terminating ';' ourselve.
        URBI_SEND_COMMAND_C((*outputStream), "");
    }
  } // namespace urbi::impl

  /*
    FIXME: find out where it is used
    std::string
    baseURBIStarter::getFullName(const std::string& name) const
    {
    if (local)
    return name + "_" + getClientConnectionID(outputStream);
    else
    return name;
    }*/
} // namespace urbi
