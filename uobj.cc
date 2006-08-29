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
#include <stdio.h>
#ifdef _MSC_VER
#define snprintf _snprintf
#endif
#include "uobj.h"
#include "ustring.h"
#include "uvalue.h"
#include "uvariablename.h"
#include "userver.h"
#include "uconnection.h"
#include "uobject.h"
  
char namebuffer[1024];
  
// **************************************************************************	
//! UObj constructor.
UObj::UObj (UString *device)
{
  this->device = new UString(device);
  binder = 0;
  internalBinder = 0; 

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
  for (HMvariabletab::iterator it = ::urbiserver->variabletab.begin();
      it != ::urbiserver->variabletab.end();
      it++) 
    if (((*it).second->method) &&
	((*it).second->devicename) && (device) &&
	((*it).second->value->dataType != DATA_OBJ) &&
        ((*it).second->devicename->equal(device)) ) {
	  
      delete (*it).second;
/*
      if ((*it).second->binder) delete (*it).second->binder;

      for (list<urbi::UGenericCallback*>::iterator itcb = internalBinder.begin();
	  itcb != internalBinder.end();
	  itcb++) 
	delete (*itcb);
      
      for (list<urbi::UGenericCallback*>::iterator itcb = internalAccessBinder.begin();
	  itcb != internalAccessBinder.end();
	  itcb++) 
	delete (*itcb);
*/
    }

  
  if (binder) { 
    char messagetosend[1024];
    snprintf(messagetosend,1024,"[5,\"%s\"]\n", device->str());

    for (list<UConnection*>::iterator it = binder->monitors.begin();
	it != binder->monitors.end();
	it++)
      (*it)->send((const ubyte*)messagetosend, strlen(messagetosend));
    delete(binder);
  }

  if (internalBinder) delete internalBinder;    
  if (device) delete(device);
}

UFunction*
UObj::searchFunction(const char* id, bool &ambiguous)
{
  UFunction *ret;
  bool found;
  
  snprintf(namebuffer,1024,"%s.%s",device->str(),id);  
  HMfunctiontab::iterator hmf = ::urbiserver->functiontab.find(namebuffer);
  if (hmf != ::urbiserver->functiontab.end()) {
    ambiguous = false;
    return (hmf->second);
  }
  else {
    ret   = 0;
    found = false;
    for (list<UObj*>::iterator itup = up.begin();
	itup != up.end();
	itup++){
      ret = (*itup)->searchFunction(id,ambiguous);
      if (ambiguous) return 0;
      if (ret)
	if (found) {
	  ambiguous = true;
	  return 0;
	}
	else 
	  found = true;      
    }
    ambiguous = false;
    return ret;
  }
} 

UVariable*
UObj::searchVariable(const char* id, bool &ambiguous)
{
  UVariable *ret;
  bool found;
  
  snprintf(namebuffer,1024,"%s.%s",device->str(),id);  
  HMvariabletab::iterator hmv = ::urbiserver->variabletab.find(namebuffer);
  if (hmv != ::urbiserver->variabletab.end()) {
    ambiguous = false;
    return (hmv->second);
  }
  else {
    ret   = 0;
    found = false;
    for (list<UObj*>::iterator itup = up.begin();
	itup != up.end();
	itup++){
      ret = (*itup)->searchVariable(id,ambiguous);
      if (ambiguous) return 0;
      if (ret)
	if (found) {
	  ambiguous = true;
	  return 0;
	}
	else 
	  found = true;      
    }
    ambiguous = false;
    return ret;
  }
}



