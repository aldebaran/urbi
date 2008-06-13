/*! \file urbi/utable.cc
 *******************************************************************************

 File: urbi/utable.cc\n
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

#include <urbi/utable.hh>
#include <urbi/ucallbacks.hh>

namespace urbi
{
  STATIC_INSTANCE(UStartlist, objectlist);
  STATIC_INSTANCE(UStartlistHub, objecthublist);

  // Lists and hashtables used.
  STATIC_INSTANCE(UTable, accessmap);
  STATIC_INSTANCE(UTable, eventendmap);
  STATIC_INSTANCE(UTable, eventmap);
  STATIC_INSTANCE(UTable, functionmap);
  STATIC_INSTANCE(UTable, monitormap);
  STATIC_INSTANCE(UVarTable, varmap);

  // Timer and update maps.
  STATIC_INSTANCE(UTimerTable, timermap);
  STATIC_INSTANCE(UTimerTable, updatemap);

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
