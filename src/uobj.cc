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
#include <cstdio>
#ifdef _MSC_VER
# define snprintf _snprintf
#endif

#include "uobj.hh"
#include "ustring.hh"
#include "uvalue.hh"
#include "uvariablename.hh"
#include "userver.hh"
#include "uconnection.hh"
#include "urbi/uobject.hh"
#include "ueventhandler.hh"

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
  {
    if (it->second->method
	&& it->second->devicename
	&& device
	&& it->second->value->dataType != DATA_OBJ
	&& it->second->devicename->equal(device)
       )
      varToDelete.push_back(it->second);
  }//for

  for (std::list<UVariable*>::iterator itd = varToDelete.begin();
       itd != varToDelete.end();
       ++itd)
    delete (*itd);


  // clean variables binders (a bit brutal, we scan all wariables...
  // I'll work on an optimized version later)
  for (HMvariabletab::iterator it = ::urbiserver->variabletab.begin();
       it != ::urbiserver->variabletab.end();
       it++)
  {
    if (it->second->binder)
    {
      bool isempty = it->second->binder->removeMonitor(device);
      if (isempty)
      {
	delete it->second->binder;
	it->second->binder = 0;
      }//if
    }//if
  }//for

  std::list<HMbindertab::iterator> deletelist;
  //clean functions binders
  for (HMbindertab::iterator it2 = ::urbiserver->functionbindertab.begin();
       it2 != ::urbiserver->functionbindertab.end();
       it2++)
  {
    if (it2->second->removeMonitor(device))
      deletelist.push_back(it2);
  }//for
  for (std::list<HMbindertab::iterator>::iterator itt = deletelist.begin();
       itt != deletelist.end();
       itt++)
    ::urbiserver->functionbindertab.erase((*itt));
  deletelist.clear();


  //clean events binders
  for (HMbindertab::iterator it3 = ::urbiserver->eventbindertab.begin();
       it3 != ::urbiserver->eventbindertab.end();
       it3++)
  {
    if (it3->second->removeMonitor(device))
      deletelist.push_back(it3);
  }//for
  for (std::list<HMbindertab::iterator>::iterator itt = deletelist.begin();
       itt != deletelist.end();
       itt++)
    ::urbiserver->eventbindertab.erase((*itt));
  deletelist.clear();



  // clean the object binder
  if (binder)
  {
    char messagetosend[1024];
    snprintf(messagetosend, 1024, "[5,\"%s\"]\n", device->str());

    for (std::list<UMonitor*>::iterator it = binder->monitors.begin();
	 it != binder->monitors.end();
	 it++)
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
  ASSERT (idit != ::urbiserver->objtab.end()) ::urbiserver->objtab.erase(idit);

  // Remove the objects from the subclass list of its parents
  for (std::list<UObj*>::iterator it = up.begin();
       it != up.end();
       ++it)
  {
    (*it)->down.remove(this);
  }

  // INTERNAL cleanups
  if ((internalBinder) &&
      (internalBinder->getUObject()))
  {
    // here we have two different cases because base classes are not
    // dynamically created and cannot be deleted, they can only
    // to be "cleaned":
    if (internalBinder->getUObject()->derived)
      delete internalBinder; // this deletes the associated UObject
    else if (internalBinder->getUObject())
      delete internalBinder->getUObject();
  }

  // clean variables internalBinder
  for (HMvariabletab::iterator it = ::urbiserver->variabletab.begin();
       it != ::urbiserver->variabletab.end();
       it++)
  {
    for (std::list<urbi::UGenericCallback*>::iterator itcb =
	 it->second->internalBinder.begin();
	 itcb != it->second->internalBinder.end();
	)
    {
      if ((*itcb)->objname == device->str())
      {
	delete *itcb;
	itcb = it->second->internalBinder.erase(itcb);
      }
      else
	++itcb;
    }//for
  }//for

  // clean variables internalAccessBinder
  for (HMvariabletab::iterator it = ::urbiserver->variabletab.begin();
       it != ::urbiserver->variabletab.end();
       it++)
  {
    for (std::list<urbi::UGenericCallback*>::iterator itcb =
	 it->second->internalAccessBinder.begin();
	 itcb != it->second->internalAccessBinder.end();
	)
    {
      if ((*itcb)->objname == device->str())
      {
	delete (*itcb);
	itcb = it->second->internalAccessBinder.erase(itcb);
      }
      else
	++itcb;
    }
  }

  // final cleanup
  delete device;
}

UFunction*
UObj::searchFunction(const char* id, bool &ambiguous)
{
  UFunction* ret;
  char namebuffer[1024];
  snprintf(namebuffer, 1024, "%s.%s", device->str(), id);

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
  ret   = 0;
  bool found = false;
  for (std::list<UObj*>::iterator itup = up.begin();
       itup != up.end();
       itup++)
  {
    UFunction* tmpres = (*itup)->searchFunction(id, ambiguous);
    if (ambiguous) return 0;
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

  snprintf(namebuffer, 1024, "%s.%s", device->str(), id);
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
    for (std::list<UObj*>::iterator itup = up.begin();
	 itup != up.end();
	 itup++)
    {
      UVariable* tmpres = (*itup)->searchVariable(id, ambiguous);
      if (ambiguous) return 0;
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

UEventHandler*
UObj::searchEvent(const char* id, bool &ambiguous)
{
  UEventHandler* ret;
  char namebuffer[1024];

  snprintf(namebuffer, 1024, "%s.%s", device->str(), id);
  bool ok = false;
  HMemittab::iterator iet;
  HMemittab::iterator ietok = ::urbiserver->emittab.end ();

  for (iet = ::urbiserver->emittab.begin ();
       iet != ::urbiserver->emittab.end () && !ok;
       ++iet)
    if (iet->second->unforgedName->equal (namebuffer))
    {
      ok = true;
      ietok = iet;
    }

  if (ok)
  {
    ambiguous = false;
    return ietok->second;
  }
  else
  {
    ret   = 0;
    bool found = false;
    for (std::list<UObj*>::iterator itup = up.begin();
	 itup != up.end();
	 itup++)
    {
       UEventHandler* tmpres = (*itup)->searchEvent(id, ambiguous);
      if (ambiguous) return 0;
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
