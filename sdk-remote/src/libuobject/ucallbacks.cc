/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/// \file libuobject/ucallbacks.cc

#include <iostream>
#include <sstream>
#include <list>

#include <libport/debug.hh>
#include <libport/lexical-cast.hh>

#include <urbi/ucallbacks.hh>
#include <urbi/usyncclient.hh>
#include <urbi/uexternal.hh>
#include <urbi/uobject.hh>

#include <libuobject/remote-ucontext-impl.hh>

GD_ADD_CATEGORY(Libuobject);

namespace urbi
{

  namespace
  {
    static
    std::string
    callback_name(const std::string& name, const std::string& type,
                  int size)
    {
      std::string res = name;
      if (type == "function" || type == "event" || type == "eventend")
        res += "__" + string_cast(size);
      return res;
    }
  }

  namespace impl
  {
    void
    RemoteUGenericCallbackImpl::initialize(UGenericCallback* owner, bool)
    {
      GD_CATEGORY(Libuobject);
      owner_ = owner;
      std::string type = owner_->type;
      //owner_->name = callback_name(owner_->name, owner_->type, owner_->nbparam);
      GD_FINFO("Registering %s %s %s into %s from %s",
               (type)(owner_->name)(owner_->nbparam)
               (owner_->name)(owner_->objname));
      UClient& cl = *dynamic_cast<RemoteUContextImpl*>(owner_->ctx_)->client_;
      if (type == "var")
        URBI_SEND_PIPED_COMMAND_C
          (cl,
           libport::format("external %s %s from %s",
                           type, owner_->name, owner_->objname));
      else if (type == "event" || type == "function")
        URBI_SEND_PIPED_COMMAND_C
          (cl,
           libport::format("external %s (%s) %s from %s",
                           type, owner_->nbparam, owner_->name,
                           owner_->objname));
      else if (type == "varaccess")
        echo("Warning: NotifyAccess facility is not available for modules in "
             "remote mode.\n");
    }

    //! UGenericCallback constructor.
    void
    RemoteUGenericCallbackImpl::initialize(UGenericCallback* owner)
    {
      owner_ = owner;
      UClient& cl = *dynamic_cast<RemoteUContextImpl*>(owner_->ctx_)->client_;
      URBI_SEND_PIPED_COMMAND_C
        (cl,
           libport::format("external %s %s",
                           owner->type, owner_->name));
    }

    void
    RemoteUGenericCallbackImpl::clear()
    {
    }

    void
    RemoteUGenericCallbackImpl::registerCallback()
    {
      GD_CATEGORY(Libuobject);
      GD_FINFO("Pushing %s in %s", (owner_->name) (owner_->type));
      UTable& t =
        dynamic_cast<RemoteUContextImpl*>(owner_->ctx_)
        ->tableByName(owner_->type);
      t[callback_name(owner_->name, owner_->type, owner_->nbparam)]
        .push_back(owner_);
    }

  }

} // namespace urbi
