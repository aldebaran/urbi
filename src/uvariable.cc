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

#include <string>
#include <cstdlib>

#include "libport/cstdio"
#include "libport/ref-pt.hh"

#include "uasyncregister.hh"
#include "ubinary.hh"
#include "ubinder.hh"
#include "ucommand.hh"
#include "uconnection.hh"
#include "urbi/uobject.hh"
#include "userver.hh"
#include "utypes.hh"
#include "uvalue.hh"
#include "uvariable.hh"
#include "uobj.hh"
#include "ucallid.hh"

MEMORY_MANAGER_INIT(UVariable);

//! UVariable constructor.
UVariable::UVariable(const char* name, UValue* _value,
		     bool _notifyWrite,
		     bool _notifyRead,
		     bool _autoUpdate):
  UASyncRegister(),
  context(0)
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
		     bool _autoUpdate):
  UASyncRegister(),
  context(0)
{
  init();
  value = _value;
  notifyRead   = _notifyRead;
  notifyWrite = _notifyWrite;
  autoUpdate = _autoUpdate;
  ::urbiserver->variabletab[setName(_id, _method)] = this;
}

//! UVariable constructor.
UVariable::UVariable(const char* name,
		     ufloat val,
		     bool _notifyWrite,
		     bool _notifyRead,
		     bool _autoUpdate):
  UASyncRegister(),
  context(0)
{
  init();
  value = new UValue(val);
  notifyRead   = _notifyRead;
  notifyWrite = _notifyWrite;
  autoUpdate = _autoUpdate;
  ::urbiserver->variabletab[setName(name)] = this;
}

//! UVariable constructor.
UVariable::UVariable(const char* _id, const char* _method, ufloat val,
		     bool _notifyWrite,
		     bool _notifyRead,
		     bool _autoUpdate):
  UASyncRegister(),
  context(0)
{
  init();
  value = new UValue(val);
  notifyRead   = _notifyRead;
  notifyWrite = _notifyWrite;
  autoUpdate = _autoUpdate;
  ::urbiserver->variabletab[setName(_id, _method)] = this;
}

//! UVariable constructor.
UVariable::UVariable(const char* name, const char* str,
		     bool _notifyWrite,
		     bool _notifyRead,
		     bool _autoUpdate):
  UASyncRegister(),
  context(0)
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
		     bool _autoUpdate):
  UASyncRegister(),
  context(0)
{
  init();
  value = new UValue(str);
  notifyRead   = _notifyRead;
  notifyWrite = _notifyWrite;
  autoUpdate = _autoUpdate;
  ::urbiserver->variabletab[setName(_id, _method)] = this;
}

//! general initialisation called by constructors
void
UVariable::init()
{
  ADDOBJ(UVariable);
  blendType  = urbi::UNORMAL;
  rangemin   = -UINFINITY;
  rangemax   = UINFINITY;
  speedmin   = 0;
  speedmax   = UINFINITY;
  delta      = 0; // delta is 0 by default
  speedmodified = false;
  reloop        = false;
  nbAssigns  = 0;
  nbAverage  = 0;
  target     = 0;
  previous   = 0;
  previous2  = 0;
  previous3  = 0;
  cancel     = 0;
  uservar    = ::urbiserver->uservarState;
  toDelete   = false;
  activity   = 0;
  binder     = 0;
  access_and_change = false;
}

//! UVariable destructor
UVariable::~UVariable()
{
  if (activity)
    ::urbiserver->reinitList.remove(this);

  if (access_and_change)
    ::urbiserver->access_and_change_varlist.remove (this);

  HMvariabletab::iterator hmi;

  if ((hmi = ::urbiserver->variabletab.find(varname.c_str())) !=
      ::urbiserver->variabletab.end())
    ::urbiserver->variabletab.erase(hmi);

  FREEOBJ(UVariable);
  if (value)
  {
    if (value->dataType == DATA_OBJ && value->str!=0)
    {
      HMobjtab::iterator idit = ::urbiserver->objtab.find(value->str->str());
      if (idit != ::urbiserver->objtab.end())
	delete idit->second;
      // NB: idit is removed from objtab in the destructor of UObj
    }
    delete value;
  }
  if (context)
    context->remove(this);
}


