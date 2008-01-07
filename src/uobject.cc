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
#include <algorithm>

#include "libport/containers.hh"

#include "urbi/uobject.hh"

#include "kernel/userver.hh"
#include "kernel/utypes.hh"
#include "kernel/uconnection.hh"
#include "kernel/uvalue.hh"
#include "kernel/uvariable.hh"

#include "ughostconnection.hh"
#include "ugroup.hh"
#include "uobj.hh"
#include "ufunction.hh"

//! Global definition of the starterlist
namespace urbi
{

  UObject* lastUObject;

  const std::string externalModuleTag = "__ExternalMessage__";


  // Useful sending functions.

  void uobject_unarmorAndSend(const char* str)
  {
    //feed this to the ghostconnection
    UConnection& ghost = urbiserver->getGhostConnection();
    if (strlen(str)>=2 && str[0]=='(')
      ghost << UConnection::received((const unsigned char *)(str+1), strlen(str)-2);
    else
      ghost << UConnection::received(str);

    ghost.newDataAdded = true;
  }

  void send(const char* str)
  {
    //feed this to the ghostconnection
    UConnection& ghost = urbiserver->getGhostConnection();
    ghost << UConnection::received(str);
    ghost.newDataAdded = true;
  }

  void send(void* buf, int size)
  {
    //feed this to the ghostconnection
    UConnection& ghost = urbiserver->getGhostConnection();
    ghost << UConnection::received((const unsigned char *)(buf), size);
    ghost.newDataAdded = true;
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
				     int size,  UTable &t, bool owned)
    : storage(0), objname(objname), name(name)
  {
    nbparam = size;

    // Autodetect redefined members higher in the hierarchy of an object
    // If one is found, cancel the binding.
    if (type == "function")
    {
      HMobjtab::iterator it = ::urbiserver->objtab.find(objname.c_str ());
      if (it != ::urbiserver->objtab.end())
      {
	UObj* srcobj = it->second;
	std::string member = name.substr (name.find ('.') + 1);
	bool ambiguous;
	UFunction* fun = srcobj->searchFunction (member.c_str (), ambiguous);
	if (fun && fun != kernel::remoteFunction && !ambiguous)
	  return;
      }
    }

    if (type == "function" || type == "event" || type =="eventend")
      t[this->name].push_back(this);

    if (type == "var" || type=="var_onrequest")
    {
      HMvariabletab::iterator it = ::urbiserver->variabletab.find(name.c_str());
      if (it == ::urbiserver->variabletab.end())
      {
	UVariable *v = new UVariable(name.c_str(), new ::UValue());
	if (v)
	{
	  if (owned)
	    v->internalTargetBinder.push_back(this);
	  else
	    v->internalBinder.push_back(this);
	}
      }
      else
      {
	if (owned)
	  it->second->internalTargetBinder.push_back(this);
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
	if ((!it->second->internalBinder.empty ()
	     || it->second->binder)
	    && !libport::has (::urbiserver->access_and_change_varlist,
			      it->second))
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

  void
  UGenericCallback::registerCallback(UTable&)
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
      gc (0),
      remote (false),
      load(s, "load")
  {
    objecthub = 0;
    autogroup = false;
    period = -1;
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
      gc (0),
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


  //! Clean a callback UTimerTable from all callbacks linked to
  //! the object whose name is \a n.
  static void
  cleanTimerTable(UTimerTable &t, const std::string& n)
  {
    for (UTimerTable::iterator i = t.begin(); i != t.end(); )
      if ((*i)->objname == n)
      {
	delete *i;
	i = t.erase(i);
      }
      else
	++i;
  }


  // Remove from \a t the item whose objname is \a n (if present).
  // FIXME: Interestingly enough, might be equal to the above, if we
  // know that the name can appear only once.  Using lists is really
  // not smart.
  static void
  remove (UTimerTable& t, const std::string& n)
  {
    // Find previous update timer on this object
    for (UTimerTable::iterator i = t.begin (); i != t.end(); ++i)
      if ((*i)->objname == n)
      {
	updatemap->erase (i);
	delete *i;
	break;
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
    delete gc;
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
      ::urbiserver->grouptab[g->name.c_str()] = g;
    }

    g->members.push_back(new UString(__name.c_str()));
  }

  void
  UObject::USetUpdate(ufloat t)
  {
    // Find previous update timer on this object
    remove (updatemap, __name);

    // Set period value
    period = t;
    // Do nothing more if negative value given
    if (period < 0)
      return;

    // Create callback
    new UTimerCallbackobj<UObject>(__name, t, this,
				   &UObject::update, updatemap);
    return;
  }

  int
  UObject::send (const std::string& s)
  {
    if (!gc)
      gc = new UGhostConnection (::urbiserver);

    return (*gc << UConnection::received (s.c_str ())).error ();
  }

  // **************************************************************************

  //! UObjectHub destructor.
  UObjectHub::~UObjectHub()
  {
    cleanTimerTable(updatemap, name);
  }

  void
  UObjectHub::USetUpdate(ufloat t)
  {
    // Find previous update timer on this object
    remove (updatemap, name);

    // Set period value
    period = t;
    // Do nothing more if negative value given
    if (period < 0)
      return;

    // Create callback
    new UTimerCallbackobj<UObjectHub>(name, t, this,
				      &UObjectHub::updateGlobal,
				      updatemap);
    return;
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
