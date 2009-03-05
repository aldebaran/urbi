/*! \file libuco/utable.cc
 *******************************************************************************

 Implementation of the UObject class.

 This file is part of LIBURBI\n
 Copyright (c) 2004-2009 Jean-Christophe Baillie.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include <urbi/utable.hh>
#include <urbi/ucallbacks.hh>

// Lists and hashtables used.

namespace urbi
{

#define SINGLETON_FUNCTION(Type, Name)          \
  Type& Name()                                  \
  {                                             \
    static Type instance;                       \
    return instance;                            \
  }

  SINGLETON_FUNCTION(UTable, accessmap);
  SINGLETON_FUNCTION(UTable, eventendmap);
  SINGLETON_FUNCTION(UTable, eventmap);
  SINGLETON_FUNCTION(UTable, functionmap);
  SINGLETON_FUNCTION(UTable, monitormap);
  SINGLETON_FUNCTION(UVarTable, varmap);
  SINGLETON_FUNCTION(UTimerTable, timermap);

#undef SINGLETON_FUNCTION


  UTable::UTable()
  {
  }

  UTable::callbacks_type*
  UTable::find0(const std::string& name)
  {
    // FIXME: Use libport::find0 which first needs to be educated to
    // support the case where the mapped_type is not a pointer type.
    super_type::iterator i = find(name);
    if (i != end())
      return &i->second;
    else
      return 0;
  }

  //! Clean a callback UTable from all callbacks linked to the
  //! object whose name is 'name'
  void
  UTable::clean(const std::string& name)
  {
    std::list<UTable::iterator> todelete;
    for (UTable::iterator i = begin(); i != end(); ++i)
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
      erase(*i);
  }


} // namespace urbi
