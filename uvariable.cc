/*! \file uvariable.cc
 *******************************************************************************

 File: uvariable.cc\n
 Implementation of the Uvariable class.

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

#include <string.h>
#include <stdlib.h>
#include "uvariable.h"
#include "userver.h"
#include "ucommand.h"
#include "udevice.h"
#include "utypes.h"
#include "uconnection.h"



//! UVariable constructor.
UVariable::UVariable(const char* name, UValue* _value, 
                     bool _notifyWrite,
                     bool _notifyRead,
                     bool _autoUpdate) 
{
  init();
  value = _value;
  notifyRead   = _notifyRead;
  notifyWrite = _notifyWrite;
  autoUpdate = _autoUpdate;
  ::urbiserver->variabletab[setName(name)] = this;
}

//! UVariable constructor.
UVariable::UVariable(const char* _id, const char* _method, UValue* _value, 
                     bool _notifyWrite,
                     bool _notifyRead,
                     bool _autoUpdate) 
{
  init();
  value = _value;
  notifyRead   = _notifyRead;
  notifyWrite = _notifyWrite;
  autoUpdate = _autoUpdate;
  ::urbiserver->variabletab[setName(_id,_method)] = this;
}

//! UVariable constructor.
UVariable::UVariable(const char* name,double val, 
                     bool _notifyWrite,
                     bool _notifyRead,
                     bool _autoUpdate) 
{
  init();
  value = new UValue(val);
  notifyRead   = _notifyRead;
  notifyWrite = _notifyWrite;
  autoUpdate = _autoUpdate;
  ::urbiserver->variabletab[setName(name)] = this;
}

//! UVariable constructor.
UVariable::UVariable(const char* _id, const char* _method, double val, 
                     bool _notifyWrite,
                     bool _notifyRead,
                     bool _autoUpdate) 
{
  init();
  value = new UValue(val);
  notifyRead   = _notifyRead;
  notifyWrite = _notifyWrite;
  autoUpdate = _autoUpdate;
  ::urbiserver->variabletab[setName(_id,_method)] = this;
}

//! UVariable constructor.
UVariable::UVariable(const char* name,const char* str, 
                     bool _notifyWrite,
                     bool _notifyRead,
                     bool _autoUpdate) 
{
  init();
  value = new UValue(str);
  notifyRead   = _notifyRead;
  notifyWrite = _notifyWrite;
  autoUpdate = _autoUpdate;
  ::urbiserver->variabletab[setName(name)] = this;
}

//! UVariable constructor.
UVariable::UVariable(const char* _id, const char* _method, const char *str, 
                     bool _notifyWrite,
                     bool _notifyRead,
                     bool _autoUpdate) 
{
  init();
  value = new UValue(str);
  notifyRead   = _notifyRead;
  notifyWrite = _notifyWrite;
  autoUpdate = _autoUpdate;
  ::urbiserver->variabletab[setName(_id,_method)] = this;
}

//! general initialisation called by constructors
void
UVariable::init() {
  
  ADDOBJ(UVariable);
  blendType  = UNORMAL;
  rangemin   = -UINFINITY;
  rangemax   = UINFINITY;
  speedmin   = 0;
  speedmax   = UINFINITY;
  unit       = 0;
  delta      = 0; // delta is 0 by default
  speedmodified = false;
  reloop        = false;

  dev        = 0;
  nbAssigns  = 0;
  nbAverage  = 0; 
  varname    = 0;
  method     = 0;
  devicename = 0;
  target     = 0;
  previous   = 0;
  previous2  = 0;
  previous3  = 0;
  cancel     = 0;
  isval      = false;
  uservar    = ::urbiserver->uservarState;
  toDelete   = false;
  activity   = 0;
  binder     = 0;
}

//! UVariable destructor
UVariable::~UVariable() {
    
  if (activity)
    ::urbiserver->reinitList.remove(this);

  HMvariabletab::iterator hmi;

  if ((hmi = ::urbiserver->variabletab.find(varname->str())) !=
      ::urbiserver->variabletab.end())    
    ::urbiserver->variabletab.erase(hmi);  

  FREEOBJ(UVariable); 
  if (value) delete value;
  if (unit)   delete unit;
  if (varname) delete varname;
  if (method) delete method;
  if (devicename) delete devicename;
}


//! Associated variable name initialization
const char*
UVariable::setName(const char *s)
{  
  char *pointPos;
  HMdevicetab::iterator hmi;
  
  varname    = new UString (s);

  pointPos = strstr(varname->str(),".");
  if (pointPos == 0)

    method = new UString("");
  else
    method = new UString(pointPos + 1);

  if (pointPos) pointPos[0] = 0;
  devicename = new UString(varname->str());
  if (pointPos) pointPos[0] = '.';
  
  if ((hmi = ::urbiserver->devicetab.find(devicename->str())) !=
      ::urbiserver->devicetab.end()) {
    dev = (*hmi).second;
    if (method->equal("val")) isval = true;
  }
  else dev = 0;

  return (varname->str());
}

//! Associated variable name initialization
const char*
UVariable::setName(const char *_id, const char* _method)
{
  char tmpVarName[1024];
  HMdevicetab::iterator hmi;
  
  snprintf(tmpVarName,1024,"%s.%s",_id,_method);

  varname    = new UString (tmpVarName);
  method     = new UString(_method); 
  devicename = new UString(_id);
 
  if ((hmi = ::urbiserver->devicetab.find(devicename->str())) !=
      ::urbiserver->devicetab.end()) {
    dev = (*hmi).second;    
    if (method->equal("val")) isval = true;  
  }
  else dev = 0;

  return (varname->str());
}

//! Set the UValue associated to the variable
/*! Note that the UValue v is going to be copied. You might
    delete it afterwards. 

    if v is null, this mean that the value in the variable is uptodate
    but the function will still perform some range check and call
    UDevice::notifyWrite if necessary.
 */
