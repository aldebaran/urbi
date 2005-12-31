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
  hash_map<string,UFunctionInitializer*> functionmap;

  UCallbackAction dispatcher(const UMessage &msg);
  UCallbackAction debug(const UMessage &msg);
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

    if (msg.listSize<3) 
      msg.client.printf("Soft Device Error: Invalid number of arguments in the server message: %d\n",msg.listSize);
    else {

      if (msg.listValue[0].type != MESSAGE_DOUBLE)
        msg.client.printf("Soft Device Error: unknown server message type %d\n",(int)msg.listValue[0].type);

      // UEM_ASSIGNVALUE
      else {
        if ((USystemExternalMessage)(int)msg.listValue[0] == UEM_ASSIGNVALUE) {
          
          if (UVar* tmpvar = varmap[(string)msg.listValue[1]])
            tmpvar->__update(msg.listValue[2]);
          else
            msg.client.printf("Soft Device Error: %s var unknown.\n",((string)msg.listValue[1]).c_str());
        }
        
      // UEM_EVALFUNCTION
      else 
        if ((USystemExternalMessage)(int)msg.listValue[0] == UEM_EVALFUNCTION) {
          
          if (UFunctionInitializer*  tmpfun = functionmap[(string)msg.listValue[1]]) {           
            /*
            UValue retval = tmpfun->__evalfunction(msg.listSize-3, &msg.listValue[2]); // que se passe-t-il lors du = ?
            // pas clair. Revoir le destructeur de UValue et l'operator=
             
            if (retval.type == MESSAGE_DOUBLE)
              URBI() << (string)msg.listValue[2] << " = " <<
                (double)retval << ";" << endl;
            else
              if (retval.type == MESSAGE_STRING)
                URBI() << (string)msg.listValue[2] << " = \"" <<
                  (string)retval << "\";" << endl;            
            */
          }
          else
            msg.client.printf("Soft Device Error: %s function unknown.\n",((string)msg.listValue[1]).c_str());
          
        }

      // UEM_EVALVALUE
      else
        if ((USystemExternalMessage)(int)msg.listValue[0] == UEM_EVALVALUE) {
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

  URBI() << externalModuleTag << ": [1,\"ball.x\",42]" << ";" ;
  URBI() << externalModuleTag << ": [0,\"ball.myfun\",\"aa.__ret123\",42,\"hello\"]" << ";" ;
}
