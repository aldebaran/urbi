/*
 * Copyright (C) 2006-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuco/uvar-common.cc

#include <urbi/ucontext.hh>
#include <urbi/uobject.hh>
#include <urbi/uvalue.hh>
#include <urbi/uvar.hh>

GD_CATEGORY(Urbi.UVar);

namespace urbi
{


  /*-------.
  | UVar.  |
  `-------*/
  UVar::UVar(const std::string& varname, impl::UContextImpl* impl)
    : UContext(impl)
    , VAR_PROP_INIT
    , impl_(0)
    , name(varname)
  {
    __init();
  }

  UVar::UVar(UObject& obj, const std::string& varname, impl::UContextImpl* impl)
    : UContext(impl)
    , VAR_PROP_INIT
    , impl_(0)
    , name(obj.__name + '.' + varname)
  {
    __init();
  }

  UVar::UVar(const std::string& objname, const std::string& varname,
             impl::UContextImpl* impl)
    : UContext(impl)
    , VAR_PROP_INIT
    , impl_(0)
    , name(objname + '.' + varname)
  {
    __init();
  }

  UVar::UVar(const UVar& b)
  : UContext(b.ctx_)
  , VAR_PROP_INIT
  , impl_(0)
  , name (b.name)
  {
    __init();
  }

  void
  UVar::init(const std::string& varname, impl::UContextImpl* ctx)
  {
    GD_FINFO_DUMP("UVar.init %s", varname);
    if (varname == name && ctx == ctx_)
    {
      GD_FINFO_TRACE("Multiple UVar.init on %s @with same parameters, ignoring",
                     varname);
      return;
    }
    if (impl_)
    {
      GD_FINFO_TRACE("UVar.init rebinds from %s to %s", name, varname);
      unnotify();
      impl_->clean();
    }
    delete impl_;
    ctx_ = ctx;
    if (!ctx_)
      ctx_ = getCurrentContext();
    impl_ = 0;
    name = varname;
    __init();
  }

  bool
  UVar::invariant() const
  {
    if (vardata)
      return true;
    else
    {
      // FIXME: this special case should be put in doc.
      GD_FERROR ("Unable to locate variable %s in hashtable. "
                 "UVars being UObject attributes must be bound to be used "
                 "in UObject C++ code.  Use UBindVar.  "
                 "In any other case, this can be a memory problem. "
                 "Please report bug.",
                 name);
      return false;
    }
  }

  void
  UVar::__init()
  {
    owned = false;
    temp = false;
    rtp = RTP_DEFAULT;
    local = false;
    vardata = 0;
    if (!impl_)
    {
      impl_ = ctx_->getVarImpl();
    }
    impl_->initialize(this);
  }

  UVar::~UVar()
  {
    GD_FINFO_DUMP("~UVar %s", name);
    if (impl_)
    {
      unnotify();
      impl_->clean();
    }
    delete impl_;
  }

  void
  UVar::reset(ufloat n)
  {
    *this = n;
  }

  std::ostream&
  operator<< (std::ostream& o, const UVar& u)
  {
    return o << "UVar(\"" << u.get_name() << "\" = " << u.val() << ')';
  }

  UVar&
  uvalue_caster<UVar>::operator() (UValue& v)
  {
    if (v.type == DATA_VOID)
    {
      if (!v.storage)
        throw std::runtime_error("invalid cast to UVar from void value.");
      else
        return *((UVar*)v.storage);
    }
    if (v.type != DATA_STRING)
      throw std::runtime_error("invalid cast to UVar from non-string value.");
    UVar* var = new UVar(*v.stringValue);
    getCurrentContext()->addCleanup(var);
    var->set_temp(true);
    return *var;
  }

  UList&
  UList::operator=(UVar& v)
  {
    (*this) = (UList)v.val();
    return *this;
  }

  libport::utime_t
  UVar::timestamp() const
  {
    check();
    return impl_->timestamp();
  }

  void InputPort::init(UObject* owner, const std::string& name,
                       impl::UContextImpl* ctx)
  {
    UVar::init(owner->__name, name, ctx);
    impl_->setInputPort(true);
  }
  void InputPort::init(const std::string& owner, const std::string& name,
                       impl::UContextImpl* ctx)
  {
    UVar::init(owner, name, ctx);
    impl_->setInputPort(true);
  }
} // namespace urbi
