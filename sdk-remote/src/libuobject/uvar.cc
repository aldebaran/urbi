/*
 * Copyright (C) 2005-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuobject/uvar.cc

#include <libport/format.hh>

#include <libport/containers.hh>
#include <libport/debug.hh>
#include <libport/escape.hh>
#include <libport/lexical-cast.hh>

#include <urbi/uabstractclient.hh>
#include <urbi/ublend-type.hh>
#include <urbi/uexternal.hh>
#include <urbi/umessage.hh>
#include <urbi/uobject.hh>
#include <urbi/usyncclient.hh>
#include <urbi/uvalue-serialize.hh>

#include <liburbi/compatibility.hh>

#include <libuobject/remote-ucontext-impl.hh>

namespace urbi
{
  namespace impl
  {

  GD_CATEGORY(Urbi.LibUObject);

  //! UVar initialization
  void
  RemoteUVarImpl::initialize(UVar* owner)
  {
    GD_FINFO_TRACE("RemoteUVarImpl::initialize %s %s", owner->get_name(),
                   this);
    owner_ = owner;
    bypass_ = false;
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    client_ = ctx->backend_;
    LockableOstream* outputStream = ctx->outputStream;
    std::string name = owner_->get_name();
    {
      libport::BlockLock bl(ctx->tableLock);
      UVarTable::callbacks_type& ct = ctx->varmap()[name];
      bool first = ct.empty();
      ct.push_back(owner_);
      if (first)
      {
        value_ = new UValue();
        timestamp_ = new time_t;
      }
      else
      {
        RemoteUVarImpl* impl = static_cast<RemoteUVarImpl*>(ct.front()->impl_);
        value_ = impl->value_;
        timestamp_ = impl->timestamp_;
      }
    }
    URBI_SEND_PIPED_COMMAND_C((*outputStream), "if (!isdef(" << name << ")) var "
                            << name);
    URBI_SEND_PIPED_COMMAND_C
          ((*outputStream),
           libport::format("external var %s from %s",
                           owner_->get_name(), ctx->hookPointName()));
    ctx->markDataSent();
  }

  bool RemoteUVarImpl::setBypass(bool enable)
  {
    bypass_ = enable;
    return true;
  }

  //! UVar out value (read mode)
  ufloat&
  RemoteUVarImpl::out()
  {
    return const_cast<ufloat&>(get().val);
  }

  //! UVar in value (write mode)
  ufloat&
  RemoteUVarImpl::in()
  {
    return const_cast<ufloat&>(get().val);
  }


  void
  RemoteUVarImpl::setProp(UProperty p, const UValue& v)
  {
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    LockableOstream* outputStream = ctx->outputStream;
    URBI_SEND_PIPED_COMMAND_C((*outputStream), owner_->get_name() << "->"
                              << urbi::name(p) << " = " << v);
    ctx->markDataSent();
  }

  void
  RemoteUVarImpl::keepSynchronized()
  {
    //FIXME: do something?
  }

  UValue
  RemoteUVarImpl::getProp(UProperty p)
  {
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    UMessage* m = ctx->syncGet(owner_->get_name() +"->"
                               + urbi::name(p));
    if (!m->value)
      throw std::runtime_error("Error fetching property on "
                               + owner_->get_name());
    UValue res = *m->value;
    delete m;
    return res;
  }

  //! UVar destructor.
  void
  RemoteUVarImpl::clean()
  {
    RemoteUContextImpl* ctx = dynamic_cast<RemoteUContextImpl*>(owner_->ctx_);
    libport::BlockLock bl(ctx->tableLock);
    ctx->varmap().clean(*owner_);
    if (ctx->varmap()[owner_->get_name()].empty())
    {
      delete value_;
      delete timestamp_;
    }
  }

  static
  std::string
  rtp_id()
  {
    // Compute once in some thread implementations, each thread has different
    // PID.
    static std::string res =
      libport::format("URTP_%s_%s", getFilteredHostname(),
#ifdef __UCLIBC__
   "default"
#else
   getpid()
#endif
    );
    return res;
  }

  static std::string makeLinkName(const std::string& key)
  {
    // We cannot have '.' in here, but we want to be able to regenerate the
    // original key unambiguously, so use something unlikely (as in reserved
    // idealy)
    std::string res =  rtp_id() + "___" + key;
    res[res.find_first_of(".")] = '_';
    return res;
  }

  void
  RemoteUContextImpl::makeRTPLink(const std::string& key)
  {
    /* Setup RTP mode
    * We create two instances of the URTP UObject: one local to this
    * remote, and one plugged in the engine, and connect them together.
    */
    // Spawn a new local RTP instance
    std::string localRTP = rtp_id();
    RemoteUContextImpl::objects_type::iterator oi = objects.find(localRTP);
    if (oi == objects.end())
      return;
    baseURBIStarter* bsa = oi->second->cloner;
    std::string linkName = makeLinkName(key);
    GD_SINFO_TRACE("Instanciating local RTP " << linkName);
    bsa->instanciate(this, linkName);
    // Call init
    localCall(linkName, "init");

    // Spawn a remote RTP instance and bind it.
    // Also destroy it when this remote disconnects.
    std::string rLinkName = linkName + "_l";
    URBI_SEND_COMMAND_C
      (*outputStream,
       libport::format("var %s = URTP.new|\n"
                       "%s.sourceContext = lobby.uid|\n",
                       rLinkName, rLinkName));
    // Now asynchronously ask the remote object to listen and to report
    // the port number.
    GD_SINFO_TRACE("fetching engine listen port...");
    backend_->setCallback(
      callback(*this, &RemoteUContextImpl::onRTPListenMessage),
      (URBI_REMOTE_RTP_INIT_CHANNEL + key).c_str());
    URBI_SEND_COMMA_COMMAND_C
      (*outputStream,
       libport::format("Channel.new(\"%s%s\") << %s.listen(\"0.0.0.0\", \"0\")",
                       URBI_REMOTE_RTP_INIT_CHANNEL, key, rLinkName));
    rtpLinks[key]  = 0; // Not ready yet.
  }

  UCallbackAction RemoteUContextImpl::onRTPListenMessage(const UMessage& mport)
  {
    // Second stage of RTP initialization: the remote is listening.
    if (mport.type != MESSAGE_DATA
        || mport.value->type != DATA_DOUBLE)
    {
      GD_SWARN("Failed to get remote RTP port, disabling RTP");
      enableRTP = false;
      return URBI_REMOVE;
    }
    // Extract key from channel.
    std::string key = mport.tag.substr(strlen(URBI_REMOTE_RTP_INIT_CHANNEL),
                                         mport.tag.npos);
    // Regenerate link name
    std::string linkName = makeLinkName(key);
    std::string rLinkName = linkName + "_l";
    // And uvar name
    std::string varname = key;
    size_t p = varname.find("___");
    if (p != varname.npos)
      varname = varname.substr(0, p) + varname.substr(p+3, varname.npos);
    int port = int(mport.value->val);
    GD_FINFO_TRACE("Finishing RTP init, link %s port %s variable %s",
                   linkName, port, varname);
    // Invoke the connect method on our RTP instance. Having a reference
    // to URTP symbols would be painful, so pass through our
    // UGenericCallback mechanism.
    localCall(linkName, "connect", backend_->getRemoteHost(), port);
    UObject* ob = getUObject(linkName);
    // Monitor this RTP link.
    URBI_SEND_COMMA_COMMAND_C(*outputStream,
      "detach('external'.monitorRTP(" << linkName << ","
      << rLinkName << ", closure() {'external'.failRTP}))|"
      << rLinkName << ".receiveVar(\"" << varname
      << "\")");
    rtpLinks[key]  = ob;
    return URBI_REMOVE;
  }

  void
  RemoteUVarImpl::set(const UValue& v)
  {
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    libport::utime_t time = libport::utime();
    if (!owner_->get_local())
      transmit(v, time);
    // Loopback notification
    ctx->assignMessage(owner_->get_name(), v, time, bypass_);
  }

  void
  RemoteUVarImpl::transmitSerialized(const UValue& v, libport::utime_t time)
  {
    GD_INFO_DEBUG("transmitSerialized");
    char av = UEM_ASSIGNVALUE;
    std::string n = owner_->get_name();
    unsigned int tlow = (unsigned int)time;
    unsigned int thi = (unsigned int)(time >> 32);
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    ctx->backend_->startPack();
    ctx->outputStream->flush();
    *static_cast<RemoteUContextImpl*>(owner_->ctx_)->
      oarchive
        << av
        << n
        << v
        << tlow << thi;
     client_->flush();
     ctx->backend_->endPack();
  }

  void
  RemoteUVarImpl::transmit(const UValue& v, libport::utime_t time)
  {
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    std::string fullname = owner_->get_name();
    size_t pos = fullname.rfind(".");
    assert(pos != std::string::npos);
    std::string owner = fullname.substr(0, pos);
    std::string name = fullname.substr(pos + 1);
    GD_FINFO_DUMP("transmit new value for %s", fullname);
    bool rtp = false;
    if (v.type == DATA_BINARY)
    {
      std::string localRTP = rtp_id();
      if (ctx->enableRTP && getUObject(localRTP)
          && owner_->get_rtp() != UVar::RTP_NO)
      {
        GD_SINFO_TRACE("Trying RTP mode using " << localRTP);
        RemoteUContextImpl::RTPLinks::iterator i
          = ctx->rtpLinks.find(owner_->get_name());
        if (i == ctx->rtpLinks.end())
        {
          // Initiate rtp link asynchronously
          GD_INFO_TRACE("Asynchronous RTP link initialization.");
          ctx->makeRTPLink(owner_->get_name());
          goto rtpfail;
        }
        else if (i->second == 0)
        {
          GD_INFO_TRACE("RTP link not ready yet, fallback");
          goto rtpfail; // init started, link not ready yet
        }
        GD_FINFO_TRACE("Link ready, using cache if %s", ctx->rtpSend);
        if (ctx->rtpSend)
          ctx->rtpSend(i->second, v);
        else
          ctx->localCall(i->second->__name, "send", v);
        rtp = true;
      }
    rtpfail:
      if (!rtp)
      {
        if (ctx->serializationMode)
          transmitSerialized(v, time);
        else
        {
          ctx->backend_->startPack();
          *ctx->outputStream
          << owner
          << ".getSlot(\"" << libport::escape(name)
          << "\").update_timed(";
        // Sendbinary is not using the stream, so we must flush.
          ctx->outputStream->flush();
          UBinary& b = *(v.binary);
          ctx->backend_->sendBinary(b.common.data, b.common.size,
                              b.getMessage());
          *ctx->outputStream << ", " << time << ")|";
          ctx->backend_->endPack();
        }
      }
    }
    else
    {
      if (ctx->enableRTP && owner_->get_rtp())
      {
        if (!ctx->sharedRTP_)
        {
          RemoteUContextImpl::RTPLinks::iterator i
          = ctx->rtpLinks.find("_shared_");
          if (i == ctx->rtpLinks.end())
          {
            GD_INFO_DUMP("Async init of RTP shared link");
            ctx->makeRTPLink("_shared_");
            goto rtpfail2;
          }
          else if (!i->second)
          {
            GD_INFO_DUMP("RTP shared link not yet ready");
            goto rtpfail2;
          }
          ctx->sharedRTP_ = i->second;
        }
        GD_INFO_DUMP("localCalling sendGrouped");
        if (ctx->rtpSendGrouped)
          ctx->rtpSendGrouped(ctx->sharedRTP_, owner_->get_name(), v, time);
        else
          ctx->localCall(ctx->sharedRTP_->__name, "sendGrouped",
                         owner_->get_name(), v, time);
        rtp = true;
      }
    rtpfail2:
      if (!rtp)
      {
        if (ctx->serializationMode)
          transmitSerialized(v, time);
        else
        {
          ctx->backend_->startPack();
          *ctx->outputStream
          << owner
          << ".getSlot(\"" << libport::escape(name)
          << "\").update_timed(";
          if (v.type == DATA_STRING)
            (*ctx->outputStream) << "\"" << libport::escape(*v.stringValue, '"') << "\"";
          else
            *ctx->outputStream << v ;
          *ctx->outputStream << ", " << time << ")|";
          ctx->backend_->endPack();
        }
      }
    }
    if (!rtp && !ctx->serializationMode)
    {
      ctx->markDataSent();
    }
    GD_FINFO_DUMP("transmit new value for %s done", fullname);
  }

  const UValue& RemoteUVarImpl::get() const
  {
    return *value_;
  };

  //! set own mode
  void
  RemoteUVarImpl::setOwned()
  {
    owner_->owned = true;
  }

  //! Get Uvalue type
  UDataType
  RemoteUVarImpl::type() const
  {
    return get().type;
  }

  void
  RemoteUVarImpl::request()
  {
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    std::string name = owner_->get_name();
    //build a getvalue message  that will be parsed and returned by the server
    URBI_SEND_PIPED_COMMAND_C((*ctx->outputStream), externalModuleTag << "<<"
                            <<'[' << UEM_ASSIGNVALUE << ","
                            << '"' << name << '"' << ',' << name << ']');
    ctx->markDataSent();
  }

  void
  RemoteUVarImpl::sync()
  {
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    std::string name = owner_->get_name();
    UMessage* m = ctx->syncGet(name + ".uvalueSerialize");
    if (m->type == MESSAGE_DATA)
      value_->set(*m->value);
    delete m;
  }

  time_t
  RemoteUVarImpl::timestamp() const
  {
    return *timestamp_;
  }

  void RemoteUVarImpl::unnotify()
  {
    GD_FINFO_TRACE("RemoteUVarImpl::unnotify on %s (%s)", owner_->get_name(),
                   this);
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    std::string name = owner_->get_name();
    size_t p = name.find_first_of(".");
    if (p == name.npos)
      throw std::runtime_error("unnotify: invalid argument: " + name);
    // Each UVar creation and each notifychange causes an 'external
    // var' message, so when the UVar dies, creation count is
    // callbacks.size +1.
    URBI_SEND_PIPED_COMMAND_C((*ctx->outputStream),
      "UObject.unnotify(\"" << name.substr(0, p) << "\", \""
                            << name.substr(p+1, name.npos) << "\","
                            << callbacks_.size()+1 << ")" );
    libport::BlockLock bl(ctx->tableLock);
    foreach(RemoteUGenericCallbackImpl* c, callbacks_)
    {
      UTable& t =
        dynamic_cast<RemoteUContextImpl*>(c->owner_->ctx_)
        ->tableByName(c->owner_->type);
      UTable::callbacks_type& ct = t[c->owner_->name];
      UTable::callbacks_type::iterator i = libport::find(ct, c->owner_);
      if (i != ct.end())
        ct.erase(i);
      owner_->ctx_->addCleanup(c->owner_); // Will clean the impl_ too.
    }
    callbacks_.clear();
    ctx->markDataSent();
    if (std::list<UVar*> *us = ctx->varmap().find0(name))
      us->remove(owner_);
  }

  void RemoteUVarImpl::useRTP(bool enable)
  {
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    std::string name = owner_->get_name();
    size_t p = name.find_first_of(".");
    if (p == name.npos)
      throw std::runtime_error("invalid argument to useRTP: "+name);
    ctx->send(libport::format("%s.getSlot(\"%s\").rtp = %s|",
                         name.substr(0, p), name.substr(p+1, name.npos),
                         enable ? "true" : "false"));
    ctx->markDataSent();
  }

  void RemoteUVarImpl::setInputPort(bool enable)
  {
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    std::string name = owner_->get_name();
    size_t p = name.find_first_of(".");
    if (p == name.npos)
      throw std::runtime_error("invalid argument to setInputPort: "+name);
    ctx->send(libport::format("%s.getSlot(\"%s\").%s|",
                         name.substr(0, p), name.substr(p+1, name.npos),
                         enable
                         ? "setSlot(\"inputPort\", true)"
                         : "removeLocalSlot(\"inputPort\")"));
    ctx->markDataSent();
  }

  }
} //namespace urbi
