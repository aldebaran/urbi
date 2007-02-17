/*! \file uobject-common.cc
 *******************************************************************************

 File: uobject-common.cc\n
 Implementation of the UObject class.

 This file is part of LIBURBI\n
Copyright (c) 2004, 2005, 2006, 2007 Jean-Christophe Baillie.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include <list>

#include "urbi/uobject.hh"

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

  UVar& cast(UValue &v, UVar *)
  {
    return *((UVar*)v.storage);
  }

  UBinary cast(UValue& v, UBinary*)
  {
    if (v.type != DATA_BINARY)
      return UBinary();
    return UBinary(*v.binary);
  }

  UList cast(UValue& v, UList*)
  {
    if (v.type != DATA_LIST)
      return UList();
    return UList(*v.list);
  }

  UObjectStruct cast(UValue& v, UObjectStruct*)
  {
    if (v.type != DATA_OBJECT)
      return UObjectStruct();
    return UObjectStruct(*v.object);
  }

  const char* cast(UValue& v, const char**)
  {
    static const char* er = "invalid";
    if (v.type != DATA_STRING)
      return er;
    return v.stringValue->c_str();
  }

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
