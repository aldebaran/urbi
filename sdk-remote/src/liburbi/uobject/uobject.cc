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

  hash_map<string,UVar* > varmap;
  UTable functionmap;
  UTable monitormap;
  UTable eventmap;

  UCallbackAction dispatcher(const UMessage &msg);
  UCallbackAction debug(const UMessage &msg);

  template <>
  UVar cast(UValue v)
  { 
    UVar result;
    result.set_value(v);
    result.set_name(v.associatedVarName);
    return result; 
  }
}

	
// **************************************************************************	
//! UGenericCallback constructor.
UGenericCallback::UGenericCallback(string type, string name, int size,  UTable &t) : 
  name(name) 
{
  t[this->name].push_back(this);
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
URBI::UMonitor(UVar v)
{
  URBI::UMonitor(v,&voidfun);
}

//! UVar monitoring with callback
void 
URBI::UMonitor(UVar v, int (*fun) ())
{  
  createUCallback("var",fun,v.get_name(), monitormap);
}

//! UVar monitoring with callback
void 
URBI::UMonitor(UVar v, int (*fun) (UVar))
{
  createUCallback("var",fun,v.get_name(), monitormap);
}


// **************************************************************************	
//! UObject constructor.
UObject::UObject(const string &s)
{
  name = s;
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
  if (msg.type == MESSAGE_LIST) {

    if (msg.listSize<2) 
      msg.client.printf("Soft Device Error: Invalid number of arguments in the server message: %d\n",msg.listSize);
    else {

      if (msg.listValue[0].type != MESSAGE_DOUBLE)
        msg.client.printf("Soft Device Error: unknown server message type %d\n",(int)msg.listValue[0].type);

      // UEM_ASSIGNVALUE
      else {
        if ((USystemExternalMessage)(int)msg.listValue[0] == UEM_ASSIGNVALUE) {
          
          if (varmap.find((string)msg.listValue[1]) != varmap.end()) {
  	    UVar* tmpvar = varmap[(string)msg.listValue[1]];
            tmpvar->__update(msg.listValue[2]);
	  }
          else
            msg.client.printf("Soft Device Error: %s var unknown.\n",((string)msg.listValue[1]).c_str());
        }
        
      // UEM_EVALFUNCTION
      else 
        if ((USystemExternalMessage)(int)msg.listValue[0] == UEM_EVALFUNCTION) {
          // For the moment, this iteration is useless since the list will contain
	  // one and only one element. There is no function overloading yet and still
	  // it would probably use a unique name identifier, hence a single element list again.	  
	  if (functionmap.find((string)msg.listValue[1]) != functionmap.end()) {
	  
            list<UGenericCallback*> tmpfun = functionmap[(string)msg.listValue[1]];
            list<UGenericCallback*>::iterator tmpfunit = tmpfun.begin();
	    UValue retval = (*tmpfunit)->__evalcall(msg.listSize<=3?0:&msg.listValue[3]);
             
            if (retval.type == MESSAGE_DOUBLE)
              URBI() << (string)msg.listValue[2] << " = " <<
                (double)retval << ";" << endl;
            else
              if (retval.type == MESSAGE_STRING)
                URBI() << (string)msg.listValue[2] << " = \"" <<
                  (string)retval << "\";" << endl;
          }          
          else
            msg.client.printf("Soft Device Error: %s function unknown.\n",((string)msg.listValue[1]).c_str());
        }

      // UEM_EMITEVENT
      else
        if ((USystemExternalMessage)(int)msg.listValue[0] == UEM_EMITEVENT) {
	  
          if (eventmap.find((string)msg.listValue[1]) != eventmap.end()) {
	    
	    list<UGenericCallback*>  tmpfun = eventmap[(string)msg.listValue[1]];
            for (list<UGenericCallback*>::iterator tmpfunit = tmpfun.begin();
	         tmpfunit != tmpfun.end();
		 tmpfunit++)
	      (*tmpfunit)->__evalcall(msg.listSize<=2?0:&msg.listValue[2]);
	  }
        }
      // DEFAULT
      else          
        msg.client.printf("Soft Device Error: unknown server message type number %d\n",(int)msg.listValue[0]);      
      } 
    
      return URBI_CONTINUE;
    }
  }
  
  msg.client.printf("Soft Device Error: unknown message content, type %d\n",(int)msg.type);
  return URBI_CONTINUE; 
}

UCallbackAction 
URBI::debug(const UMessage &msg)
{
  if (msg.type != MESSAGE_LIST)
    msg.client.printf("DEBUG: got a message at %d with tag %s : %s\n",
                      msg.timestamp, 
                      msg.tag,
                      msg.message);
  else
    msg.client.printf("DEBUG: got a message at %d with tag %s : list[%d]\n",
                      msg.timestamp, 
                      msg.tag,
                      msg.listSize);
    
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
  URBI() << externalModuleTag << ": [0,\"ball.myfun\",\"aa.__ret123\",42,\"hello\"]" << ";" ;
  URBI() << externalModuleTag << ": [0,\"ball.myfun\",\"aa.__ret124\",\"fff\",12]" << ";" ;
  URBI() << externalModuleTag << ": [2,\"ball.myevent\"]" << ";" ;
}
