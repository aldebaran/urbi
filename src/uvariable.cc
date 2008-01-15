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

#include <boost/foreach.hpp>

#include "libport/assert.hh"
#include "libport/cstdio"
#include "libport/ref-pt.hh"

#include "urbi/uobject.hh"

#include "kernel/uconnection.hh"
#include "kernel/userver.hh"
#include "kernel/utypes.hh"
#include "kernel/uvalue.hh"
#include "kernel/uvariable.hh"
#include "kernel/uasyncregister.hh"

#include "ubinary.hh"
#include "ubinder.hh"
#include "ucommand.hh"
#include "uobj.hh"
#include "ucallid.hh"

MEMORY_MANAGER_INIT(UVariable);

//! UVariable constructor.
UVariable::UVariable(const char* name, UValue* _value,
		     bool _notifyWrite,
		     bool _notifyRead,
		     bool _autoUpdate):
  UASyncRegister(),
  context(0),
  inSetTarget(false)
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
  : UASyncRegister(),
    context(0),
    inSetTarget(false)
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
  context(0),
  inSetTarget(false)
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
  context(0),
  inSetTarget(false)
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
  context(0),
  inSetTarget(false)
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
  context(0),
  inSetTarget(false)
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
  assert (::urbiserver);
  uservar    = ::urbiserver->uservarState;
  toDelete   = false;
  activity   = 0;
  binder     = 0;
  access_and_change = false;
  useCpt     = 0;

  cycleBeginTime = -1;
}

//! UVariable destructor
UVariable::~UVariable()
{
  //  std::cerr << "Deleting " << this << std::endl;
  if (activity)
    ::urbiserver->reinitList.remove(this);

  if (access_and_change)
    ::urbiserver->access_and_change_varlist.remove (this);

  {
    HMvariabletab::iterator i = ::urbiserver->variabletab.find(varname.c_str());
    if (i != ::urbiserver->variabletab.end())
      ::urbiserver->variabletab.erase(i);
  }

  // Disactivate corresponding UVars
  {
    urbi::UVarTable::iterator i = urbi::varmap->find(varname.c_str ());
    if (i != urbi::varmap->end())
    {
      BOOST_FOREACH (urbi::UVar* j, i->second)
	j->setZombie ();
    }
  }

  FREEOBJ(UVariable);
  if (value)
  {
    if (value->dataType == DATA_OBJ && value->str)
    {
      HMobjtab::iterator idit = ::urbiserver->objtab.find(value->str->c_str());
      if (idit != ::urbiserver->objtab.end())
	delete idit->second;
      // NB: idit is removed from objtab in the destructor of UObj
    }
    delete value;
  }

  if (context)
    context->remove(this);
}

std::ostream&
UVariable::dump(std::ostream& o) const
{
#define UV_PRINT(Id) " " #Id " = " << Id
  return
    o << "Uvariable {"
      << UV_PRINT(devicename) << ','
      << UV_PRINT(method) << ','
      << UV_PRINT(varname) << ','
      << UV_PRINT(unit)
      << " }";
#undef UV_PRINT
}

std::ostream&
UVariable::print(std::ostream& o) const
{
  o << getVarname() << " = ";
  switch (value->dataType)
  {
    case DATA_NUM:
      o << value->val;
      break;

    case DATA_STRING:
      o << value->echo ();
      break;

    case DATA_BINARY:
      o << "BIN ";
      if (value->refBinary)
	o << value->refBinary->ref()->bufferSize;
      else
	o << "0 null";
      break;

    case DATA_LIST:
      o << "LIST";
      break;

    case DATA_OBJ:
      o << "OBJ";
      break;

    case DATA_VOID:
      o << "VOID";
      break;

    default:
      o << "UNKNOWN TYPE";
  }
  return o;
}

