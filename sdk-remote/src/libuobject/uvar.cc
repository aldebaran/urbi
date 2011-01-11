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

  GD_CATEGORY(LibUObject);

  //! UVar initialization
  void
  RemoteUVarImpl::initialize(UVar* owner)
  {
    owner_ = owner;
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    client_ = ctx->backend_;
    LockableOstream* outputStream = ctx->outputStream;
    std::string name = owner_->get_name();
    ctx->varmap()[name].push_back(owner_);
    URBI_SEND_PIPED_COMMAND_C((*outputStream), "if (!isdef(" << name << ")) var "
                            << name);
    URBI_SEND_PIPED_COMMAND_C
          ((*outputStream),
           libport::format("external var %s from dummy",
                           owner_->get_name()));
    ctx->dataSent = true;
  }

  bool RemoteUVarImpl::setBypass(bool enable)
  {
    return !enable;
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
    ctx->dataSent = true;
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

  /*
    UBlendType
    UVar::blend()
    {
    echo("Properties not implemented in remote mode yet.\n");
    return UNORMAL;
    }
  */

  //! UVar destructor.
  void
  RemoteUVarImpl::clean()
  {
    RemoteUContextImpl* ctx = dynamic_cast<RemoteUContextImpl*>(owner_->ctx_);
    ctx->varmap().clean(*owner_);
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

  std::string
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
      return "";
    baseURBIStarter* bsa = oi->second->cloner;
    std::string linkName = localRTP + "_" + key;
    GD_SINFO_TRACE("Instanciating local RTP " << linkName);
    linkName[linkName.find_first_of(".")] = '_';
    bsa->instanciate(this, linkName);
    // Call init
    localCall(linkName, "init");

    // Spawn a remote RTP instance and bind it.
    // Also destroy it when this remote disconnects
    std::string rLinkName = linkName + "_l";
    *outputStream
      << "var " << rLinkName <<" = URTP.new|\n"
      << rLinkName << ".sourceContext = lobby.uid|\n"
      << "disown({var t = Tag.new | t:at(Lobby.onDisconnect?(lobby))\n"
      << "{\n"
      << "  wall(\" destroying lRTP...\")|\n"
      << "  try { " << rLinkName << ".destroy} catch {}|\n"
      << "  t.stop\n"
      << "}})|;" << std::endl;
    GD_SINFO_TRACE("fetching engine listen port...");
    UMessage* mport =
      syncGet(rLinkName +".listen(\"0.0.0.0\", \"0\");");
    if (!mport
        || mport->type != MESSAGE_DATA
        || mport->value->type != DATA_DOUBLE)
    {
      GD_SWARN("Failed to get remote RTP port, disabling RTP");
      enableRTP = false;
      return "";
    }
    int port = int(mport->value->val);
    delete mport;
    GD_SINFO_TRACE("...ok: " << port);
    // Invoke the connect method on our RTP instance. Having a reference
    // to URTP symbols would be painful, so pass through our
    // UGenericCallback mechanism.
    localCall(linkName, "connect", backend_->getRemoteHost(), port);
    UObject* ob = getUObject(linkName);
    rtpLinks[key]  = ob;
    // Monitor this RTP link.
    *outputStream
      << "detach('external'.monitorRTP(" << linkName << ","
      << rLinkName << ", closure() {'external'.failRTP}))|"
      << std::endl;
    return linkName;
  }

  void
  RemoteUVarImpl::set(const UValue& v)
  {
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    libport::utime_t time = libport::utime();
    if (!owner_->get_local())
      transmit(v, time);
    // Loopback notification
    ctx->assignMessage(owner_->get_name(), v, time);
  }

  void
  RemoteUVarImpl::transmitSerialized(const UValue& v, libport::utime_t time)
  {
    GD_INFO_DEBUG("transmitSerialized");
    char av = UEM_ASSIGNVALUE;
    std::string n = owner_->get_name();
    unsigned int tlow = (unsigned int)time;
    unsigned int thi = (unsigned int)(time >> 32);
    static_cast<RemoteUContextImpl*>(owner_->ctx_)->outputStream->flush();
    *static_cast<RemoteUContextImpl*>(owner_->ctx_)->
      oarchive
        << av
        << n
        << v
        << tlow << thi;
     client_->flush();
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
          std::string linkName = ctx->makeRTPLink(owner_->get_name());
          if (linkName.empty())
            goto rtpfail;

          std::string rLinkName = linkName + "_l";
          (*ctx->outputStream)
           << rLinkName << ".receiveVar(\"" << owner_->get_name()
           <<"\")|";
          i = ctx->rtpLinks.find(owner_->get_name());
        }
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
        RemoteUContextImpl::RTPLinks::iterator i
          = ctx->rtpLinks.find("_shared_");
        if (i == ctx->rtpLinks.end())
        {
          std::string linkName = ctx->makeRTPLink("_shared_");
          if (linkName.empty())
            goto rtpfail2;
          i = ctx->rtpLinks.find("_shared_");
        }
        GD_INFO_DUMP("localCalling sendGrouped");
        ctx->localCall(i->second->__name, "sendGrouped",
                         owner_->get_name(), v);
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
    if (!rtp)
    {
      if (ctx->backend_->isCallbackThread() && ctx->dispatchDepth)
      {
        if (!ctx->serializationMode) // No it cannot go in the if above.
          ctx->dataSent = true;
      }
      else // we were not called by dispatch: send the terminating ';' ourselve.
        URBI_SEND_COMMAND_C((*ctx->outputStream), "");
    }
  }

  const UValue& RemoteUVarImpl::get() const
  {
    return value_;
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
    ctx->dataSent = true;
  }

  void
  RemoteUVarImpl::sync()
  {
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    std::string name = owner_->get_name();
    UMessage* m = ctx->syncGet(name + ".uvalueSerialize");
    if (m->type == MESSAGE_DATA)
      value_ = *m->value;
    delete m;
  }

  time_t
  RemoteUVarImpl::timestamp() const
  {
    return timestamp_;
  }

  void
  RemoteUVarImpl::update(const UValue& v, time_t timestamp)
  {
    value_ = v;
    timestamp_ = timestamp;
  }

  void RemoteUVarImpl::unnotify()
  {
    RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
    std::string name = owner_->get_name();
    size_t p = name.find_first_of(".");
    if (p == name.npos)
      throw std::runtime_error("invalid argument to unnotify: "+name);
    // Each UVar creation and each notifychange causes an 'external
    // var' message, so when the UVar dies, creation count is
    // callbacks.size +1.
    ctx->send(libport::format("UObject.unnotify(\"%s\", \"%s\", %s)|",
                         name.substr(0, p), name.substr(p+1, name.npos),
                         callbacks_.size()+1).c_str());
    foreach(RemoteUGenericCallbackImpl* c, callbacks_)
    {
      UTable& t =
        dynamic_cast<RemoteUContextImpl*>(c->owner_->ctx_)
        ->tableByName(c->owner_->type);
      UTable::callbacks_type& ct = t[c->owner_->name];
      UTable::callbacks_type::iterator i = libport::find(ct, c->owner_);
      if (i != ct.end())
        ct.erase(i);
      owner_->ctx_->addCleanup(c->owner_);
      owner_->ctx_->addCleanup(c);
    }
    callbacks_.clear();
    ctx->dataSent = true;
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
    ctx->send(libport::format("%s.getSlot(\"%s\").rtp = %s;",
                         name.substr(0, p), name.substr(p+1, name.npos),
                         enable ? "true" : "false"));
    ctx->dataSent = true;
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
    ctx->dataSent = true;
  }

  }
} //namespace urbi
