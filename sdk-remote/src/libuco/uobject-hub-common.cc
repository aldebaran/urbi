/*
 * Copyright (C) 2007-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file libuco/uobject-hub-common.cc

#include <urbi/uobject.hh>

//! Global definition of the starterlist
namespace urbi
{
  /*-------------.
  | UObjectHub.  |
  `-------------*/

  //! UObjectHub constructor.
  UObjectHub::UObjectHub(const std::string& s,  impl::UContextImpl* ctx)
    : UContext(ctx)
    , name(s)
  {
    ctx_->registerHub(this);
  }

  int
  UObjectHub::updateGlobal()
  {
    for (UObjectList::iterator i = members.begin(); i != members.end(); ++i)
      (*i)->update();
    update();
    return 0;
  }

  void
  UObjectHub::addMember(UObject* obj)
  {
    members.push_back(obj);
  }

  void
  UObjectHub::delMember(UObject* obj)
  {
    members.remove (obj);
  }

  UObjectList*
  UObjectHub::getSubClass(const std::string& subclass)
  {
    UObjectList* res = new UObjectList();
    for (UObjectList::iterator i = members.begin(); i != members.end(); ++i)
      if ((*i)->classname == subclass)
	res->push_back(*i);

    return res;
  }

  void
  UObjectHub::USetUpdate(ufloat period)
  {
    ctx_->setHubUpdate(this, period);
  }
  UObjectHub::~UObjectHub()
  {
    ctx_->removeHub(this);
  }
} // namespace urbi
