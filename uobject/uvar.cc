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
UVar::UVar(string varname, bool sync)
{
  name = varname;  
  __init(sync);
}

//! UVar constructor: object reference + var name
UVar::UVar(UObject& obj, string varname, bool sync)
{
  name = obj.name + "." + varname;
  __init(sync);
}

//! UVar constructor: object name + var name
UVar::UVar(string objname, string varname, bool sync)
{
  name = objname + "." + varname;
  __init(sync);
}


//! UVar initialization
void
UVar::init(string objname, string varname, bool sync)
{  
  name = objname + "." + varname;  
  __init(sync);
}

//! UVar initializationvoid
void
UVar::__init(bool sync)
{  
  varmap[name].push_back(this);
  
  HMvariabletab::iterator it = ::urbiserver->variabletab.find(name.c_str());
  if (it == ::urbiserver->variabletab.end()) 
    vardata = new UVardata(new UVariable(name.c_str(),new
    ::UValue(),false,false,sync));  
  else
    vardata = new UVardata(it->second);
  synchro = sync;
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
UVar::operator = (UFloat n)
{ 
  if (!vardata) { 
    urbi::echo("Unable to locate variable %s in hashtable. Memory problem, report bug.\n",
	name.c_str());
    return;
  }
  
  // type mismatch is not integrated at this stage
  vardata->variable->value->dataType = ::DATA_NUM;
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

// UVar Casting

UVar::operator int () {	 

  if ((vardata)  && (vardata->variable->value->dataType == ::DATA_NUM))    
    return ((int)(vardata->variable->value->val));  
  else  
    return 0;  
};

UVar::operator UFloat () { 

  if ((vardata) && (vardata->variable->value->dataType == ::DATA_NUM))
    return (UFloat(vardata->variable->value->val));  
  else  
    return UFloat(0);  
};


UVar::operator string () { 

  if ((vardata)  && (vardata->variable->value->dataType == ::DATA_STRING))    
    return (string(vardata->variable->value->str->str()));  
  else  
    return string("");    
};


//! UVar out value (read mode)
UFloat&
UVar::out()
{ 
  if ((vardata) && (vardata->variable->value->dataType == ::DATA_NUM))
    return (vardata->variable->target);
}

//! UVar in value (write mode)
UFloat&
UVar::in()
{  
  if ((vardata) && (vardata->variable->value->dataType == ::DATA_NUM))
    return (vardata->variable->value->val);
}




//! UVar update
void
UVar::__update(UValue& v)
{ 
  value = v;
}


 
