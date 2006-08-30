/*! \file uvar.cc
 *******************************************************************************

 File: uvar.cc\n
 Implementation of the UVar class.

 This file is part of LIBURBI\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include "uobject.h"
#include "uexternal.h"
using namespace urbi;

#include <usyncclient.h>

namespace 
urbi {
  class UVardata {
    public:	
    UVardata() {};
    ~UVardata() {};
  };
};
	
static const char * propNames[]={
	"rangemin",
	"rangemax",
	"speedmin",
	"speedmax",
	"blend",
	"delta"
};
// **************************************************************************	
//! UVar constructor: implicit object ref (using 'lastUOjbect') + varname
UVar::UVar(const string &varname)
:VAR_PROP_INIT
{
  name = varname;  
  __init();
}

//! UVar constructor: object reference + var name
UVar::UVar(UObject& obj, const string &varname) 
:VAR_PROP_INIT
{
  name = obj.__name + "." + varname;
  __init();
}

//! UVar constructor: object name + var name
UVar::UVar(const string &objname, const string &varname)
:VAR_PROP_INIT
{
  name = objname + "." + varname;
  __init();
}


//! UVar initialization
void
UVar::init(const string &objname, const string &varname)
{  
  name = objname + "." + varname;  
  __init();
}

//! UVar initialization
void
UVar::__init()
{  
  varmap[name].push_back(this);
  vardata = 0; // unused. For internal softdevices only
  this->owned = false;
}

//! UVar out value (read mode)
ufloat&
UVar::out()
{  
  return (value.val); 
}

//! UVar in value (write mode)
ufloat&
UVar::in()
{  
  return (value.val); 
}


void 
UVar::setProp(UProperty prop, const UValue &v) {
	URBI()<<name<<"->"<<propNames[(int)prop]<<"="<<v<<";";
}

void 
UVar::setProp(UProperty prop, const char * v) {
	URBI()<<name<<"->"<<propNames[(int)prop]<<"="<<v<<";";
}
void 
UVar::setProp(UProperty prop, double v) {
	//TODO : generalize
	if (prop == PROP_BLEND && v>=0 && v< blendNum)
		URBI()<<name<<"->"<<propNames[(int)prop]<<"="<<blendNames[(int)v]<<";";
	else
		URBI()<<name<<"->"<<propNames[(int)prop]<<"="<<v<<";";
}

UValue
UVar::getProp(UProperty prop) {
	UMessage *m=((USyncClient&)URBI()).syncGet("%s->%s",name.c_str(), propNames[(int)prop]);
	UValue v = *(m->value);
	delete m;
	return v;
}

/*
UBlendType
UVar::blend()
{
  urbi::echo("Properties not implemented in remote mode yet.\n");
  return (UNORMAL);
}
*/

//! UVar destructor.
UVar::~UVar()
{  
  UVarTable::iterator varmapfind = varmap.find(name);
  
  if (varmapfind != varmap.end()) {
    
    for (list<UVar*>::iterator it = varmapfind->second.begin();
	it != varmapfind->second.end();)
      if ((*it) == this) 
	it=varmapfind->second.erase(it);
      else
	it++;
	
    if (varmapfind->second.empty())
      varmap.erase(varmapfind);
  }
}

//! UVar float assignment
void
UVar::operator = (ufloat n)
{  
  URBI() << name << "=" << n << ";";  
}

//! UVar string assignment
void
UVar::operator = (string s)
{  
  URBI() << name << "=\"" << s << "\";";  
}

//! UVar binary assignment
void
UVar::operator = (const UBinary &b)
{  
  URBI() << name << "=";  
  URBI()<<"BIN "<<b.common.size<<" "<<b.getMessage()<<";";
  URBI().write((char *)b.common.data, b.common.size);
}

void
UVar::operator = (const UImage &i) 
{
	//we don't use UBinary Image ctor because it copies data
	UBinary b;
	b.type = BINARY_IMAGE;
	b.image = i;
	(*this)=b;
	b.common.data=0; //required, dtor frees data
}

void
UVar::operator = (const USound &i) 
{
	//we don't use UBinary Image ctor because it copies data
	UBinary b;
	b.type = BINARY_SOUND;
	b.sound = i;
	(*this)=b;
	b.common.data=0; //required, dtor frees data
}

UVar::operator int () {	 
  return ((int)value); 
};

UVar::operator ufloat () { 
  return ((ufloat)value); 
};


UVar::operator string () { 
  return ((string)value); 
};
 

UVar::operator UBinary() {
  return value;
};

UVar::operator UBinary*() {
  return new UBinary(value.operator UBinary());
};

UVar::operator UImage() {
  return (UImage)value;
};

UVar::operator USound() {
  return (USound)value;
};


//! UVar update
void
UVar::__update(UValue& v)
{  
  cout << "  Variable " << name << " updated to : ";
  if (v.type == DATA_DOUBLE)
    cout << (double)v << endl;
  if (v.type == DATA_STRING)  
    cout << (string)v << endl;
      
  value = v;
}

//! set own mode
void
UVar::setOwned()
{
  owned = true;
}

void
UVar::requestValue() {
  //build a getvalue message  that will be parsed and returned by the server
  URBI() << externalModuleTag<<':'<<'[' << UEM_ASSIGNVALUE << ","  <<'"'<<name<<'"'<<','<<name<<"];";
}
