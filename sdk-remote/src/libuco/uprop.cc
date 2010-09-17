/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuco/uprop.cc
#include <urbi/uprop.hh>
#include <urbi/uvar.hh>

namespace urbi
{

  UProp::UProp(UVar& owner, UProperty name)
    : owner(owner)
    , name(name)
  {}

  void
  UProp::operator =(const UValue& v)
  {
    owner.setProp(name, v);
  }

  void
  UProp::operator =(const ufloat v)
  {
    owner.setProp(name, v);
  }

  void
  UProp::operator =(const std::string& v)
  {
    owner.setProp(name, v);
  }

  UProp::operator ufloat()
  {
    return static_cast<ufloat>(owner.getProp(name));
  }

  UProp::operator std::string()
  {
    return owner.getProp(name);
  }

  UProp::operator UValue()
  {
    return owner.getProp(name);
  }


} // end namespace urbi
