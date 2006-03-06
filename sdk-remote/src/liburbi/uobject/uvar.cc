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
using namespace urbi;

#include <uclient.h>

namespace 
urbi {
  class UVardata {
    public:	
    UVardata() {};
    ~UVardata() {};
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

//! UVar initialization
void
UVar::__init(bool sync)
{  
  varmap[name].push_back(this);
  vardata = 0; // unused. For internal softdevices only
  synchro = true;// sync is unused here in remote mode.
}

//! UVar out value (read mode)
UFloat&
UVar::out()
{  
  return (value.val); 
}

//! UVar in value (write mode)
UFloat&
UVar::in()
{  
  return (value.val); 
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
  URBI() << name << "=" << n << ";";  
}

//! UVar string assignment
void
UVar::operator = (string s)
{  
  URBI() << name << "=\"" << s << "\";";  
}

UVar::operator int () {	 
  return ((int)value); 
};

UVar::operator UFloat () { 
  return ((UFloat)value); 
};


UVar::operator string () { 
  return ((string)value); 
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

