/// \file libuobject/uobject.cc

#include <iostream>
#include <sstream>
#include <list>
#include <libport/foreach.hh>
#include <libport/lexical-cast.hh>
#include <libport/unistd.h>

#include <libport/debug.hh>
#include <libport/format.hh>

#include <liburbi/compatibility.hh>

#include <urbi/uexternal.hh>
#include <urbi/uobject.hh>
#include <urbi/ustarter.hh>
#include <urbi/usyncclient.hh>
#include <urbi/uvar.hh>
#include <urbi/ucontext-factory.hh>
#include <libuobject/remoteucontextimpl.hh>

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
            c->__evalcall(args);
          args.setOffset(0);
        }
      }
    }

    typedef libport::hash_map<std::string, impl::UContextImpl*> contexts_type;
    static contexts_type contexts;
    impl::UContextImpl* makeRemoteContext(const std::string& host,
                                          const std::string& port)
    {
      impl::UContextImpl* c = new impl::RemoteUContextImpl(
        new USyncClient(host, strtol(port.c_str(), 0, 0)));
      return c;
    }
    impl::UContextImpl* getRemoteContext(const std::string& host,
                                         const std::string& port)
    {
      std::string key = host + ':' + port;
      contexts_type::iterator i = contexts.find(key);
      if (i != contexts.end())
        return i->second;
      impl::UContextImpl* c = new impl::RemoteUContextImpl(
        new USyncClient(host, strtol(port.c_str(), 0, 0)));
      contexts[key] = c;
      return c;
    }

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

    RemoteUContextImpl::RemoteUContextImpl(USyncClient* client)
      : client_(client)
      , dummyUObject(0)
    {
      client_->setCallback(callback(*this, &RemoteUContextImpl::dispatcher),
                           externalModuleTag.c_str());
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

    //! UObject constructor.
    void RemoteUObjectImpl::initialize(UObject* owner)
    {
      //We were called by UObject base constructor.
      period = -1;
      this->owner_ = owner;
      if (owner->__name == "_dummy")
        return;
      owner_->ctx_->registerObject(owner);
      UClient* client = dynamic_cast<RemoteUContextImpl*>(owner_->ctx_)->client_;
      URBI_SEND_PIPED_COMMAND_C((*client),
                                "class " << owner_->__name << "{}");
      URBI_SEND_PIPED_COMMAND_C((*client),
                                "external object " << owner_->__name);
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
      RemoteUContextImpl& ctx = dynamic_cast<RemoteUContextImpl&>
        (*(owner_->ctx_));
      // Forge names for callback and tag
      std::string tagName = "maintimer_" + owner_->__name;
      std::string cbName = owner_->__name + ".maintimer";

      // Stop any previous update
      if (2 <= kernelMajor())
        URBI_SEND_COMMAND_C(*ctx.getClient(),
                            (libport::format("if (!%s.hasLocalSlot(\"%s\"))\n"
                                             "  var %s.%s = Tag.new(\"%s\")|\n"
                                             " %s.%s.stop",
                                             owner_->__name, tagName,
                                             owner_->__name, tagName, tagName,
                                             owner_->__name, tagName)));
      else
        URBI_SEND_COMMAND_C(*ctx.getClient(), "stop " << tagName);

      // Find previous update timer on this object and delete.
      {
        std::string cbFullName = cbName + "__0";
        std::list<UGenericCallback*>& cs = ctx.eventmap()[cbFullName];
        typedef std::list<UGenericCallback*>::iterator iterator;
        for (iterator i = cs.begin(); i != cs.end(); ++i)
          if ((*i)->getName() == cbFullName)
          {
            delete *i;
            cs.erase(i);
            break;
          }
      }

      // Set period value
      period = t;
      // Do nothing more if negative value given
      if (period < 0)
        return;

      // FIXME: setting update at 0 put the kernel in infinite loop
      //        and memory usage goes up to 100%
      if (period == 0)
        period = 1;

      // Create callback
      createUCallback(owner_->__name, "event",
                      owner_, &UObject::update, cbName, false);

      // Set update at given period
      std::string base = 2 <= kernelMajor() ? owner_->__name + "." : "";
      URBI_SEND_COMMAND_C
        (*ctx.getClient(),
         base << tagName << ": every(" << period << "ms)"
         "                     { " << compatibility::emit(cbName) << ";},");
      return;
    }

    void RemoteUContextImpl::yield()
    {
      yield_until(libport::utime());
    }

    void RemoteUContextImpl::yield_until(libport::utime_t deadline)
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

    void RemoteUContextImpl::yield_until_things_changed()
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

    UCallbackAction
    RemoteUContextImpl::dispatcher(const UMessage& msg)
    {
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

      if (array.size() < 2)
      {
        msg.client.printf("Component Error: Invalid number "
                          "of arguments in the server message: %lu\n",
                          static_cast<unsigned long>(array.size()));
        return URBI_CONTINUE;
      }

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
              GD_FERROR("Unable to cast %x to a RemoteUVarImpl.", (u->impl_));
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
            u[0].storage = c->storage;
            c->__evalcall(u);
          }
        }
      }
      break;

      case UEM_EVALFUNCTION:
      {
        GD_FINFO_DUMP("dispatching call of %s...", ((std::string)(array[1])));
	callbacks_type tmpfun = functionmap()[array[1]];
        const std::string var = array[2];
	callbacks_type::iterator tmpfunit = tmpfun.begin();
        if (tmpfunit == tmpfun.end())
          throw std::runtime_error("no callback found");
	array.setOffset(3);
	UValue retval = (*tmpfunit)->__evalcall(array);
	array.setOffset(0);
        GD_FINFO_DUMP("...dispatch of %s done", ((std::string)(array[1])));
	switch (retval.type)
        {
        case DATA_BINARY:
	  // Send it
          // URBI_SEND_COMMAND does not now how to send binary since it
          // depends on the kernel version.
          client_->startPack();
          *client_ << " var  " << var << "=";
          client_->send(retval);
          *client_ << ";";
          client_->endPack();
          break;

        case DATA_VOID:
          URBI_SEND_COMMAND_C((*client_), "var " << var);
          break;

        default:
          URBI_SEND_COMMAND_C((*client_), "var " << var << "=" << retval);
          break;
        }
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

    void RemoteUContextImpl::setTimer(UTimerCallback* cb)
    {
      // Register ourself as an event.
      std::string cbname = "timer" + string_cast(cb);
      std::string event = cb->objname + "." + cbname;
      createUCallback(cb->objname, "event", cb, &UTimerCallback::call,
                      event, false);
      URBI_SEND_COMMAND_C((*client_),
                          "timer_" << cb->objname << ": every("
                          << cb->period << "ms)"
                          "{ " << compatibility::emit(event) << ";}");
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
      r = r.substr(0, r.length() - 1) + ")";
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
