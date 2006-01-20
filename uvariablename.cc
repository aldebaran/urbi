/*! \file uvariablename.cc
 *******************************************************************************

 File: uvariablename.cc\n
 Implementation of the UVariableName class.

 This file is part of 
 %URBI Kernel, version __kernelversion__\n
 (c) Jean-Christophe Baillie, 2004-2005.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#include <math.h>

#include "uvariablename.h"
#include "ucommand.h"
#include "uconnection.h"
#include "udevice.h"
#include "userver.h"
#include "ucallid.h"
                                      
MEMORY_MANAGER_INIT(UVariableName);
// **************************************************************************	
//! UVariableName constructor for variable of the type device.id[...][...]...
UVariableName::UVariableName(UString* device, 
                     UString *id, 
                     bool rooted,
                     UNamedParameters *index)
{	
  ADDOBJ(UVariableName);	

  this->device    = device;
  this->id        = id;
  this->method    = 0; 
  this->rooted    = rooted;
  this->index     = index;
  this->fullname_ = 0;
  this->str       = 0;
  this->isstatic  = false;
  this->isnormalized = false;
  this->deriv     = UNODERIV;
  this->varerror  = false;
  this->cached    = false;
  this->fromGroup = false;  
  this->variable  = 0; 
  this->function  = 0;
  this->firsttime = true;
  this->nostruct  = false;

  localFunction   = false;
  selfFunction    = false;
  if ((device) && (device->equal("__Funct__")))
    localFunction = true;
  if ((device) && (device->equal("self")))
    selfFunction = true;

}

//! UVariableName constructor for string based variables: $("...")
UVariableName::UVariableName(UExpression* str, bool rooted)
{	
  ADDOBJ(UVariableName);	

  this->device    = 0;
  this->id        = 0;
  this->method    = 0; 
  this->rooted    = rooted;
  this->index     = 0;
  this->fullname_ = 0;
  this->str       = str;
  this->isstatic  = false;  
  this->isnormalized = false;
  this->deriv     = UNODERIV;
  this->varerror  = false;
  this->cached    = false;  
  localFunction   = false; 
  selfFunction    = false;
  this->variable  = 0;
  this->function  = 0;
  this->firsttime = true;
  this->nostruct  = false;

}

//! UVariableName destructor.
UVariableName::~UVariableName()
{
  FREEOBJ(UVariableName);	
  if (device) delete(device);
  if (id) delete (id);
  if (method) delete (method);
  if (fullname_) delete (fullname_);
  if (str) delete (str);
  if (index) delete (index);
}


//! UVariableName reset cache access
void
UVariableName::resetCache() 
{
  cached = false;
  variable = 0;
  if (fullname_) delete fullname_;
  fullname_ = 0;
}

//! UVariableName access to variable (with cache)
/*! If variable is not null, it means that the variable name is
    constant and that the access to the variable hash table has been done
    already. This access is then cached to limitate the number of calls to
    the hash table.
*/
UVariable* 
UVariableName::getVariable(UCommand *command, UConnection *connection)
{
  UVariable *tmpvar;

  if (variable) 
    if (variable->toDelete) return(0);
  else
    return (variable);

  if ((!fullname_) || (!cached))
    buildFullname(command,connection);

  if (!fullname_) return(0);

  if ( (hmi2 = ::urbiserver->variabletab.find(fullname_->str())) !=
       ::urbiserver->variabletab.end())
    tmpvar = (*hmi2).second;
  else 
    tmpvar = 0;
  
  if (cached) variable = tmpvar;

  return (tmpvar);
}

//! UVariableName access to function (with cache)
/*! If function is not null, it means that the function name is
    constant and that the access to the function hash table has been done
    already. This access is then cached to limitate the number of calls to
    the hash table.
*/
UFunction* 
UVariableName::getFunction(UCommand *command, UConnection *connection)
{
  UFunction *tmpfun;

  if (function) return (function);

  if ((!fullname_) || (!cached))
    buildFullname(command,connection);

  if (!fullname_) return(0);

  if ( (hmf = ::urbiserver->functiontab.find(fullname_->str())) !=
       ::urbiserver->functiontab.end())
    tmpfun = (*hmf).second;
  else 
    tmpfun = 0;
  
  if (cached) function = tmpfun;

  return (tmpfun);
}

//! UVariableName access to device (with cache)
UString* 
UVariableName::getDevice()
{
  if (device) return (device);

  if (!fullname_) return (0);
  char *pointPos = strstr(fullname_->str(),".");
  if (pointPos == 0) return (fullname_);     
  pointPos[0] = 0;
 
  device = new UString(fullname_->str());
  pointPos[0] = '.';
  return (device);
}

//! UVariableName access to method (with cache)
UString* 
UVariableName::getMethod()
{
  if (method) return (method);
 
  if (!fullname_) return (0);
  char *pointPos = strstr(fullname_->str(),".");
 
  if (pointPos == 0) 
    method = new UString("");
  else
    method = new UString(pointPos + 1);
  return(method);
}

//! UVariableName access to dev
UDevice* 
UVariableName::getDev(UCommand *command, UConnection *connection)
{
  HMdevicetab::iterator hmi;

  if (!variable) {
    
    UString* dev = getDevice();
    if (!dev) return(0);

    if ((hmi = ::urbiserver->devicetab.find(dev->str())) !=
        ::urbiserver->devicetab.end()) 
      return( (*hmi).second);    
    else
      return(0);
  }
  else
    return (variable->dev);
}


