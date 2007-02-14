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
#include "libport/cstdio"
#include <list>
#include <sstream>

#include "uconnection.hh"
#include "ughostconnection.hh"
#include "ugroup.hh"
#include "uobj.hh"
#include "urbi/uobject.hh"
#include "userver.hh"
#include "utypes.hh"
#include "uvalue.hh"
#include "uvariable.hh"

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
    return *((UVar*)v.storage);
  };

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

  // Useful sending functions.

  void uobject_unarmorAndSend(const char* str)
  {
    //feed this to the ghostconnection
    UConnection* ghost = urbiserver->getGhostConnection();
    if (strlen(str)>=2 && str[0]=='(')
      ghost->received((const unsigned char *)(str+1), strlen(str)-2);
    else
      ghost->received(str);

    ghost->newDataAdded = true;
  }

  void send(const char* str)
  {
    //feed this to the ghostconnection
    UConnection* ghost = urbiserver->getGhostConnection();
    ghost->received(str);
    ghost->newDataAdded = true;
  }

  void send(void* buf, int size)
  {
    //feed this to the ghostconnection
    UConnection* ghost = urbiserver->getGhostConnection();
    ghost->received((const unsigned char *)(buf), size);
    ghost->newDataAdded = true;
  }


  void
  main(int, char *[])
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

    if (type == "function" || type == "event" || type == "eventend")
      t[this->name].push_back(this);

    if (type == "var" || type=="var_onrequest")
    {
      HMvariabletab::iterator it = ::urbiserver->variabletab.find(name.c_str());
      if (it == ::urbiserver->variabletab.end())
      {
	UVariable *v = new UVariable(name.c_str(), new ::UValue());
	if (v)
	  v->internalBinder.push_back(this);
      }
      else
      {
	it->second->internalBinder.push_back(this);
	if ( !it->second->internalAccessBinder.empty ()
	     && std::find (::urbiserver->access_and_change_varlist.begin (),
			   ::urbiserver->access_and_change_varlist.end (),
			   it->second) ==
	     ::urbiserver->access_and_change_varlist.end ())
	{
	  it->second->access_and_change = true;
	  ::urbiserver->access_and_change_varlist.push_back (it->second);
	}
      }
    }

    if (type == "varaccess")
    {
      HMvariabletab::iterator it = ::urbiserver->variabletab.find(name.c_str());
      if (it == ::urbiserver->variabletab.end())
      {
	UVariable *v = new UVariable(name.c_str(), new ::UValue());
	if (v)
	  v->internalAccessBinder.push_back(this);
      }
      else
      {
	it->second->internalAccessBinder.push_back(this);
	if ( (!it->second->internalBinder.empty ()
	      || it->second->binder)
	     && std::find (::urbiserver->access_and_change_varlist.begin (),
			   ::urbiserver->access_and_change_varlist.end (),
			   it->second) ==
	     ::urbiserver->access_and_change_varlist.end ())
	{
	  it->second->access_and_change = true;
	  ::urbiserver->access_and_change_varlist.push_back (it->second);
	}
      }
    }
  }

  //! UGenericCallback constructor.
  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string&,
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
  UObject::USync(UVar &)
  {
    // nothing to do here, UVars are always sync'd in plugin mode.
  }

  // **************************************************************************
  //! UObject constructor.
  UObject::UObject(const std::string &s)
    : __name(s),
      classname(s),
      derived(false),
      remote (false),
      load(s, "load")
  {
    objecthub = 0;
    autogroup = false;
    lastUObject = this;
    UString tmps(__name.c_str()); // quelle merde ces UString!!!!
    UObj* tmpobj = new UObj(&tmps);

    for (UStartlist::iterator i = urbi::objectlist->begin();
	 i != objectlist->end();
	 ++i)
      if ((*i)->name == __name)
	tmpobj->internalBinder = *i;

    // default
    load = 1;
  }

  //! Dummy UObject constructor.
  UObject::UObject(int index)
    : derived(false),
      remote (false)
  {
    std::stringstream ss;
    ss << "dummy" << index;
    __name = ss.str();
    classname = __name;
    objecthub = 0;
    autogroup = false;
    period = -1;
  }

  //! Clean a callback UTable from all callbacks linked to the
  //! object whose name is 'name'
  void
  cleanTable(UTable &t, const std::string& name)
  {
    std::list<UTable::iterator> todelete;
    for (UTable::iterator i = t.begin();
	 i != t.end();
	 ++i)
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


  //! Clean a callback UTimerTable from all callbacks linked to
  //! the object whose name is 'name'
  void
  cleanTimerTable(UTimerTable &t, const std::string& name)
  {
    for (UTimerTable::iterator i = t.begin();
	 i != t.end();
      )
    {
      if ((*i)->objname == name)
      {
	delete *i;
	i = t.erase(i);
      }
      else
	++i;
    }
  }


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
    HMgrouptab::iterator hma = ::urbiserver->grouptab.find(gpname.c_str());
    UGroup *g;
    if (hma != ::urbiserver->grouptab.end())
      g = hma->second;
    else
    {
      g = new UGroup(gpname);
      ::urbiserver->grouptab[g->name.str()] = g;
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
    for (UObjectList::iterator i = members.begin();
	 i != members.end();
	 ++i)
      (*i)->update();
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
    for (UObjectList::iterator i = members.begin();
	 i != members.end();
	 ++i)
      if ((*i)->classname == subclass)
	res->push_back(*i);

    return res;
  }


  //! retrieve a UObjectHub based on its name
  UObjectHub*
  getUObjectHub(const std::string& name)
  {
    for (UStartlistHub::iterator i = objecthublist->begin();
	 i != objecthublist->end();
	 ++i)
      if ((*i)->name == name)
	return (*i)->getUObjectHub();

    return 0;
  }

  //! retrieve a UObject based on its name
  UObject*
  getUObject(const std::string& name)
  {
    for (UStartlist::iterator i = objectlist->begin();
	 i != objectlist->end();
	 ++i)
      if ((*i)->name == name)
	return (*i)->getUObject();

    return 0;
  }

  //! echo method
  void
  echo(const char* format, ... )
  {
    char buf[1024];

    va_list arg;
    va_start(arg, format);
    vsnprintf(buf, sizeof buf, format, arg);
    va_end(arg);

    ::urbiserver->debug(buf);
  }
}