//! Associated variable name initialization
const char*
UVariable::setName(const char* s)
{
  varname = s;

  size_t pos = varname.find('.');
  if (pos==std::string::npos)
  {
    method = "";
    devicename = varname;
  }
  else
  {
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
  return setName(s->c_str());
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
	*value->str = v->str->c_str();
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
	    value->str = new UString(*v->str);
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
	    passert (v->dataType, !"not reachable");
	}
	break;
      case DATA_VARIABLE:
      case DATA_UNKNOWN:
      case DATA_FILE:
      case DATA_FUNCTION:
      case DATA_OBJ:
	passert (value->dataType, !"not reachable");
    }
  }
  UVariable::UVarSet r = UOK;
  if (value->dataType == DATA_NUM)
    r = selfSet(&(value->val));
  setTarget();
  return r;
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
  /* stupid, target update, not sensor update
  else
    setSensorVal(f);
  */
  return selfSet(&f);
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
      localspeed = (*valcheck - previous) / ::urbiserver->period_get();
      s = speedmax / 1000.;

      if (ABSF(localspeed) > s)
      {
	if (localspeed>0)
	  *valcheck -= (localspeed - s) * ::urbiserver->period_get();
	else
	  *valcheck -= (localspeed + s) * ::urbiserver->period_get();

	speedmodified = true;
      }
    }

    target = *valcheck; // for consistancy reasons <- LIE! this is crucial
  }

  modified = true;
  //selfset is called to update target which must not trigger updated updated();

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
UVariable::get()
{
  inSetTarget = true;

  // recursive call for objects
  if (value->dataType == DATA_OBJ)
    for (HMvariabletab::iterator it = ::urbiserver->variabletab.begin();
	 it != ::urbiserver->variabletab.end();
	 ++it)
    if (!it->second->getMethod().empty()
	  && value->str
	  && it->second->value->dataType != DATA_OBJ
	  && it->second->getDevicename()== (std::string)value->str->c_str())
	it->second->get();

  // check for existing notifychange
  BOOST_FOREACH (urbi::UGenericCallback* i, internalAccessBinder)
  {
    urbi::UList l;
    if (i->storage)
    {
      // monitor with &UVar reference
      urbi::UValue *v = new urbi::UValue();
      v->storage = i->storage;
      l.array.push_back(v);
    }
    i->__evalcall(l);
  }

  inSetTarget = false;
  return value;
}

//! This function takes care of notifying the monitors that the var is updated
/*! When the variable is updated, either by the kernel of robot-specific
    part, this function must be called.
    @param uvar_assign: unused
*/
void
UVariable::updated(bool )
{
  inSetTarget = true;
  // triggers associated commands update
  updateRegisteredCmd ();

  if (binder)
  {
    BOOST_FOREACH (UMonitor* i, binder->monitors)
    {
      *(i->c) << UConnection::prefix(EXTERNAL_MESSAGE_TAG);
      *(i->c) << UConnection::sendc((const ubyte*)"[1,\"", 4);
      *(i->c) << UConnection::sendc((const ubyte*)varname.c_str(), varname.length());
      *(i->c) << UConnection::sendc((const ubyte*)"\",", 2);
      value->echo(i->c);
      *(i->c) << UConnection::send((const ubyte*)"]\n", 2);
    }
  }

  BOOST_FOREACH (urbi::UGenericCallback* i, internalBinder)
  {
    // handled better otherwise
//     if (!uvar_assign
//         || i->objname != devicename->str ())
    {
      urbi::UList tmparray;

      if (i->storage)
      {
	// monitor with &UVar reference
	urbi::UValue *tmpvalue = new urbi::UValue();
	tmpvalue->storage = i->storage;
	tmparray.array.push_back(tmpvalue);
      };

      i->__evalcall(tmparray);
    }
  }

  inSetTarget = false;
}

bool
UVariable::isDeletable()
{
  if (value &&
      value->dataType == DATA_OBJ &&
      value->str)
  {
    HMobjtab::iterator idit = ::urbiserver->objtab.find(value->str->c_str());
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

/// Called when the target value has been updated. Call all callbacks.
void UVariable::setTarget() {
  std::list<urbi::UGenericCallback*>* callbacks = 0;
  inSetTarget = true; //set to false when exiting function
  if (this->autoUpdate)
    callbacks = &internalBinder;
  else
    callbacks = &internalTargetBinder;

  BOOST_FOREACH (urbi::UGenericCallback* i, *callbacks)
  {
    urbi::UList tmparray;

    if (i->storage)
    {
      // monitor with &UVar reference
      urbi::UValue *tmpvalue = new urbi::UValue();
      tmpvalue->storage = i->storage;
      tmparray.array.push_back(tmpvalue);
    };

    i->__evalcall(tmparray);
  }
  if (autoUpdate)
  {
     if (binder)
     {
       BOOST_FOREACH (UMonitor* i, binder->monitors)
	 {
	   *(i->c) << UConnection::prefix(EXTERNAL_MESSAGE_TAG);
	   *(i->c) << UConnection::sendc((const ubyte*)"[1,\"", 4);
	   *(i->c) << UConnection::sendc((const ubyte*)varname.c_str(),
					 varname.length());
	   *(i->c) << UConnection::sendc((const ubyte*)"\",", 2);
	   value->echo(i->c);
	   *(i->c) << UConnection::send((const ubyte*)"]\n", 2);
	 }
     }
  }
  inSetTarget = false;
}