//! Associated variable name initialization
const char*
UVariable::setName(const char* s)
{
  varname = s;

  size_t pos = varname.find('.');
  if (pos==std::string::npos) {
    method = "";
    devicename = varname;
  }
  else {
    method = varname.substr(pos + 1, varname.length());
    devicename = varname.substr(0,pos);
  }


  return varname.c_str();
}

//! Associated variable name initialization
const char*
UVariable::setName(const char *_id, const char* _method)
{
  char tmpVarName[1024];

  snprintf(tmpVarName, 1024, "%s.%s", _id, _method);

  varname = tmpVarName;
  method = _method;
  devicename = _id;

  return varname.c_str();
}

const char*
UVariable::setName(UString *s)
{
  return setName(s->str());
}

//! Set the UValue associated to the variable
/*! Note that the UValue v is going to be copied. You might
    delete it afterwards.

    if v is null, this mean that the value in the variable is uptodate
    but the function will still perform some range check and call
    UDevice::notifyWrite if necessary.
 */
UVariable::UVarSet
UVariable::set(UValue *v)
{
  if (!v)
    return selfSet(&(value->val));

  if (value  && value->dataType != v->dataType)
  {
    delete value;
    value = 0;
  }
  if (!value)
    value = v->copy();
  else
  {
    switch (value->dataType)
    {
      case DATA_STRING:
	value->str->update(v->str->str());
	break;
      case DATA_NUM:
	setSensorVal(v->val);
	break;
      case DATA_LIST:
	delete value;
	value = v->copy();
	break;
      case DATA_BINARY:
	LIBERATE(value->refBinary);
	value->refBinary = v->refBinary->copy();
	break;
      case DATA_VOID:   // uninitialized def's
	value->dataType = v->dataType;
	switch (v->dataType)
	{
	  case DATA_STRING:
	    value->str = new UString(v->str);
	    break;
	  case DATA_NUM:
	    initSensorVal(v->val);
	    break;
	  case DATA_BINARY:
	    value->refBinary = v->refBinary->copy();
	    break;
	  case DATA_LIST:
	    delete value;
	    value = v->copy();
	    break;
	  case DATA_VOID:
	    //this can happen. do nothing
	    break;
	  case DATA_UNKNOWN:
	  case DATA_FILE:
	  case DATA_FUNCTION:
	  case DATA_OBJ:
	  case DATA_VARIABLE:
	    assert (!"not reachable");
	}
	break;
      case DATA_VARIABLE:
      case DATA_UNKNOWN:
      case DATA_FILE:
      case DATA_FUNCTION:
      case DATA_OBJ:
	assert (!"not reachable");
    }
  }

  return selfSet(&(value->val));
}

//! Set the UValue associated to the variable
/*! Here, the type is known to be a float, which saves the
    necessity to have a UValue passed as parameter
*/
UVariable::UVarSet
UVariable::setFloat(ufloat f)
{
  if (!value)
    value = new UValue(f);
  else
    setSensorVal(f);

  return selfSet(&value->val);
}

//! Check the float reference associated to the variable
/*! This function works like setFloat except that the value is
    assumed to be set already and the valcheck pointeur is
    used to perform range checking.

    valcheck can point either on value->val or on target.
*/
UVariable::UVarSet
UVariable::selfSet(ufloat *valcheck)
{
  ufloat s;
  ufloat localspeed;

  if (value->dataType == DATA_NUM)
  {
    if (*valcheck < rangemin)
      *valcheck = rangemin;
    if (*valcheck > rangemax)
      *valcheck = rangemax;

    if (speedmax != UINFINITY)
    {
      localspeed = (*valcheck - previous) / ::urbiserver->getFrequency();
      s = speedmax / 1000.;

      if (ABSF(localspeed) > s)
      {
	if (localspeed>0)
	  *valcheck = *valcheck - ((localspeed - s) * ::urbiserver->getFrequency());
	else
	  *valcheck = *valcheck - ((localspeed + s) * ::urbiserver->getFrequency());

	speedmodified = true;
      }
    }

    target = *valcheck; // for consistancy reasons
  }

  modified = true;
  updated();

  if (speedmodified)
    return USPEEDMAX;

  return UOK ;
}

