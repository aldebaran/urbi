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
#include "uobjectdata.h"
using namespace URBI;

#include <uclient.h>
		
// **************************************************************************	
//! UVar constructor: implicit object ref (using 'lastUOjbect') + varname
UVar::UVar(string varname, bool writeonly)
{
  name = varname;  
  __init(writeonly);
}

//! UVar constructor: object reference + var name
UVar::UVar(UObject& obj, string varname, bool writeonly)
{
  name = obj.get_name() + "." + varname;
  __init(writeonly);
}

//! UVar constructor: object name + var name
UVar::UVar(string objname, string varname, bool writeonly)
{
  name = objname + "." + varname;
  __init(writeonly);
}


//! UVar initialization
void
UVar::init(string objname, string varname)
{  
  name = objname + "." + varname;  
  __init(false);
}

//! UVar initialization
/*! The writeonly flag is an optimization: when a variable is 'writeonly', it
    will not be updated by the server when the server changes it's value. This
    will save bandwidth in the case when you only want to assign (write)
    values on the variable and don't care about it's actually server-side value
*/
void
UVar::__init(bool writeonly)
{  
  if (!writeonly)
    URBI() << "external " << name << ";";

  varmap[name] = this;
}

//! UVar destructor.
UVar::~UVar()
{  
}

//! UVar float assignment
void
UVar::operator = (float n)
{  
  URBI() << name << "=" << n << ";";  
}

//! UVar string assignment
void
UVar::operator = (string s)
{  
  URBI() << name << "=\"" << s << "\";";  
}

//! UVar update
void
UVar::__update(UValue& v)
{  
  cout << "  Variable " << name << " updated to : ";
  if (v.type == MESSAGE_DOUBLE)
    cout << (double)v << endl;
  if (v.type == MESSAGE_STRING)
    cout << (string)v << endl;

  value = v;
  value.associatedVarName = name;

  if (monitormap.find(name) != monitormap.end()) {
  
    list<UGenericCallback*> cb = monitormap[name];
    cout << "there is somebody home..." << endl;
    for (list<UGenericCallback*>::iterator cbit = cb.begin();
    	 cbit != cb.end();
	 cbit++)
      // test of return value here
      (*cbit)->__evalcall(&value);
  }
}
