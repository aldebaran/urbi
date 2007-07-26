/*! \file uobj.cc
 *******************************************************************************

 File: uobj.cc\n
 Implementation of the UObj class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#include <sstream>

#include "libport/cstdio"
#include "libport/containers.hh"

#include "kernel/userver.hh"
#include "kernel/ustring.hh"
#include "kernel/uconnection.hh"
#include "kernel/uvalue.hh"
#include "kernel/uvariable.hh"
#include "ubinder.hh"
#include "ueventhandler.hh"
#include "ufunction.hh"
#include "uobj.hh"
#include "urbi/uobject.hh"
#include "uvariablename.hh"

// **************************************************************************
//! UObj constructor.
UObj::UObj (UString *device)
  : device (new UString(*device)),
    binder (0),
    internalBinder (0)
{
  ::urbiserver->getObjTab ()[this->device->c_str()] = this;
  UValue* objvalue = new UValue();
  objvalue->dataType = DATA_OBJ;
  objvalue->str = new UString(*device);
  new UVariable(this->device->c_str(), objvalue);
}


// Empty event binders.
void
remove (HMbindertab& t, UString& s)
{
  std::list<HMbindertab::iterator> deletelist;
  for (HMbindertab::iterator i = t.begin(); i != t.end(); ++i)
    if (i->second->removeMonitor(s))
      deletelist.push_back(i);

  for (std::list<HMbindertab::iterator>::iterator i = deletelist.begin();
       i != deletelist.end();
       ++i)
    t.erase((*i));
  deletelist.clear();
}

//! UObj destructor
UObj::~UObj()
{
  // Removal of all variable bindings
  std::list<UVariable*> varToDelete;
  for (HMvariabletab::iterator it = ::urbiserver->getVariableTab ().begin();
       it != ::urbiserver->getVariableTab ().end();
       ++it)
    if (!it->second->getMethod().empty()
	&& device
	&& it->second->value->dataType != DATA_OBJ
	&& it->second->getDevicename() == (std::string)device->c_str())
      varToDelete.push_back(it->second);

  libport::deep_clear (varToDelete);

  // clean variables binders (a bit brutal, we scan all wariables...
  // I'll work on an optimized version later)
  for (HMvariabletab::iterator i = ::urbiserver->getVariableTab ().begin();
       i != ::urbiserver->getVariableTab ().end();
       ++i)
    if (i->second->binder
      && i->second->binder->removeMonitor(*device))
    {
      delete i->second->binder;
      i->second->binder = 0;
    }

  // clean functions binders.
  remove(::urbiserver->getFunctionBinderTab (), *device);
  // Clean events binders.
  remove(::urbiserver->getEventBinderTab (), *device);

  // clean the object binder
  if (binder)
  {
    std::ostringstream o;
    o << "[5,\"" << device->c_str() << "\"]\n";

    for (std::list<UMonitor*>::iterator it = binder->monitors.begin();
	 it != binder->monitors.end();
	 ++it)
    {
      (*it)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
      (*it)->c->send((const ubyte*)o.str().c_str(), o.str().size());
    }

    // in fact here we can delete the binder since it contained only bindings
    // to a single object (himself) associated to unique connections:
    delete binder;
  }

  // Remove the object from the hashtable
  HMobjtab::iterator idit = ::urbiserver->getObjTab ().find(device->c_str());
  ASSERT (idit != ::urbiserver->getObjTab ().end())
    ::urbiserver->getObjTab ().erase(idit);

  // Remove the objects from the subclass list of its parents
  for (std::list<UObj*>::iterator i = up.begin();
       i != up.end();
       ++i)
    (*i)->down.remove(this);

  // INTERNAL cleanups
  if (internalBinder && internalBinder->getUObject())
  {
    // here we have two different cases because base classes are not
    // dynamically created and cannot be deleted, they can only
    // to be "cleaned":
    if (internalBinder->getUObject()->derived)
      // this deletes the associated UObject
      delete internalBinder;
    else
      delete internalBinder->getUObject();
  }

  // clean variables internalBinder
  for (HMvariabletab::iterator i = ::urbiserver->getVariableTab ().begin();
       i != ::urbiserver->getVariableTab ().end();
       ++i)
    for (std::list<urbi::UGenericCallback*>::iterator j =
	 i->second->internalBinder.begin();
	 j != i->second->internalBinder.end();
	)
      if ((*j)->objname == device->c_str())
      {
	delete *j;
	j = i->second->internalBinder.erase(j);
      }
      else
	++j;

  // clean variables internalAccessBinder
  for (HMvariabletab::iterator i = ::urbiserver->getVariableTab ().begin();
       i != ::urbiserver->getVariableTab ().end();
       ++i)
    for (std::list<urbi::UGenericCallback*>::iterator j =
	 i->second->internalAccessBinder.begin();
	 j != i->second->internalAccessBinder.end();
	)
      if ((*j)->objname == device->c_str())
      {
	delete *j;
	j = i->second->internalAccessBinder.erase(j);
      }
      else
	++j;

  // final cleanup
  delete device;
}

UFunction*
UObj::searchFunction(const char* id, bool &ambiguous) const
{
  UFunction* ret;
  std::ostringstream o;
  o << device->c_str() << '.' << id;

  // test for pure urbi symbols
  HMfunctiontab::iterator hmf = ::urbiserver->getFunctionTab ().find(o.str().c_str());
  if (hmf != ::urbiserver->getFunctionTab ().end())
  {
    ambiguous = false;
    return hmf->second;
  }

  // test for remote uobjects symbols
  if (::urbiserver->getFunctionBinderTab ().find(o.str().c_str())
      != ::urbiserver->getFunctionBinderTab ().end())
  {
    ambiguous = false;
    return kernel::remoteFunction;
  }

  // test for plugged uobjects symbols
  if (libport::mhas(*::urbi::functionmap, o.str().c_str()))
  {
    ambiguous = false;
    return kernel::remoteFunction;
  }

  // try recursively with parents
  ret = 0;
  bool found = false;
  for (std::list<UObj*>::const_iterator i = up.begin();
       i != up.end();
       ++i)
  {
    UFunction* f = (*i)->searchFunction(id, ambiguous);
    if (ambiguous)
      return 0;
    if (f)
      if (found)
      {
	ambiguous = true;
	return 0;
      }
      else
      {
	ret = f;
	found = true;
      }
  }
  ambiguous = false;
  return ret;
}

UVariable*
UObj::searchVariable(const char* id, bool &ambiguous) const
{
  UVariable* ret;
  std::ostringstream o;
  o << device->c_str() << '.' << id;
  HMvariabletab::iterator hmv = ::urbiserver->getVariableTab ().find(o.str().c_str());
  if (hmv != ::urbiserver->getVariableTab ().end())
  {
    ambiguous = false;
    return hmv->second;
  }
  else
  {
    ret   = 0;
    bool found = false;
    for (std::list<UObj*>::const_iterator i = up.begin();
	 i != up.end();
	 ++i)
    {
      UVariable* tmpres = (*i)->searchVariable(id, ambiguous);
      if (ambiguous)
	return 0;
      if (tmpres)
	if (found)
	{
	  ambiguous = true;
	  return 0;
	}
	else
	{
	  ret = tmpres;
	  found = true;
	}
    }
    ambiguous = false;
    return ret;
  }
}

bool
UObj::searchEvent(const char* id, bool &ambiguous) const
{
  std::ostringstream o;
  o << device->c_str() << '.' << id;
  if (kernel::eventSymbolDefined (o.str().c_str()))
  {
    ambiguous = false;
    return true;
  }
  else
  {
    bool found = false;
    for (std::list<UObj*>::const_iterator i = up.begin();
	 i != up.end();
	 ++i)
    {
      bool tmpres = (*i)->searchEvent(id, ambiguous);
      if (ambiguous)
	return false;
      if (tmpres)
	if (found)
	{
	  ambiguous = true;
	  return false;
	}
	else
	  found = true;
    }
    ambiguous = false;
    return found;
  }
}
