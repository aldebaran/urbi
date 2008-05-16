/*! \file uobject-common.cc
 *******************************************************************************

 File: uobject-common.cc\n
 Implementation of the UObject class.

 This file is part of LIBURBI\n
Copyright (c) 2004, 2005, 2006, 2007, 2008 Jean-Christophe Baillie.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include <list>

#include "urbi/uobject.hh"

// These calls are made out of any namespace due to vcxx error C2888
// cf: http://msdn.microsoft.com/en-us/library/27zksbks(VS.80).aspx
STATIC_INSTANCE_NS(UStartlist, objectlist, urbi);
STATIC_INSTANCE_NS(UStartlistHub, objecthublist, urbi);

// Lists and hashtables used.
STATIC_INSTANCE_NS(UTable, accessmap, urbi);
STATIC_INSTANCE_NS(UTable, eventendmap, urbi);
STATIC_INSTANCE_NS(UTable, eventmap, urbi);
STATIC_INSTANCE_NS(UTable, functionmap, urbi);
STATIC_INSTANCE_NS(UTable, monitormap, urbi);
STATIC_INSTANCE_NS(UVarTable, varmap, urbi);

// Timer and update maps.
STATIC_INSTANCE_NS(UTimerTable, timermap, urbi);
STATIC_INSTANCE_NS(UTimerTable, updatemap, urbi);

namespace urbi
{
  //! Clean a callback UTable from all callbacks linked to the
  //! object whose name is 'name'
  void
  cleanTable(UTable &t, const std::string& name)
  {
    std::list<UTable::iterator> todelete;
    for (UTable::iterator i = t.begin(); i != t.end(); ++i)
    {
      std::list<UGenericCallback*>& tocheck = i->second;
      for (std::list<UGenericCallback*>::iterator j = tocheck.begin();
	   j != tocheck.end();
	)
      {
	if ((*j)->objname == name)
	{
	  delete *j;
	  j = tocheck.erase(j);
	}
	else
	  ++j;
      }

      if (tocheck.empty())
	todelete.push_back(i);
    }

    for (std::list<UTable::iterator>::iterator i = todelete.begin();
	 i != todelete.end();
	 ++i)
      t.erase(*i);
  }

} // namespace urbi
