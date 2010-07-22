/*
 * Copyright (C) 2005-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuobject/uvar.cc

#include <libport/format.hh>

#include <libport/debug.hh>
#include <libport/escape.hh>
#include <libport/lexical-cast.hh>

#include <urbi/uabstractclient.hh>
#include <urbi/ublend-type.hh>
#include <urbi/uexternal.hh>
#include <urbi/umessage.hh>
#include <urbi/uobject.hh>
#include <urbi/usyncclient.hh>

#include <liburbi/compatibility.hh>

#include <libuobject/remote-ucontext-impl.hh>
namespace urbi
{
  namespace impl
  {

  //! UVar initialization
  void
  RemoteUVarImpl::initialize(UVar* owner)
  {
    owner_ = owner;
    RemoteUContextImpl* ctx = dynamic_cast<RemoteUContextImpl*>(owner_->ctx_);
    client_ = ctx->client_;
    std::string name = owner_->get_name();
    ctx->varmap()[name].push_back(owner_);
    URBI_SEND_PIPED_COMMAND_C((*client_), "if (!isdef(" << name << ")) var "
                            << name);
    UObject* dummyUObject = ctx->getDummyUObject();
    createUCallback(*dummyUObject, owner,
		    "var",
		    dummyUObject, &UObject::voidfun, name);
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
    URBI_SEND_PIPED_COMMAND_C((*client_), owner_->get_name() << "->"
                              << urbi::name(p) << " = " << v);
  }

  void
  RemoteUVarImpl::keepSynchronized()
  {
    //FIXME: do something?
  }

  UValue
  RemoteUVarImpl::getProp(UProperty p)
  {
    UMessage* m = client_->syncGet("%s->%s", owner_->get_name().c_str(),
                                  urbi::name(p));
    aver(m->value);
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
    return libport::format("URTP_%s_%s", getFilteredHostname(), getpid());
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
    *client_ << "var " << rLinkName <<" = URTP.new|"
    <<"disown({var t = Tag.new | t:at(Lobby.onDisconnect?(lobby)) {"
    << "wall(\" destroying lRTP...\")|"
    << "try { " << rLinkName << ".destroy} catch(var e) {};t.stop}})|;";
    GD_SINFO_TRACE("fetching engine listen port...");
    UMessage* mport =
    client_->syncGet(rLinkName +".listen(\"0.0.0.0\", \"0\");");
    if (!mport || mport->type != MESSAGE_DATA
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
    localCall(linkName, "connect", client_->getRemoteHost(),
              port);
    UObject* ob = getUObject(linkName);
    rtpLinks[key]  = ob;
    // Monitor this RTP link.
    (*client_) << "detach('external'.monitorRTP(" << linkName << ","
    << rLinkName << ", closure() {'external'.failRTP}))|" << std::endl;
    return linkName;
  }

  void
  RemoteUVarImpl::set(const UValue& v)
  {
    RemoteUContextImpl* ctx = dynamic_cast<RemoteUContextImpl*>(owner_->ctx_);
    if (v.type == DATA_BINARY)
    {
      bool rtpOK = false;
      std::string localRTP = rtp_id();
      if (ctx->enableRTP && getUObject(localRTP))
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
          (*ctx->client_)
           << rLinkName << ".receiveVar(\"" << owner_->get_name()
           <<"\")|";
          i = ctx->rtpLinks.find(owner_->get_name());
        }
        ctx->localCall(i->second->__name, "send", v);
        rtpOK = true;
      }
    rtpfail:
      if (!rtpOK)
      {
        client_->startPack();
        (*client_) << owner_->get_name() << "=";
        UBinary& b = *(v.binary);
        client_->sendBinary(b.common.data, b.common.size,
                            b.getMessage());
        (*client_) << "|;";
        client_->endPack();
      }
    }
    else
    {
      bool rtp = false;
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
        ctx->localCall(i->second->__name, "sendGrouped",
                         owner_->get_name(), v);
        rtp = true;
      }
    rtpfail2:
      if (!rtp)
      {
        client_->startPack();
        (*client_) << owner_->get_name() << "=";
        if (v.type == DATA_STRING)
          (*client_) << "\"" << libport::escape(*v.stringValue, '"') << "\"|";
        else
          *client_ << v << "|";
        client_->endPack();
      }
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
    std::string name = owner_->get_name();
    //build a getvalue message  that will be parsed and returned by the server
    URBI_SEND_PIPED_COMMAND_C((*client_), externalModuleTag << "<<"
                            <<'[' << UEM_ASSIGNVALUE << ","
                            << '"' << name << '"' << ',' << name << ']');
  }

  void
  RemoteUVarImpl::sync()
  {
    std::string tag(client_->fresh());
    std::string repeatChannel;
    if (client_->kernelMajor() < 2)
      repeatChannel = tag + " << ";
    static boost::format
      fmt("{\n"
          "  if (isdef (%s) && !(%s))\n"
          "  {\n"
          "    %s %s\n"
          "  }\n"
          "  else\n"
          "  {\n"
          "     %s 1/0\n"
          "  }\n"
          "};\n");
    std::string name = owner_->get_name();
    std::string cmd = str(fmt
                          % name
                          % compatibility::isvoid(name.c_str(),
                                                  client_->kernelMajor())
                          % repeatChannel
                          % name
                          % repeatChannel);
    UMessage* m = client_->syncGetTag("%s", tag.c_str(), 0, cmd.c_str());
    if (m->type == MESSAGE_DATA)
      value_ = *m->value;
  }

  void
  RemoteUVarImpl::update(const UValue& v)
  {
    value_ = v;
  }

  void RemoteUVarImpl::unnotify()
  {
    std::string name = owner_->get_name();
    size_t p = name.find_first_of(".");
    if (p == name.npos)
      throw std::runtime_error("Invalid argument to unnotify: "+name);
    send(libport::format(
                         "UObject.unnotify(\"%s\", \"%s\", %s),",
                         name.substr(0, p), name.substr(p+1, name.npos),
                         callbacks_.size()));
    foreach(RemoteUGenericCallbackImpl* c, callbacks_)
    {
      UTable& t =
      dynamic_cast<RemoteUContextImpl*>(c->owner_->ctx_)
        ->tableByName(c->owner_->type);
      UTable::callbacks_type& ct = t[c->owner_->name];
      UTable::callbacks_type::iterator i =
        std::find(ct.begin(), ct.end(), c->owner_);
      if (i != ct.end())
        ct.erase(i);
      owner_->ctx_->addCleanup(c->owner_);
      owner_->ctx_->addCleanup(c);
    }
    callbacks_.clear();
  };
  void RemoteUVarImpl::useRTP(bool enable)
  {
    std::string name = owner_->get_name();
    size_t p = name.find_first_of(".");
    if (p == name.npos)
      throw std::runtime_error("Invalid argument to unnotify: "+name);
    send(name.substr(0, p) + ".getSlot(\"" + name.substr(p+1, name.npos)
         + "\").rtp = " + (enable?"true;":"false;"));
  }
  }
} //namespace urbi
