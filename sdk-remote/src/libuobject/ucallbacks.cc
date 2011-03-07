/*
 * Copyright (C) 2007-2011, Gostai S.A.S.
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

#include <libport/containers.hh>
#include <libport/debug.hh>
#include <libport/lexical-cast.hh>

#include <urbi/ucallbacks.hh>
#include <urbi/usyncclient.hh>
#include <urbi/uexternal.hh>
#include <urbi/uobject.hh>

#include <libuobject/remote-ucontext-impl.hh>

GD_CATEGORY(Urbi.LibUObject);

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
      owner_ = owner;
      std::string type = owner_->type;
      //owner_->name =
      // callback_name(owner_->name, owner_->type, owner_->nbparam);
      GD_FINFO("Registering %s %s %s into %s from %s",
               type,
               owner_->name,
               owner_->nbparam,
               owner_->name,
               owner_->objname);
      LockableOstream& cl
        = *dynamic_cast<RemoteUContextImpl*>(owner_->ctx_)->outputStream;
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
        GD_WARN("NotifyAccess facility is not available for modules in "
                "remote mode.");
    }

    //! UGenericCallback constructor.
    void
    RemoteUGenericCallbackImpl::initialize(UGenericCallback* owner)
    {
      owner_ = owner;
      LockableOstream& cl
        = *dynamic_cast<RemoteUContextImpl*>(owner_->ctx_)->outputStream;
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
      RemoteUContextImpl* ctx = static_cast<RemoteUContextImpl*>(owner_->ctx_);
      GD_FINFO("Pushing %s in %s", owner_->name, owner_->type);
      UTable& t =
        dynamic_cast<RemoteUContextImpl*>(owner_->ctx_)
        ->tableByName(owner_->type);
      t[callback_name(owner_->name, owner_->type, owner_->nbparam)]
        .push_back(owner_);
      if (owner_->target)
      {
        static_cast<RemoteUVarImpl*>(owner_->target->impl_)
          ->callbacks_.push_back(this);
        std::string targetname = owner_->target->get_name();
        // Register the UVar to be updated if it's not there.
        std::list<UVar*> &us = ctx->varmap()[targetname];
        if (!libport::has(us, owner_->target))
          us.push_back(owner_->target);
      }
    }

  }

} // namespace urbi
