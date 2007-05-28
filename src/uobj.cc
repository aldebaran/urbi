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

#include "libport/cstdio"
#include "libport/containers.hh"

#include "ubinder.hh"
#include "uconnection.hh"
#include "ueventhandler.hh"
#include "ufunction.hh"
#include "uobj.hh"
#include "urbi/uobject.hh"
#include "userver.hh"
#include "ustring.hh"
#include "uvalue.hh"
#include "uvariable.hh"
#include "uvariablename.hh"

// **************************************************************************
//! UObj constructor.
UObj::UObj (UString *device)
  : device (new UString(device)),
    binder (0),
    internalBinder (0)
{
  ::urbiserver->objtab[this->device->str()] = this;
  UValue* objvalue = new UValue();
  objvalue->dataType = DATA_OBJ;
  objvalue->str = new UString(device);
  new UVariable(this->device->str(), objvalue);
}

//! UObj destructor
UObj::~UObj()
{
  // Removal of all variable bindings
  std::list<UVariable*> varToDelete;
  for (HMvariabletab::iterator  it = ::urbiserver->variabletab.begin();
       it != ::urbiserver->variabletab.end();
       ++it)
    if (it->second->method
	&& it->second->devicename
	&& device
	&& it->second->value->dataType != DATA_OBJ
	&& it->second->devicename->equal(device))
      varToDelete.push_back(it->second);

  libport::deep_clear (varToDelete);

  // clean variables binders (a bit brutal, we scan all wariables...
  // I'll work on an optimized version later)
  for (HMvariabletab::iterator i = ::urbiserver->variabletab.begin();
       i != ::urbiserver->variabletab.end();
       ++i)
    if (i->second->binder
      && i->second->binder->removeMonitor(device))
    {
      delete i->second->binder;
      i->second->binder = 0;
    }

  std::list<HMbindertab::iterator> deletelist;
  //clean functions binders
  for (HMbindertab::iterator i = ::urbiserver->functionbindertab.begin();
       i != ::urbiserver->functionbindertab.end();
       ++i)
    if (i->second->removeMonitor(device))
      deletelist.push_back(i);

  for (std::list<HMbindertab::iterator>::iterator i = deletelist.begin();
       i != deletelist.end();
       ++i)
    ::urbiserver->functionbindertab.erase((*i));
  deletelist.clear();

  //clean events binders
  for (HMbindertab::iterator i = ::urbiserver->eventbindertab.begin();
       i != ::urbiserver->eventbindertab.end();
       ++i)
    if (i->second->removeMonitor(device))
      deletelist.push_back(i);

  for (std::list<HMbindertab::iterator>::iterator i = deletelist.begin();
       i != deletelist.end();
       ++i)
    ::urbiserver->eventbindertab.erase((*i));
  deletelist.clear();

  // clean the object binder
  if (binder)
  {
    char messagetosend[1024];
    snprintf(messagetosend, 1024, "[5,\"%s\"]\n", device->str());

    for (std::list<UMonitor*>::iterator it = binder->monitors.begin();
	 it != binder->monitors.end();
	 ++it)
    {
      (*it)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
      (*it)->c->send((const ubyte*)messagetosend, strlen(messagetosend));
    }

    // in fact here we can delete the binder since it contained only bindings
    // to a single object (himself) associated to unique connections:
    delete binder;
  }

  // Remove the object from the hashtable
  HMobjtab::iterator idit = ::urbiserver->objtab.find(device->str());
  ASSERT (idit != ::urbiserver->objtab.end())
    ::urbiserver->objtab.erase(idit);

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
    else if (internalBinder->getUObject())
      delete internalBinder->getUObject();
  }

  // clean variables internalBinder
  for (HMvariabletab::iterator i = ::urbiserver->variabletab.begin();
       i != ::urbiserver->variabletab.end();
       ++i)
    for (std::list<urbi::UGenericCallback*>::iterator j =
	 i->second->internalBinder.begin();
	 j != i->second->internalBinder.end();
	)
      if ((*j)->objname == device->str())
      {
	delete *j;
	j = i->second->internalBinder.erase(j);
      }
      else
	++j;

  // clean variables internalAccessBinder
  for (HMvariabletab::iterator i = ::urbiserver->variabletab.begin();
       i != ::urbiserver->variabletab.end();
       ++i)
    for (std::list<urbi::UGenericCallback*>::iterator j =
	 i->second->internalAccessBinder.begin();
	 j != i->second->internalAccessBinder.end();
	)
      if ((*j)->objname == device->str())
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
UObj::searchFunction(const char* id, bool &ambiguous)
{
  UFunction* ret;
  char namebuffer[1024];
  UString::makeName(namebuffer, device, id);

  // test for pure urbi symbols
  HMfunctiontab::iterator hmf = ::urbiserver->functiontab.find(namebuffer);
  if (hmf != ::urbiserver->functiontab.end())
  {
    ambiguous = false;
    return hmf->second;
  }

  // test for remote uobjects symbols
  if (::urbiserver->functionbindertab.find(namebuffer)
      != ::urbiserver->functionbindertab.end())
  {
    ambiguous = false;
    return kernel::remoteFunction;
  }

  // test for plugged uobjects symbols
  if (::urbi::functionmap.find(namebuffer)
      != ::urbi::functionmap.end())
  {
    ambiguous = false;
    return kernel::remoteFunction;
  }

  // try recursively with parents
  ret = 0;
  bool found = false;
  for (std::list<UObj*>::iterator i = up.begin();
       i != up.end();
       ++i)
  {
    UFunction* tmpres = (*i)->searchFunction(id, ambiguous);
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

UVariable*
UObj::searchVariable(const char* id, bool &ambiguous)
{
  UVariable* ret;
  char namebuffer[1024];
  UString::makeName(namebuffer, device, id);

  HMvariabletab::iterator hmv = ::urbiserver->variabletab.find(namebuffer);
  if (hmv != ::urbiserver->variabletab.end())
  {
    ambiguous = false;
    return hmv->second;
  }
  else
  {
    ret   = 0;
    bool found = false;
    for (std::list<UObj*>::iterator i = up.begin();
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
UObj::searchEvent(const char* id, bool &ambiguous)
{
  char namebuffer[1024];
  UString::makeName(namebuffer, device, id);

  if (kernel::eventSymbolDefined (namebuffer))
  {
    ambiguous = false;
    return true;
  }

  else
  {
    bool found = false;
    for (std::list<UObj*>::iterator i = up.begin();
	 i != up.end();
	 ++i)
    {
       bool tmpres = (*i)->searchEvent(id, ambiguous);
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
	  found = true;
	}
    }
    ambiguous = false;
    return found;
  }
}

/*********************************************/
/* UWaitCounter                              */
/*********************************************/

UWaitCounter::UWaitCounter(UString *id, int nb)
{
  this->id = new UString(id);
  this->nb = nb;
}

UWaitCounter::~UWaitCounter()
{
  delete id;
}
