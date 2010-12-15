/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// \file libuco/uevent-common.cc
#include <urbi/ucontext.hh>
#include <urbi/uobject.hh>
#include <urbi/uvalue.hh>

namespace urbi
{

  /*---------.
  | UEvent.  |
  `---------*/
  UEvent::UEvent(const std::string& varname,
                 impl::UContextImpl* impl)
    : UContext(impl)
    , name(varname)
  {
    __init();
  }

  UEvent::UEvent(UObject& obj, const std::string& varname,
                 impl::UContextImpl* impl)
    : UContext(impl)
    , name(obj.__name + '.' + varname)
  {
    __init();
  }

  UEvent::UEvent(const std::string& objname, const std::string& varname,
                 impl::UContextImpl* impl)
    : UContext(impl)
    , name(objname + '.' + varname)
  {
    __init();
  }

  UEvent::UEvent(const UEvent& b)
    : UContext(b.ctx_)
    , name(b.name)
  {
    __init();
  }

  void
  UEvent::init(const std::string& objname, const std::string& varname,
               impl::UContextImpl* ctx)
  {
    ctx_ = ctx;
    if (!ctx_)
      ctx_ = getCurrentContext();
    name = objname + '.' + varname;
    ctx->declare_event(this);
  }

} // namespace urbi
