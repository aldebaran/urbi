/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/// \file libuobject/uvar.cc

#include <boost/format.hpp>

#include <libport/escape.hh>

#include <urbi/uabstractclient.hh>
#include <urbi/ublend-type.hh>
#include <urbi/uexternal.hh>
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
    createUCallback(dummyUObject->__name,
		    "var",
		    dummyUObject, &UObject::voidfun, name, false);
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
    assert(m->value);
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


  void
  RemoteUVarImpl::set(const UValue& v)
  {
    if (v.type == DATA_BINARY)
    {
      UBinary& b = *(v.binary);
      client_->startPack();
      // K1 only supports a binary at top level within ';' and no other separator.
      if (client_->kernelMajor() < 2)
        URBI_SEND_COMMAND_C((*client_),"");
      (*client_) << owner_->get_name() << "=";
      client_->sendBinary(b.common.data, b.common.size,
                            b.getMessage());
      (*client_) << ";";
      client_->endPack();
    }
    else if (v.type == DATA_STRING)
      URBI_SEND_PIPED_COMMAND_C((*client_), owner_->get_name() << "=\""
                              << libport::escape(*v.stringValue, '"') << '"');
    else
      URBI_SEND_PIPED_COMMAND_C((*client_), owner_->get_name() << "=" << v);
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
    static boost::format
      fmt("{\n"
          "  if (isdef (%s) && !(%s))\n"
          "  {\n"
          "    %s\n"
          "  }\n"
          "  else\n"
          "  {\n"
          "     1/0\n"
          "  }\n"
          "};\n");
    std::string name = owner_->get_name();
    std::string cmd = str(fmt
                          % name
                          % compatibility::isvoid(name.c_str())
                          % name);
    UMessage* m = client_->syncGetTag("%s", tag.c_str(), 0, cmd.c_str());
    if (m->type == MESSAGE_DATA)
      value_ = *m->value;
  }

  void
  RemoteUVarImpl::update(const UValue& v)
  {
    value_ = v;
  }
  }
} //namespace urbi
