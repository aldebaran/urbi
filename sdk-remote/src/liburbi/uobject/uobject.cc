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
#include "uobject.h"
#include "uclient.h"
#include "uobjectdata.h"
#include "usharedexternal.h"

using namespace URBI;
using namespace std;

#define LIBURBIDEBUG

//! Global definition of the starterlist
namespace URBI {
  list<baseURBIStarter*> objectlist;
  const string externalModuleTag = "__ExternalMessage__";

  UVarTable varmap;
  UTable functionmap;
  UTable monitormap;
  UTable eventmap;
  UTable eventendmap;

  UCallbackAction dispatcher(const UMessage &msg);
  UCallbackAction debug(const UMessage &msg);

  
  template <>
  UVar& cast(UValue &v) {
    return (*((UVar*)v.storage));    
  };

  template<>
  UBinary cast(UValue &v) {
	if (v.type != DATA_BINARY) {
	  return UBinary();
	}
	return UBinary(*v.binary);
  }
  
  template<>
  UList cast(UValue &v) {
	if (v.type != DATA_LIST)
	  return UList();
	return UList(*v.list);
  }
  
  template<>
  UObjectStruct cast(UValue &v) {
	if (v.type != DATA_OBJECT)
	  return UObjectStruct();
	return UObjectStruct(*v.object);
  }
  
}


// **************************************************************************	
//! UGenericCallback constructor.
UGenericCallback::UGenericCallback(string type, string name, int size,  UTable &t) : 
  name(name) 
{
  if ((type == "function") || (type== "event") || (type=="eventend")) {
    std::ostringstream oss;
    oss << size;
    this->name = this->name + "__" + oss.str();
  }
  t[this->name].push_back(this);

  cout << "Registering " << type << " " << name << " " << size << " into " << this->name << endl;

  if (type == "var")
    URBI() << "external " << type << " " << name <<";";
  if ((type == "event") || (type == "function"))
    URBI() << "external " << type << "(" << size << ") " << name <<";";
};
	
//! UGenericCallback constructor.
UGenericCallback::UGenericCallback(string type, string name, UTable &t) : 
  name(name) 
{
  t[this->name].push_back(this);
  URBI() << "external " << type << " " << name <<";";
};

UGenericCallback::~UGenericCallback()
{
};


UGenericCallback* createUCallback(string type, void (*fun) (), string funname,UTable &t)
{
  return ((UGenericCallback*) new UCallbackGlobalvoid0 (type,fun,funname,t));
}

	
// **************************************************************************	
//  Monitoring functions

int voidfun() {};

//! Generic UVar monitoring without callback
void
URBI::UMonitor(UVar &v)
{
  URBI::UMonitor(v,&voidfun);
}

//! UVar monitoring with callback
void 
URBI::UMonitor(UVar &v, int (*fun) ())
{  
  createUCallback("var",fun,v.get_name(), monitormap);
}

//! UVar monitoring with callback
void 
URBI::UMonitor(UVar &v, int (*fun) (UVar&))
{
  UGenericCallback* cb = createUCallback("var",fun,v.get_name(), monitormap);
  if (cb) cb->storage = (void*)(&v);
}


// **************************************************************************	
//! UObject constructor.
UObject::UObject(const string &s) :
  name(s)
{
  objectData = new UObjectData(this);  
  lastUObject = this;
}


//! UObject destructor.
UObject::~UObject()
{  
}

// This part is specific for standalone linux objects
// LIBURBI 'Module mode'

