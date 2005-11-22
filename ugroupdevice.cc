/*! \file ugroupdevice.cc
 *******************************************************************************

 File: ugroupdevice.cc\n
 Implementation of the UGroupDevice class.

 This file is part of 
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 ************************************************************************** */
#include "ugroupdevice.h"
#include "userver.h"


UGroupDevice::UGroupDevice(const UString &name)
  : UDevice(name.str(), "group device", 0.0, 0.0, "na", DATA_LIST, false, true, true) {}

void        UGroupDevice::notifyWrite     ( const UVariable *variable) {
  //later
}

void        UGroupDevice::notifyRead      ( const UVariable *variable) {
  
  //do read
  UGroup * gr =  ::urbiserver->grouptab[device->str()];
  if (!gr) {
    fprintf(stderr, "FATAL no group for %s\n", device->str());
    return;
  }
  if (gr->members.empty()) {
    fprintf(stderr, "FATAL empty group for %s\n", device->str());
    UDevice * dev = ::urbiserver->devicetab[gr->device->str()];
    if (dev) {
      char vname[1024];
      strcpy(vname, gr->device->str());
      strcat(vname, ".");
      strcat(vname, variable->method->str()); 
      dev->notifyRead(::urbiserver->variabletab[vname]);
    }
  }
  
  
  else {
    UValue * val = new UValue();
    val->dataType = DATA_LIST;
    val->list = 0;
    UValue * current = val;
    for (list<UGroup*>::iterator it = gr->members.begin(); it != gr->members.end();it++) {
      char vname[1024];
      strcpy(vname, (*it)->device->str());
      strcat(vname, ".");
      strcat(vname, variable->method->str());
      if ( ::urbiserver->variabletab.find(vname) ==  ::urbiserver->variabletab.end()) {
	//no variable? could be...
	UVariable *var = new UVariable(vname,new UValue(), true, true, false);
	if (::urbiserver->devicetab.find((*it)->device->str()) != ::urbiserver->devicetab.end())
	  var->dev = ::urbiserver->devicetab[(*it)->device->str()];
	var->value->dataType = DATA_LIST; 
      }
      
      UValue * n = ::urbiserver->variabletab[vname]->get()->copy();
      current->list = n;
      while (current->list)
	current = current->list;
    }

    if (::urbiserver->variabletab[variable->varname->str()]==0)
      new UVariable(variable->varname->str(),0.0);
    ::urbiserver->variabletab[variable->varname->str()]->set(val);
    
    
  }
  
}


UValue*     UGroupDevice::evalFunction    ( UCommand *command,
                                        UConnection *connection,                                       
                                        const char *method,
					    UNamedParameters *parameters) {
  return 0; //new UValue(0.0);
}
