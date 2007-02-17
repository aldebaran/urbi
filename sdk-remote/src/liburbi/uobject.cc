/*! \file uobject.cc
 *******************************************************************************

 File: uobject.cc\n
 Implementation of the UObject class.

 This file is part of LIBURBI\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include <iostream>
#include <sstream>
#include <list>

#include "urbi/uobject.hh"

#include "urbi/usyncclient.hh"
#include "urbi/uexternal.hh"

//#define LIBURBIDEBUG

//! Global definition of the starterlist
namespace urbi
{
  UObject* dummyUObject;

  UCallbackAction dispatcher(const UMessage& msg);
  UCallbackAction debug(const UMessage& msg);


  std::ostream& unarmorAndSend(const char* a);

  void uobject_unarmorAndSend(const char* a)
  {
    unarmorAndSend(a);
  }

  void send(const char* a)
  {
    std::ostream& s = getDefaultClient() == 0 ? std::cerr
      : ((UAbstractClient*)getDefaultClient())->getStream();
    s << a;
  }

  void send(void* buf, int size)
  {
    std::ostream& s = getDefaultClient() == 0 ? std::cerr
      : ((UAbstractClient*)getDefaultClient())->getStream();
    s.rdbuf()->sputn(static_cast<char*> (buf), size);
  }


  // **************************************************************************
  //! UGenericCallback constructor.
  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string& type,
				     const std::string& name,
				     int size,  UTable& t)
    : objname (objname), name (name)
  {
    nbparam = size;

    if (type == "function" || type== "event" || type == "eventend")
    {
      std::ostringstream oss;
      oss << size;
      this->name = this->name + "__" + oss.str();
    }
    t[this->name].push_back(this);

    std::cout << "Registering " << type << " " << name << " " << size
	      << " into " << this->name
	      << " from " << objname
	      << std::endl;

    if (type == "var")
      URBI(()) << "external " << type << " "
	       << name << " from " << objname << ";";

    if (type == "event" || type == "function")
      URBI(()) << "external " << type << "(" << size << ") "
	       << name << " from " << objname << ";";

    if (type == "varaccess")
      echo("Warning: NotifyAccess facility is not available for modules in "
	   "remote mode.\n");
  };

  //! UGenericCallback constructor.
  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string& type,
				     const std::string& name, UTable& t)
    : objname(objname), name(name)
  {
    t[this->name].push_back(this);
    URBI(()) << "external " << type << " " << name << ";";
  };

  UGenericCallback::~UGenericCallback()
  {
  };


  // **************************************************************************
  //! UTimerCallbacl constructor.

  UTimerCallback::UTimerCallback(const std::string& objname,
				 ufloat period, UTimerTable& tt)
    : period(period),
      objname(objname)
  {
    tt.push_back(this);
    lastTimeCalled = -9999999;
    std::ostringstream os;
    os << "timer"<<tt.size();
    //register oursselves as an event
    std::string cbname = os.str();

    // needed by MSVC
    //createUCallback(objname, "event", this, &UTimerCallback::call, objname +
    //                "." + cbname, eventmap);
    new UCallbackvoid0<UTimerCallback> (objname, "event", this,
					&UTimerCallback::call,
					objname + '.' + cbname, eventmap);

    os.str("");
    os.clear();
    os << "timer_" << objname << ": every(" << period << ") { emit "
       << (objname + '.' + cbname) << ";};";
    URBI(()) << os.str();
  }

  UTimerCallback::~UTimerCallback()
  {
  }

  // **************************************************************************
  //  Monitoring functions

  //! Generic UVar monitoring without callback
  void
  UObject::USync(UVar&)
  {
    //UNotifyChange(v, &UObject::voidfun);
  }

  // **************************************************************************
  //! UObject constructor.
  UObject::UObject(const std::string& s)
    : __name(s),
      classname(s),
      derived(false),
      remote (true),
      load(s, "load")
  {
    objecthub = 0;
    autogroup = false;

    URBI(()) << "class " << __name << "{};";
    URBI(()) << "external object " << __name << ";";
    period = -1;

    // default
    load = 1;
  }

  //! Dummy UObject constructor.
  UObject::UObject(int index)
    : derived(false),
      remote (true)
  {
    std::stringstream ss;
    ss << "dummy" << index;
    __name = ss.str();
    classname = __name;
    objecthub = 0;
    autogroup = false;
    period = -1;
  }


  //! UObject cleaner
  void
  UObject::clean()
  {
    cleanTable(monitormap, __name);
    cleanTable(accessmap, __name);
    cleanTable(functionmap, __name);
    cleanTable(eventmap, __name);
    cleanTable(eventendmap, __name);

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
    std::string groupregister = "addgroup " + gpname +" { "+__name+"};";
    uobject_unarmorAndSend(groupregister.c_str());
  }

  void
  UObject::USetUpdate(ufloat t)
  {
    std::ostringstream os;
    if (period != -1)
    {
      //kill previous timer
      os << "stop maintimer_" << __name << ";";
      URBI(())<<os.str();
    }
    period = t;
    if (period <= 0)
      period = 1;
    std::string cbname = __name + ".maintimer";
    createUCallback(__name, "event",
		    this, &UObject::update, cbname, eventmap);
    os.str("");
    os.clear();
    os << "maintimer_" << __name << ": every(" << period << ") "
      "{ emit "<<cbname<<";};";
    URBI(()) << os.str();
  }

  // This part is specific for standalone linux objects
  // LIBURBI 'Module mode'

  UCallbackAction
  dispatcher(const UMessage& msg)
  {
    //check message type
    if (msg.type != MESSAGE_DATA || msg.value->type != DATA_LIST)
    {
      msg.client.printf("Component Error: "
			"unknown message content, type %d\n",
			(int) msg.type);
      return URBI_CONTINUE;
    }

    UList& array = *msg.value->list;

    if (array.size()<2)
    {
      msg.client.printf("Component Error: Invalid number "
			"of arguments in the server message: %d\n",
			array.size());
      return URBI_CONTINUE;
    }

    if (array[0].type != DATA_DOUBLE)
    {
      msg.client.printf("Component Error: "
			"unknown server message type %d\n",
			(int) array[0].type);
      return URBI_CONTINUE;
    }

    if (array[0].type != DATA_DOUBLE)
    {
      msg.client.printf("Component Error: "
			"unknown server message type %d\n",
			(int) array[0].type);
      return URBI_CONTINUE;
    }


    // UEM_ASSIGNVALUE
    if ((USystemExternalMessage)(int)array[0] == UEM_ASSIGNVALUE)
    {
      UVarTable::iterator varmapfind = varmap.find(array[1]);
      if (varmapfind != varmap.end())
	for (std::list<UVar*>::iterator it = varmapfind->second.begin();
	     it != varmapfind->second.end();
	     ++it)
	  (*it)->__update(array[2]);

      UTable::iterator monitormapfind = monitormap.find(array[1]);
      for (std::list<UGenericCallback*>::iterator
	     cbit = monitormapfind->second.begin();
	   cbit != monitormapfind->second.end();
	   ++cbit)
      {
	// test of return value here
	UList u;
	u.array.push_back(new UValue());
	u[0].storage = (*cbit)->storage;
	(*cbit)->__evalcall(u);
      }
    }

    // UEM_EVALFUNCTION
    else if ((USystemExternalMessage)(int)array[0] == UEM_EVALFUNCTION)
    {
      /* For the moment, this iteration is useless since the list will
       * contain one and only one element. There is no function overloading
       * yet and still it would probably use a unique name identifier, hence
       * a single element list again. */
      if (functionmap.find(array[1]) != functionmap.end())
      {
	std::list<UGenericCallback*> tmpfun = functionmap[array[1]];
	std::list<UGenericCallback*>::iterator tmpfunit = tmpfun.begin();
	array.setOffset(3);
	UValue retval = (*tmpfunit)->__evalcall(array);
	array.setOffset(0);
	if (retval.type == DATA_VOID)
	  URBI(()) << "var " << (std::string) array[2];
	else
	{
	  URBI(()) << (std::string) array[2] << "=";
	  getDefaultClient()->send(retval);//I'd rather not use << for bins
	}
	URBI(()) << ";";
      }
      else
	msg.client.printf("Component Error: %s function unknown.\n",
			  ((std::string) array[1]).c_str());
    }

    // UEM_EMITEVENT
    else if ((USystemExternalMessage)(int)array[0] == UEM_EMITEVENT)
    {
      if (eventmap.find(array[1]) != eventmap.end())
      {
	std::list<UGenericCallback*>  tmpfun = eventmap[array[1]];
	for (std::list<UGenericCallback*>::iterator tmpfunit = tmpfun.begin();
	     tmpfunit != tmpfun.end();
	     ++tmpfunit)
	{
	  array.setOffset(2);
	  (*tmpfunit)->__evalcall(array);
	  array.setOffset(0);
	}
      }
    }

    // UEM_ENDEVENT
    else if ((USystemExternalMessage)(int)array[0] == UEM_ENDEVENT)
    {
      if (eventendmap.find(array[1]) != eventendmap.end())
      {
	std::list<UGenericCallback*>  tmpfun = eventendmap[array[1]];
	for (std::list<UGenericCallback*>::iterator tmpfunit = tmpfun.begin();
	     tmpfunit != tmpfun.end();
	     ++tmpfunit)
	{
	  array.setOffset(2);
	  (*tmpfunit)->__evalcall(array);
	  array.setOffset(0);
	}
      }
    }

    // UEM_NEW
    else if ((USystemExternalMessage)(int)array[0] == UEM_NEW)
    {
      std::list<baseURBIStarter*>::iterator found = objectlist->end();
      for (std::list<baseURBIStarter*>::iterator retr = objectlist->begin();
	   retr != objectlist->end();
	   ++retr)
	if ((*retr)->name == (std::string)array[2])
	  if (found != objectlist->end())
	    msg.client.printf("Double object definition %s\n",
			      (*retr)->name.c_str());
	  else
	    found = retr;

      if (found == objectlist->end())
	msg.client.printf("Unknown object definition %s\n",
			  ((std::string) array[2]).c_str());
      else
	(*found)->copy((std::string) array[1]);

    }

    // UEM_DELETE
    else if ((USystemExternalMessage)(int)array[0] == UEM_DELETE)
    {
      std::list<baseURBIStarter*>::iterator found = objectlist->end();
      for (std::list<baseURBIStarter*>::iterator retr = objectlist->begin();
	   retr != objectlist->end();
	   ++retr)
	if ((*retr)->name == (std::string)array[1])
	  if (found != objectlist->end())
	    msg.client.printf("Double object definition %s\n",
			      (*retr)->name.c_str());
	  else
	    found = retr;

      if (found == objectlist->end())
	msg.client.printf("Unknown object definition %s\n",
			  ((std::string) array[1]).c_str());
      else
      {
	// remove the object from objectlist or terminate
	// the component if there is nothing left
	if (objectlist->size() == 1)
	  exit(0);
	else
	{
	  // delete the object
	  delete (*found);
	}
      }
    }


    // DEFAULT
    else
      msg.client.printf("Component Error: "
			"unknown server message type number %d\n",
			(int)array[0]);

    return URBI_CONTINUE;
  }




  // **************************************************************************

  void
  UObjectHub::USetUpdate(ufloat t)
  {
    period = t;
    // nothing happend in remote mode...
  }

  //! echo method
  void
  echo(const char* format, ...)
  {
    va_list arg;
    va_start(arg, format);
    vfprintf(stderr, format, arg);
    va_end(arg);
  }



  // **************************************************************************
  // Other functions

  UCallbackAction
  debug(const UMessage& msg)
  {
    std::stringstream mesg;
    mesg<<msg;
    msg.client.printf("DEBUG: got a message  : %s\n",
		      mesg.str().c_str());

    return URBI_CONTINUE;
  }


  void
  main(int argc, char* argv[])
  {
    // Retrieving command line arguments
    if (argc!=2)
    {
      std::cout << "usage:\n"
		<< argv[0] << " <URBI Server IP>" << std::endl;
      exit(0);
    }

    //serverIP = argv[1];
    std::cout << "Running Remote Component '" << argv[0]
	      << "' on " << argv[1] << std::endl;
    //we need a usyncclient connect(argv[1]);
    new USyncClient(argv[1]);


#ifdef LIBURBIDEBUG
    getDefaultClient()->setWildcardCallback( callback (&debug));
#else
    getDefaultClient()->setErrorCallback( callback (&debug));
#endif

    getDefaultClient()->setCallback(&dispatcher,
				    externalModuleTag.c_str());

    dummyUObject = new UObject (0);
    for (UStartlist::iterator retr = objectlist->begin();
	 retr != objectlist->end();
	 ++retr)
      (*retr)->init((*retr)->name);
  }

} // namespace urbi