//! UVariableName name extraction, witch caching
/*! This method builds the name of the variable (or function) and stores it in fullname_.
    If the building blocks are static, non variable parameters (like static 
    indexes in an array or constant string in a $(...)), cached is set to
    true to avoid recalculus on next call.
*/
UString* 
UVariableName::buildFullname(UCommand *command, UConnection *connection, bool withalias)
{
  int    fullnameMaxSize = 1024;
  char   name[fullnameMaxSize];
  char   indexstr[fullnameMaxSize];
  UValue *e1;
  bool   errordetected;
  UNamedParameters* itindex;
      
  if (cached) return (fullname_);

  if (str) {

    e1 = str->eval(command,connection);
    cached = str->isconst;

    if ((e1==0) || (e1->str==0) || (e1->dataType != DATA_STRING)) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! dynamic variable evaluation failed\n");
      connection->send(tmpbuffer,command->tag->str());
      if (e1) delete e1;      
      if (fullname_) {
        delete fullname_;
        fullname_ = 0;
      }
      return(0);
    }
    
    if (strchr(e1->str->str(),'.') == 0) {
      if (connection->stack.empty())
        snprintf(name,fullnameMaxSize,
                 "%s.%s",connection->connectionTag->str(),
                 e1->str->str());
      else
        snprintf(name,fullnameMaxSize,
                 "%s.%s",connection->stack.front()->str(),
                 e1->str->str());
    }
    else      
      strncpy(name,e1->str->str(),fullnameMaxSize);
   
    delete e1;        
  }
  else {

    // Local function call
    if ((localFunction || selfFunction) && (firsttime)) {
      firsttime = false;
      if (!connection->stack.empty()) {
        UCallid *funid = connection->stack.front();
        if (funid) {
	  if (localFunction)
	    device->update(funid->str());
	  if (selfFunction)
	    device->update(funid->self());	    
	}
      }
      else {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "!!! invalid prefix resolution\n");
        connection->send(tmpbuffer,command->tag->str());  
        if (fullname_) {
          delete fullname_;
          fullname_ = 0;
        }
        return(0);                    
      }        
    }
      
    if (device->equal("local"))
      device->update(connection->connectionTag->str());

    if (index == 0) {    

      cached = true;

      // Create the concatened variable name      
      snprintf(name,fullnameMaxSize,"%s.%s",device->str(),id->str());       
    }
    else { 
      // rebuilding name based on index
      
      snprintf(name,fullnameMaxSize,"%s.%s",device->str(),id->str());      
      itindex = index;
      errordetected = false;
      cached = true;

      while (itindex) {
        
        e1 = itindex->expression->eval(command,connection);  
        if (e1==0) {
          snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                   "!!! expression error: array ignored\n");
          connection->send(tmpbuffer,command->tag->str());
          errordetected = true;
          break;   
        }
        if (e1->dataType == DATA_NUM) 
          snprintf(indexstr,fullnameMaxSize-strlen(name),"__%d",(int)e1->val);      
        else         
          if (e1->dataType == DATA_STRING) 
            snprintf(indexstr,fullnameMaxSize-strlen(name),"__%s",(int)e1->str->str());
          else {
            
            delete e1;
            snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                     "!!! invalid array type: array ignored\n");
            connection->send(tmpbuffer,command->tag->str());
            errordetected = true;
            break;
          }
        
        // Suppress this to make index non static by default
        // if (!itindex->expression->isconst) cached = false;

        strcat(name,indexstr);
        itindex = itindex->next;
        delete e1;
      } 
      if (errordetected) {        
        if (fullname_) {
          delete fullname_;
          fullname_ = 0;
        }
        return(0);
      }
    }
  } // else str


  // Alias updating
  if (withalias) {
/*
  
    hmi = ::urbiserver->aliastab.find(name);      
    past_hmi = hmi;
    
    while  (hmi != ::urbiserver->aliastab.end()) {      
      past_hmi = hmi;
      hmi = ::urbiserver->aliastab.find((*hmi).second->str());          
    };
    
    if (past_hmi != ::urbiserver->aliastab.end()) {
      strncpy(name, (*past_hmi).second->str(), fullnameMaxSize);                   		
      if (device) delete(device); device = 0;
      if (method) delete(method); method = 0; // forces recalc of device.method
      if (variable) variable = 0;
    }
    */
  }

  if (fullname_) fullname_->update(name);
  else fullname_ = new UString(name);

  return (fullname_);
}


//! UVariableName name update for functions scope hack
void 
UVariableName::nameUpdate(const char* _device, const char* _id)
{
  const int    fullnameMaxSize = 1024;
  char   name[fullnameMaxSize];

  if (device)
    device->update(_device);
  else device = new UString(_device);
  
  if (id)
    id->update(_id);
  else id = new UString(_id);  
}

//! UNamedParameters hard copy function
UVariableName*
UVariableName::copy()
{
  UString*  copy_device;
  UString*  copy_id;
  UNamedParameters*  copy_index;
  UVariableName *ret;
  
  if (id)     copy_id = new UString(id); else copy_id = 0;
  if (device) copy_device = new UString(device); else copy_device = 0;
  if (index) copy_index = index->copy(); else copy_index = 0;

  if (str==0)
    ret = new UVariableName(copy_device,copy_id,rooted,copy_index);  
  else
    ret = new UVariableName(str->copy(),rooted);  
  
  ret->isstatic     = isstatic;
  ret->isnormalized = isnormalized;
  ret->deriv        = deriv;
  ret->varerror     = varerror;
  ret->fromGroup    = fromGroup;

  return (ret);
}

//! Print the variable
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UVariableName::print()
{
  ::urbiserver->debug("(VAR root=%d ",rooted);
  if (device) ::urbiserver->debug("device='%s' ",device->str());
  if (id) ::urbiserver->debug("id='%s' ",id->str());
  ::urbiserver->debug(") ");
}
