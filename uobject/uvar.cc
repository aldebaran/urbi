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
UVar::UVar(string varname)
{
  name = varname;  
  __init();
}

//! UVar constructor: object reference + var name
UVar::UVar(UObject& obj, string varname)
{
  name = obj.name + "." + varname;
  __init();
}

//! UVar constructor: object name + var name
UVar::UVar(string objname, string varname)
{
  name = objname + "." + varname;
  __init();
}


//! UVar initialization
void
UVar::init(string objname, string varname)
{  
  name = objname + "." + varname;  
  __init();
}

//! UVar initializationvoid
void
UVar::__init()
{  
  varmap[name].push_back(this);
  vardata = 0;
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
    // first time initialization
    HMvariabletab::iterator it = ::urbiserver->variabletab.find(name.c_str());
    if (it == ::urbiserver->variabletab.end()) {

      vardata = new UVardata(new UVariable(name.c_str(),new ::UValue(n)));
    }
    else
      vardata = new UVardata(it->second);
    if (!vardata) {
      urbi::echo("Unable to locate variable %s in hashtable. Memory problem, report bug.\n",
	  name.c_str());
      return;      
    }
    if (vardata->variable->value->dataType == ::DATA_VOID) 
      vardata->variable->value->dataType = ::DATA_NUM;
      
    if (vardata->variable->value->dataType != ::DATA_NUM) {
      urbi::echo("Invalid type for variable %s in softdevice assignment\n",
	  name.c_str());
      return;      
    }
  }
  vardata->variable->value->val = n;
}

//! UVar string assignment
void
UVar::operator = (string s)
{  
  if (!vardata) {
    // first time initialization
    HMvariabletab::iterator it = ::urbiserver->variabletab.find(name.c_str());
    if (it == ::urbiserver->variabletab.end()) {

      vardata = new UVardata(new UVariable(name.c_str(),new ::UValue(s.c_str())));
    }
    else
      vardata = new UVardata(it->second);
    if (!vardata) {
      urbi::echo("Unable to locate variable %s in hashtable. Memory problem, report bug.\n",
	  name.c_str());
      return;      
    }
    if (vardata->variable->value->dataType == ::DATA_VOID) {
      vardata->variable->value->dataType = ::DATA_STRING; 
      vardata->variable->value->str = new UString("");
    }

    if (vardata->variable->value->dataType != ::DATA_STRING) {
      urbi::echo("Invalid type for variable %s in softdevice assignment\n",
	  name.c_str());
      return;      
    }
  }
  
  vardata->variable->value->str->update(s.c_str());
}

//! UVar update
void
UVar::__update(UValue& v)
{ 
  value = v;
}
 
