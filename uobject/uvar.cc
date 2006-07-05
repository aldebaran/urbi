/*! \file uvar.cc
 *******************************************************************************

 File: uvar.cc\n
 Implementation of the UVar class.

 This file is part of 
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include "uvariable.h"
#include "userver.h"
#include "uobject.h"
using namespace urbi;

namespace urbi {

  class UVardata {
    public:
      UVardata(UVariable *v) { variable = v; };
      ~UVardata() {};

      UVariable *variable;
  };
};
	
// **************************************************************************	
//! UVar constructor: implicit object ref (using 'lastUOjbect') + varname
UVar::UVar(const string &varname) 
{
  name = varname;  
  __init();
}

//! UVar constructor: object reference + var name
UVar::UVar(UObject& obj, const string &varname)
{
  name = obj.__name + "." + varname;
  __init();
}

//! UVar constructor: object name + var name
UVar::UVar(const string &objname, const string &varname)
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

//! UVar initializationvoid
void
UVar::__init()
{  
  this->owned = false;
  varmap[name].push_back(this);
  
  HMvariabletab::iterator it = ::urbiserver->variabletab.find(name.c_str());
  if (it == ::urbiserver->variabletab.end()) 
    vardata = new UVardata(new UVariable(name.c_str(),new
    ::UValue(),false,false,true));  // autoupdate unless otherwise specified
  else {
    vardata = new UVardata(it->second); 
    //XXX why?? owned = !vardata->variable->autoUpdate;
  }  
}

//! set own mode
void
UVar::setOwned()
{
  owned = true;
  if (vardata) 
    vardata->variable->autoUpdate = false;  
}

//! UVar destructor.
UVar::~UVar()
{  
  UVarTable::iterator varmapfind = varmap.find(name);
  
  if (varmapfind != varmap.end()) {
    
    for (list<UVar*>::iterator it = varmapfind->second.begin();
	it != varmapfind->second.end();)
      if ((*it) == this) 
	varmapfind->second.erase(it);
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
  if (!vardata) { 
    urbi::echo("Unable to locate variable %s in hashtable. Memory problem, report bug.\n",
	name.c_str());
    return;
  }
  
  // type mismatch is not integrated at this stage
  vardata->variable->value->dataType = ::DATA_NUM;

  if (owned)
    in() = n;
  else
    vardata->variable->setFloat(n);
}

//! UVar string assignment
void
UVar::operator = (string s)
{  
  if (!vardata) {
    urbi::echo("Unable to locate variable %s in hashtable. Memory problem, report bug.\n",
	name.c_str());
    return;      
  }

  if (vardata->variable->value->dataType == ::DATA_VOID) 
    vardata->variable->value->str = new UString("");
   
  // type mismatch is not integrated at this stage
  vardata->variable->value->dataType = ::DATA_STRING;
  ::UValue tmpv(s.c_str());
  vardata->variable->set(&tmpv);
}

//! UVar binary assignment
void
UVar::operator = (const UBinary &b)
{  
  if (!vardata) {
    urbi::echo("Unable to locate variable %s in hashtable. Memory problem, report bug.\n",
	name.c_str());
    return;      
  }
  *vardata->variable->value=b;
  vardata->variable->updated();
}

//! UVar binary assignment
void
UVar::operator = (const UImage &b)
{
  if (!vardata) {
    urbi::echo("Unable to locate variable %s in hashtable. Memory problem, report bug.\n",
	name.c_str());
    return;
  }
  *vardata->variable->value=b;
  vardata->variable->updated();
}
                        

// UVar Casting

UVar::operator int () {	 
  //check of dataType is done inside in and out
  if (owned)
    return (int)out();
  else
    return (int)in();   
}

UVar::operator ufloat () { 
  //check of dataType is done inside in and out
  if (owned)
    return out();
  else
    return in();   
}


UVar::operator string () { 

  if ((vardata)  && (vardata->variable->value->dataType == ::DATA_STRING))    
    return (string(vardata->variable->value->str->str()));  
  else  
    return string("");    
}

UVar::operator UList() {
return (UList)*vardata->variable->value;

}

UVar::operator urbi::UBinary() {
  if ((vardata)  && (vardata->variable->value->dataType == ::DATA_BINARY)) {
    //simplest way is to echo our bin headers and parse again
    UBinary b;
    std::ostringstream msg;
    msg << vardata->variable->value->refBinary->ref()->bufferSize;
    UNamedParameters *param = vardata->variable->value->refBinary->ref()->parameters;
    while (param) {
      if (param->expression) {
	if (param->expression->dataType == ::DATA_NUM)
	  msg<< " "<<(int)param->expression->val;
	else if (param->expression->dataType == ::DATA_STRING)
	  msg << " "<<param->expression->str->str();
      }
      param = param->next;
    }
    

    std::list<urbi::BinaryData> lBin;
    lBin.push_back(urbi::BinaryData( vardata->variable->value->refBinary->ref()->buffer,  vardata->variable->value->refBinary->ref()->bufferSize));
    std::list<urbi::BinaryData>::iterator lIter = lBin.begin();
    b.parse(msg.str().c_str(), 0, lBin, lIter);
  return b;
  }
  else return UBinary();
}

UVar::operator urbi::UBinary*() {
  if ((vardata)  && (vardata->variable->value->dataType == ::DATA_BINARY)) {
    //simplest way is to echo our bin headers and parse again
    UBinary* b = new UBinary();
    std::ostringstream msg;
    msg << vardata->variable->value->refBinary->ref()->bufferSize;
    UNamedParameters *param = vardata->variable->value->refBinary->ref()->parameters;
    while (param) {
      if (param->expression) {
	if (param->expression->dataType == ::DATA_NUM)
	  msg<< " "<<(int)param->expression->val;
	else if (param->expression->dataType == ::DATA_STRING)
	  msg << " "<<param->expression->str->str();
      }
      param = param->next;
    }

    std::list<urbi::BinaryData> lBin;
    lBin.push_back(urbi::BinaryData( vardata->variable->value->refBinary->ref()->buffer,  vardata->variable->value->refBinary->ref()->bufferSize));
    std::list<urbi::BinaryData>::iterator lIter = lBin.begin();
    b->parse(msg.str().c_str(), 0, lBin, lIter);
    return b;
  }
  else return new UBinary();
}

UVar::operator UImage() {
 return (UImage)*vardata->variable->value;
}

UVar::operator USound() {
 return (USound)*vardata->variable->value;
}


//! UVar out value (read mode)
ufloat&
UVar::out()
{ 
  static ufloat er=0;
  if ((vardata) && (vardata->variable->value->dataType == ::DATA_NUM))
    return (vardata->variable->target);
  else return er;
}

//! UVar in value (write mode)
ufloat&
UVar::in()
{  
 static ufloat er=0;
 if ((vardata) && (vardata->variable->value->dataType == ::DATA_NUM))
   return (vardata->variable->value->val);
 else return er;
}

UBlendType
UVar::blend()
{
  if (vardata)
    return ((UBlendType)vardata->variable->blendType);
  else {
    urbi::echo("Internal error on variable 'vardata', should not be zero\n");
    return (UNORMAL);
  }
}




//! UVar update
void
UVar::__update(UValue& v)
{ 
  value = v;
}


 
