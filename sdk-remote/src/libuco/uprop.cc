/// \file libuco/uprop.cc

// This file is part of UObject Component Architecture
// Copyright (c) 2007, 2008 Gostai S.A.S.
//
// Permission to use, copy, modify, and redistribute this software for
// non-commercial use is hereby granted.
//
// This software is provided "as is" without warranty of any kind,
// either expressed or implied, including but not limited to the
// implied warranties of fitness for a particular purpose.
//
// For more information, comments, bug reports: http://www.urbiforge.com

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