UVarSet
UVariable::set(UValue *v)
{  
  if (!value)
    value = v->copy();
  else {  
    switch (value->dataType) {
      case DATA_STRING: value->str->update(v->str->str()); break;
      case DATA_NUM:    setSensorVal(v->val); break;
      case DATA_LIST:   delete value;value = v->copy(); break;
      case DATA_BINARY: LIBERATE(value->refBinary);
                        value->refBinary = v->refBinary->copy();
                        break;
      case DATA_VOID:   // uninitialized def's 

        value->dataType = v->dataType;
        switch (v->dataType) {
	case DATA_STRING: value->str = new UString(v->str); break;
	case DATA_NUM:    initSensorVal(v->val); break;
	case DATA_BINARY: value->refBinary = v->refBinary->copy(); break;    
	case DATA_LIST: delete value;value = v->copy(); break;
        }
    }
  }
  
  return(selfSet(&(value->val)));
}

//! Set the UValue associated to the variable
/*! Here, the type is known to be a float, which saves the 
    necessity to have a UValue passed as parameter
*/
UVarSet
UVariable::setFloat(double f)
{  
  if (!value)
    value = new UValue(f);
  else 
    setSensorVal(f);

  return(selfSet(&(value->val)));
}

//! Check the float reference associated to the variable
/*! This function works like setFloat except that the value is
    assumed to be set already and the valcheck pointeur is
    used to perform range checking.

    valcheck can point either on value->val or on target.
*/
UVarSet
UVariable::selfSet(double *valcheck)
{    
  double s,localspeed;
  
  if (value->dataType == DATA_NUM) {    
    if (*valcheck < rangemin) *valcheck = rangemin;
    if (*valcheck > rangemax) *valcheck = rangemax;
   
    if (speedmax != UINFINITY) {
      localspeed = (*valcheck - previous) / ::urbiserver->getFrequency();    
      s = speedmax / 1000.;
      
      if (ABSF(localspeed) > s) {
        if (localspeed>0)
          *valcheck = *valcheck - ((localspeed - s) * ::urbiserver->getFrequency());
        else 
          *valcheck = *valcheck - ((localspeed + s) * ::urbiserver->getFrequency());
        
        speedmodified = true;
      }
    }    
    
    target   = *valcheck; // for consistancy reasons
  }

  modified = true;
  if ((notifyWrite) && (dev)) dev->notifyWrite(this); 
  updated();
  
  if (speedmodified) return (USPEEDMAX);

  return ( UOK );
}

//! Get the UValue associated to the variable
/*! If the variable is a device variable, a call to 
    updateVariable is done for the device to update it before
    the UValue is actually sent back.
*/
UValue*
UVariable::get()
{ 
  //are we a group?
  if ((notifyRead) && (dev)) dev->notifyRead(this);
  return value; 
};

//! This function takes care of notifying the monitors that the var is updated
/*! When the variable is updated, either by the kernel of robot-specific
    part, this function must be called. It's called automatically by the above
    set methods.
*/
inline void
UVariable::updated()
{
  if (!binder) return;  

  for (list<UConnection*>::iterator it = binder->monitors.begin();
      it != binder->monitors.end();
      it++) {

    (*it)->sendPrefix(EXTERNAL_MESSAGE_TAG);
    (*it)->send((const ubyte*)"[1,\"",4);
    (*it)->send((const ubyte*)varname->str(),varname->len());
    (*it)->send((const ubyte*)"\",",2);
    value->echo((*it));
    (*it)->send((const ubyte*)"]\n",2);    
  }
  
  /*
  //debug
  switch (value->dataType) {
    case DATA_STRING: 
     ::urbiserver->debug("UPDATED: %s = \"%s\"\n",varname->str(),value->str->str());     
     break;
    case DATA_NUM:  
     ::urbiserver->debug("UPDATED: %s = %f\n",varname->str(),value->val);
     break;
    default:
     ::urbiserver->debug("UPDATED: %s (unknown type)\n",varname->str());
  }
  */
}

