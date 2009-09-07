/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/// \file libuco/uvar-common.cc

#include <urbi/uobject.hh>
#include <urbi/uvalue.hh>
#include <urbi/uvar.hh>

namespace urbi
{

    /*-----------.
    | UVarImpl.  |
    `-----------*/
  namespace impl
  {
    UVarImpl::~UVarImpl()
    {
    }
  }


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


  void
  UVar::init(const std::string& objname, const std::string& varname,
             impl::UContextImpl* ctx)
  {
    ctx_ = ctx;
    if (!ctx_)
      ctx_ = getCurrentContext();
    impl_ = 0;
    name = objname + '.' + varname;
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
      echo ("Unable to locate variable %s in hashtable. "
	    "UVars being UObject attributes must be binded to be used "
	    "in UObject C++ code. Use UBindVar. "
	    "In any other case, this can be a memory problem. "
	    "Please report bug.\n",
	     name.c_str());
      return false;
    }
  }

  void
  UVar::__init()
  {
    owned = false;
    vardata = 0;
    if (!impl_)
    {
      impl_ = ctx_->getVarImpl();
    }
    impl_->initialize(this);
  }

  UVar::~UVar()
  {
    if (impl_)
      impl_->clean();
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

} // namespace urbi