//! Get the UValue associated to the variable
/*! If the variable is a device variable, a call to
    updateVariable is done for the device to update it before
    the UValue is actually sent back.
*/
UValue*
UVariable::get(bool autoloop)
{
  // recursive call for objects
  if (value->dataType == DATA_OBJ)
    for (HMvariabletab::iterator it = ::urbiserver->variabletab.begin();
	 it != ::urbiserver->variabletab.end();
	 ++it)
    if (!it->second->getMethod().empty()
	  && value->str
	  && it->second->value->dataType != DATA_OBJ
	  && it->second->getDevicename()== (std::string)value->str->str())
	it->second->get ();

  // data preparation for the UNotifyChange/Access loop control
  int nb_change = internalBinder.size ();
  std::string change_objname = "";
  if (nb_change == 1 && autoloop)
    change_objname = (*internalBinder.begin ())->objname;

  // check for existing notifychange
  for (std::list<urbi::UGenericCallback*>::iterator i =
	 internalAccessBinder.begin();
       i != internalAccessBinder.end();
       ++i)
  {
    // checking if there is a UNotifyChange/Access loop on a UOwned inside
    // the same object
    urbi::UVar* tmpvar = static_cast<urbi::UVar*> ((*i)->storage);

    if (!tmpvar->owned
	|| change_objname != (*i)->objname)
    {
      urbi::UList l;
      if ((*i)->storage)
      {
	// monitor with &UVar reference
	urbi::UValue *v = new urbi::UValue();
	v->storage = (*i)->storage;
	l.array.push_back(v);
      }
      (*i)->__evalcall(l);
    }
  }

  return value;
}

//! This function takes care of notifying the monitors that the var is updated
/*! When the variable is updated, either by the kernel of robot-specific
    part, this function must be called. It's called automatically by the above
    set methods.
*/
void
UVariable::updated(bool uvar_assign)
{
  // triggers associated commands update
  updateRegisteredCmd ();

  if (!binder && internalBinder.empty())
    return;

  if (binder)
    for (std::list<UMonitor*>::iterator i = binder->monitors.begin();
	 i != binder->monitors.end();
	 ++i)
      {
	(*i)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
	(*i)->c->sendc((const ubyte*)"[1,\"", 4);
	(*i)->c->sendc((const ubyte*)varname.c_str(), varname.length());
	(*i)->c->sendc((const ubyte*)"\",", 2);
	value->echo((*i)->c);
	(*i)->c->send((const ubyte*)"]\n", 2);
      }

  for (std::list<urbi::UGenericCallback*>::iterator i =
       internalBinder.begin();
       i != internalBinder.end();
       ++i)
  {
    if (!uvar_assign
	|| (*i)->objname != getDevicename())
    {
      urbi::UList tmparray;

      if ((*i)->storage)
      {
	// monitor with &UVar reference
	urbi::UValue *tmpvalue = new urbi::UValue();
	tmpvalue->storage = (*i)->storage;
	tmparray.array.push_back(tmpvalue);
      };

      (*i)->__evalcall(tmparray);
    }
  }
}

bool
UVariable::isDeletable()
{
  if (value &&
      value->dataType == DATA_OBJ &&
      value->str)
  {
    HMobjtab::iterator idit = ::urbiserver->objtab.find(value->str->str());
    if ((idit != ::urbiserver->objtab.end()) &&
	(!idit->second->down.empty()) )
      return false;
  }
  return true;
}

void
UVariable::setSensorVal(ufloat f)
{
  valPrev2 = valPrev;
  valPrev = value->val;
  value->val = f;
}

void
UVariable::initSensorVal(ufloat f)
{
  value->dataType = DATA_NUM;
  valPrev2 = f;
  valPrev = f;
  value->val = f;
}
