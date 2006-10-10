/*! \file uobject.cc
*******************************************************************************

File: uobject.cc\n
Implementation of the UObject class.

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

#include <cstdarg>
#include <cstdio>
#include <list>
#include "userver.h"
#include "uconnection.h"
#include "ughostconnection.h"
#include "uobject.h"
#include "utypes.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif
#define LIBURBIDEBUG

//! Global definition of the starterlist
namespace urbi
{

  UObject* lastUObject;

  STATIC_INSTANCE(UStartlist, objectlist);
  STATIC_INSTANCE(UStartlistHub, objecthublist);

  const std::string externalModuleTag = "__ExternalMessage__";

  UVarTable varmap;
  UTable functionmap;
  UTable monitormap;
  UTable accessmap;
  UTable eventmap;
  UTable eventendmap;

  UTimerTable timermap;
  UTimerTable updatemap;


  UVar& cast(UValue &v, UVar *)
  {
    return (*((UVar*)v.storage));
  };

  UBinary cast(UValue &v, UBinary *)
  {
    if (v.type != DATA_BINARY)
      return UBinary();
    return UBinary(*v.binary);
  }

  UList cast(UValue &v, UList *)
  {
    if (v.type != DATA_LIST)
      return UList();
    return UList(*v.list);
  }

  UObjectStruct cast(UValue &v, UObjectStruct*)
  {
    if (v.type != DATA_OBJECT)
      return UObjectStruct();
    return UObjectStruct(*v.object);
  }

  // USeful sending functions

  void uobject_unarmorAndSend(const char * str)
  {
    //feed this to the ghostconnection
    UConnection * ghost = urbiserver->getGhostConnection();
    if (strlen(str)>=2 && str[0]=='(')
      ghost->received((const unsigned char *)(str+1),strlen(str)-2);
    else
      ghost->received(str);

    ghost->newDataAdded = true;
  }

  void send(const char* str)
  {
    //feed this to the ghostconnection
    UConnection * ghost = urbiserver->getGhostConnection();
    ghost->received(str);
    ghost->newDataAdded = true;
  }

  void send(void* buf, int size)
  {
    //feed this to the ghostconnection
    UConnection * ghost = urbiserver->getGhostConnection();
    ghost->received((const unsigned char *)(buf),size);
    ghost->newDataAdded = true;
  }


  void
  main(int argc, char *argv[])
  {
    // no effect here
  }

  // **************************************************************************
  //! UGenericCallback constructor.
  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string& type,
				     const std::string& name,
				     int size,  UTable &t)
    : storage(0), objname(objname), name(name)
  {
    nbparam = size;

    if (type == "function"
	|| type== "event"
	|| type=="eventend")
      t[this->name].push_back(this);

    if (type == "var" || type=="var_onrequest")
      {
	HMvariabletab::iterator it = ::urbiserver->variabletab.find(name.c_str());
	if (it == ::urbiserver->variabletab.end())
	  {
	    UVariable *variable = new UVariable(name.c_str(), new ::UValue());
	    if (variable) variable->internalBinder.push_back(this);
	  }
	else
	  it->second->internalBinder.push_back(this);
      }

    if (type == "varaccess")
      {
	HMvariabletab::iterator it = ::urbiserver->variabletab.find(name.c_str());
	if (it == ::urbiserver->variabletab.end())
	  {
	    UVariable *variable = new UVariable(name.c_str(), new ::UValue());
	    if (variable)
	      variable->internalAccessBinder.push_back(this);
	  }
	else
	  it->second->internalAccessBinder.push_back(this);
      }
  }

  //! UGenericCallback constructor.
  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string& type,
				     const std::string& name, UTable &t)
    : storage(0), objname(objname), name(name)
  {
    t[this->name].push_back(this);
  }

  UGenericCallback::~UGenericCallback()
  {
  }


  // **************************************************************************
  //! UTimerCallbacl constructor.

  UTimerCallback::UTimerCallback(const std::string& objname,
				 ufloat period, UTimerTable &tt)
    : period(period),
      objname(objname)
  {
    tt.push_back(this);
    lastTimeCalled = -9999999;
  }

  UTimerCallback::~UTimerCallback()
  {
  }


  // **************************************************************************
  //  Monitoring functions

  //! Generic UVar monitoring without callback
  void
  UObject::USync(UVar &v)
  {
    UNotifyChange(v,&UObject::voidfun);
  }

  // **************************************************************************
  //! UObject constructor.
  UObject::UObject(const std::string &s)
    : __name(s)
  {
    objecthub = 0;
    autogroup = false;
    lastUObject = this;
    UString tmps(__name.c_str()); // quelle merde ces UString!!!!
    UObj* tmpobj = new UObj(&tmps);

    for (UStartlist::iterator retr = urbi::objectlist->begin();
	 retr != objectlist->end();
	 retr++)
      if ((*retr)->name == __name)
	tmpobj->internalBinder = (*retr);

    // default
    derived = false;
    classname = __name;

    UBindVar(UObject,load);
    load = 1;
  }


  //! Clean a callback UTable from all callbacks linked to the object whose name is 'name'
  void
  cleanTable(UTable &t, std::string name)
  {
    std::list<UTable::iterator> todelete;
    for (UTable::iterator it = t.begin();
	 it != t.end();
	 ++it)
      {
	std::list<UGenericCallback*>& tocheck = (*it).second;
	for (std::list<UGenericCallback*>::iterator it2 = tocheck.begin();
	     it2 != tocheck.end();
	     )
	  {
	    if ((*it2)->objname == name)
	      {
		delete *it2;
		it2 = tocheck.erase(it2);
	      }
	    else
	      ++it2;
	  }//for

	if (tocheck.empty())
	  todelete.push_back(it);
      }//for

    for (std::list<UTable::iterator>::iterator dit = todelete.begin();
	 dit != todelete.end();
	 ++dit)
      {
	t.erase(*dit);
      }
  }//function


  //! Clean a callback UTimerTable from all callbacks linked to the object whose name is 'name'
  void
  cleanTimerTable(UTimerTable &t, std::string name)
  {
    for (UTimerTable::iterator it = t.begin();
	 it != t.end();
	 )
      {
	if ((*it)->objname == name)
	  {
	    delete *it;
	    it = t.erase(it);
	  }
	else
	  ++it;
      }//for
  }//function


  //! UObject cleaner
  void
  UObject::clean()
  {
    cleanTable(functionmap, __name);
    cleanTable(eventmap, __name);
    cleanTable(eventendmap, __name);

    cleanTimerTable(timermap, __name);
    cleanTimerTable(updatemap, __name);

    if (objecthub)
      objecthub->members.remove(this);
  }



  //! UObject destructor.
  UObject::~UObject()
  {
    clean();
  }

  void
  UObject::UJoinGroup(const std::string& gpname)
  {
    HMgrouptab::iterator hma;
    UGroup *g;

    hma = ::urbiserver->grouptab.find(gpname.c_str());
    if (hma != ::urbiserver->grouptab.end())
      g = hma->second;
    else
      {
	g = new UGroup(new UString(gpname.c_str()));
	::urbiserver->grouptab[g->name->str()] = g;
      }

    g->members.push_back(new UString(__name.c_str()));
  }

  void
  UObject::USetUpdate(ufloat t)
  {
    period = t;
    new UTimerCallbackobj<UObject>(__name, t, this,
				   &UObject::update, updatemap);
  }

  // **************************************************************************
  //! UObjectHub constructor.

  UObjectHub::UObjectHub(const std::string& s) : name(s)
  {
  }

  //! UObjectHub destructor.
  UObjectHub::~UObjectHub()
  {
  }

  void
  UObjectHub::USetUpdate(ufloat t)
  {
    period = t;
    new UTimerCallbackobj<UObjectHub>(name, t, this,
				      &UObjectHub::updateGlobal, updatemap);
  }

  int
  UObjectHub::updateGlobal()
  {
    for (UObjectList::iterator it = members.begin();
	 it != members.end();
	 it++)
      (*it)->update();
    update();
    return 0;
  }

  void
  UObjectHub::addMember(UObject* obj)
  {
    members.push_back(obj);
  }

  UObjectList*
  UObjectHub::getSubClass(const std::string& subclass)
  {
    UObjectList* res = new UObjectList();
    for (UObjectList::iterator it = members.begin();
	 it != members.end();
	 it++)
      if ((*it)->classname == subclass)
	res->push_back(*it);

    return(res);
  }


  //! retrieve a UObjectHub based on its name
  UObjectHub*
  getUObjectHub(const std::string& name)
  {
    for (UStartlistHub::iterator retr = objecthublist->begin();
	 retr != objecthublist->end();
	 retr++)
      if ((*retr)->name == name)
	return (*retr)->getUObjectHub();

    return 0;
  }

  //! retrieve a UObject based on its name
  UObject*
  getUObject(const std::string& name)
  {
    for (UStartlist::iterator retr = objectlist->begin();
	 retr != objectlist->end();
	 retr++)
      if ((*retr)->name == name)
	return (*retr)->getUObject();

    return 0;
  }

  //! echo method
  void
  echo(const char* format, ... )
  {
    char tmpoutput[1024];

    va_list arg;
    va_start(arg, format);
    vsnprintf(tmpoutput, 1024, format, arg);
    va_end(arg);

    ::urbiserver->debug(tmpoutput);
  }

}