UCallbackAction
URBI::dispatcher(const UMessage &msg)
{
  //check message type
  if (msg.type != MESSAGE_DATA || msg.value->type != DATA_LIST) {
    msg.client.printf("Soft Device Error: unknown message content, type %d\n",(int)msg.type);
    return URBI_CONTINUE;
  }

  UList & array = *msg.value->list;

  if (array.size()<2) {
    msg.client.printf("Soft Device Error: Invalid number of arguments in the server message: %d\n",array.size());
    return URBI_CONTINUE;
  }
    
  if (array[0].type != DATA_DOUBLE) { 
	msg.client.printf("Soft Device Error: unknown server message type %d\n",(int)array[0].type);
	return URBI_CONTINUE;
  }
  
  if (array[0].type != DATA_DOUBLE) {
    msg.client.printf("Soft Device Error: unknown server message type %d\n",(int)array[0].type);
    return URBI_CONTINUE;
  }

 
  // UEM_ASSIGNVALUE
  if ((USystemExternalMessage)(int)array[0] == UEM_ASSIGNVALUE) {

    UVarTable::iterator varmapfind = varmap.find((string)array[1]);
    if (varmapfind != varmap.end()) {

      for (list<UVar*>::iterator it = varmapfind->second.begin();
	  it != varmapfind->second.end();
	  it++) 	    	      
	(*it)->__update(array[2]);	    
    } 

    UTable::iterator monitormapfind = monitormap.find((string)array[1]);
    if (monitormapfind != monitormap.end()) {

      for (list<UGenericCallback*>::iterator cbit = monitormapfind->second.begin();
	  cbit != monitormapfind->second.end();
	  cbit++) {
	// test of return value here
	UList u;
	u.array.push_back(new UValue());
	u[0].storage = (*cbit)->storage;
	(*cbit)->__evalcall(u);
      }
    }	 	  
  }
  
  // UEM_EVALFUNCTION
  else if ((USystemExternalMessage)(int)array[0] == UEM_EVALFUNCTION) {
    // For the moment, this iteration is useless since the list will contain
    // one and only one element. There is no function overloading yet and still
    // it would probably use a unique name identifier, hence a single element list again.	  
    if (functionmap.find((string)array[1]) != functionmap.end()) {	
      
      list<UGenericCallback*> tmpfun = functionmap[(string)array[1]];
      list<UGenericCallback*>::iterator tmpfunit = tmpfun.begin();
      array.setOffset(3);
      UValue retval = (*tmpfunit)->__evalcall(array);
      array.setOffset(0);
      URBI() << (string)array[2] << "=";
      retval.send(urbi::getDefaultClient()); //I'd rather not use << for bins
      URBI() << ";";
    }          
    else
      msg.client.printf("Soft Device Error: %s function unknown.\n",((string)array[1]).c_str());
  }
  
  // UEM_EMITEVENT
  else if ((USystemExternalMessage)(int)array[0] == UEM_EMITEVENT) {
  
    if (eventmap.find((string)array[1]) != eventmap.end()) {
      
      list<UGenericCallback*>  tmpfun = eventmap[(string)array[1]];
      for (list<UGenericCallback*>::iterator tmpfunit = tmpfun.begin();
	  tmpfunit != tmpfun.end();
	  tmpfunit++) {
	array.setOffset(2);
	(*tmpfunit)->__evalcall(array);
	array.setOffset(0);
      }
    
    }
  }
  
  // UEM_ENDEVENT
  else if ((USystemExternalMessage)(int)array[0] == UEM_ENDEVENT) {
    
    if (eventendmap.find((string)array[1]) != eventendmap.end()) {
         
      list<UGenericCallback*>  tmpfun = eventendmap[(string)array[1]];
      for (list<UGenericCallback*>::iterator tmpfunit = tmpfun.begin();
	  tmpfunit != tmpfun.end();
	  tmpfunit++) {
	array.setOffset(2);
	(*tmpfunit)->__evalcall(array);
	array.setOffset(0);
      }
    
    }
  }

  // DEFAULT
  else          
    msg.client.printf("Soft Device Error: unknown server message type number %d\n",(int)array[0]);      
  
  return URBI_CONTINUE;
}
 
UCallbackAction 
URBI::debug(const UMessage &msg)
{
  std::stringstream mesg;
  mesg<<msg;
  msg.client.printf("DEBUG: got a message  : %s\n",
	  mesg.str().c_str());
    
  return URBI_CONTINUE;  
}


void
URBI::main(int argc, char *argv[])
{ 
  // Retrieving command line arguments
  if (argc!=2) {
    cout << "usage: " << endl << argv[0] << " <URBI Server IP>" << endl;
    urbi::exit(0);
  }

  serverIP = argv[1];
  cout << "Running Soft Device Module '" << argv[0] << "' on " << serverIP << endl;
  urbi::connect(argv[1]);


  
#ifdef LIBURBIDEBUG
  urbi::getDefaultClient()->setWildcardCallback( callback (&debug));
#endif

  urbi::getDefaultClient()->setCallback(&dispatcher,
                                        externalModuleTag.c_str());
  
  for (list<baseURBIStarter*>::iterator retr = objectlist.begin();
       retr != objectlist.end();
       retr++)
    (*retr)->init();

  URBI() << externalModuleTag << ": [1,\"ball.x\",666]" << ";" ;
  URBI() << externalModuleTag << ": [1,\"ball.y\",\"hi!\"]" << ";" ;
  URBI() << externalModuleTag << ": [0,\"ball.myfun__2\",\"aa.__ret123\",42,\"hello\"]" << ";" ;
  URBI() << externalModuleTag << ": [0,\"ball.myfun__2\",\"aa.__ret124\",\"fff\",12]" << ";" ;
  URBI() << externalModuleTag << ": [2,\"ball.myevent__1\",123]" << ";" ;
}

