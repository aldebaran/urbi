/*! \file uobjectdata.cc
 *******************************************************************************

 File: uobjectdata.cc\n
 Implementation of the UObjectData class.

 This file is part of LIBURBI\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include <list>
#include "uobject.h"
#include "uobjectdata.h"

using namespace URBI;
using namespace std;
	
// Global scope variables with namespace URBI

namespace URBI {
  string serverIP;
  UObject* lastUObject;
}
	
// **************************************************************************	
//! UObjectData constructor.
UObjectData::UObjectData(UObject *obj)
{ 
  object = obj;
}


//! UObjectData destructor.
UObjectData::~UObjectData()
{  
}

