/*! \file uexpression.cc
 *******************************************************************************

 File: uexpression.cc\n
 Implementation of the UExpression class.

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
#include <stdio.h>
#include <sstream>
#ifdef _MSC_VER
#define snprintf _snprintf
#endif
#include "uexpression.h"
#include "ucommand.h"
#include "uconnection.h"

#include "userver.h"

MEMORY_MANAGER_INIT(UExpression);
// **************************************************************************
//! UExpression base constructor called by every specific constructor.
void UExpression::initialize()
{
  ADDOBJ(UExpression);

  expression1      = 0;
  expression2      = 0; 
  str              = 0;
  id               = 0;
  variablename     = 0;
  parameters       = 0;
  firsteval        = true;
  isconst          = false;
  issofttest       = false;
  staticcache      = 0;
  tmp_value        = 0;

  this->softtest_time = 0; 
  
  dataType  = DATA_UNKNOWN;
}
	
//! UExpression constructor for numeric value.
/*! The parameter 'type' is required here only for the sake of uniformity
    between all the different constructors.
*/
UExpression::UExpression(UExpressionType type, ufloat *val) 
{	
   initialize();
  
   this->val  = *val;
   delete val; //other steal pointer, we copy->destroy
   this->type = type; // should be EXPR_VALUE
   this->isconst = true;
  
   dataType   = DATA_NUM;
}
		
			
//! UExpression constructor for numeric value.
/*! The parameter 'type' is required here only for the sake of uniformity
    between all the different constructors.
*/
UExpression::UExpression(UExpressionType type, ufloat val) 
{	
   initialize();
  
   this->val  = val;
   this->type = type; // should be EXPR_VALUE
   this->isconst = true;
  
   dataType   = DATA_NUM;
}


//! UExpression constructor for string value.
/*! The parameter 'type' is required here only for the sake of uniformity
    between all the different constructors.
*/
UExpression::UExpression(UExpressionType type, UString *str) 
{	
  initialize();
  this->str  = str;
  this->type = type; // should be EXPR_VALUE
  this->isconst = true;
  dataType   = DATA_STRING;
}
	
//! UExpression constructor for numeric value.
/*! The parameter 'type' is required here only for the sake of uniformity
    between all the different constructors.
*/
UExpression::UExpression(UExpressionType type, UValue *v) 
{	
   initialize();
    
   this->type = type; // should be EXPR_VALUE
   this->isconst = true;
   dataType   = v->dataType;

   if (v->dataType == DATA_NUM) this->val  = v->val;
   else
     if (v->dataType == DATA_STRING) this->str = v->str->copy();
     else {
       tmp_value = v;
       dataType = DATA_UNKNOWN;  
     }
}
		
//! UExpression constructor for functors 
/*! The parameter 'type' is required here only for the sake of uniformity
    between all the different constructors.
*/
UExpression::UExpression(UExpressionType type, 
                         UString *oper, 
                         UVariableName *variablename) 
{	
  initialize();
  this->str          = oper;
  this->variablename = variablename;
  if (variablename)
    this->isconst    = variablename->isstatic;

  this->type       = type; // should be EXPR_PROPERTY
  dataType         = DATA_UNKNOWN;
}	       
		
//! UExpression constructor for variable
/*! The parameter 'type' is required here only for the sake of uniformity
    between all the different constructors.
*/
UExpression::UExpression(UExpressionType type, UVariableName* variablename) 
{	
  initialize();
  this->type     = type; // should be EXPR_VARIABLE or EXPR_ADDR_VARIABLE or EXPR_GROUPLIST
  /*if (variablename)
    dataType = variablename->dataType;
  */
  if (type == EXPR_ADDR_VARIABLE) dataType = DATA_STRING;
  this->variablename = variablename;
}
	
//! UExpression constructor for function
/*! The parameter 'type' is required here only for the sake of uniformity
    between all the different constructors.
*/
UExpression::UExpression(UExpressionType type,
                         UVariableName* variablename, 
                         UNamedParameters *parameters) 
{	
  initialize();
  this->type         = type; // should be EXPR_FUNCTION  
  dataType           = DATA_UNKNOWN;
  this->variablename = variablename;
  this->parameters   = parameters;
}	

//! UExpression constructor for function
/*! The parameter 'type' is required here only for the sake of uniformity
    between all the different constructors.
*/
UExpression::UExpression(UExpressionType type,                        
                         UNamedParameters *parameters) 
{	
  initialize();
  this->type         = type; // should be EXPR_LIST
  dataType           = DATA_LIST;
  this->parameters   = parameters;
}

//! UExpression constructor for composed operation
UExpression::UExpression(UExpressionType type, 
                         UExpression* expression1, 
                         UExpression* expression2)
{	
  initialize();
  this->type        = type;

  this->expression1 = expression1;
  this->expression2 = expression2;

  // Compile time calculus reduction

  if ((expression1) && (expression2))
    if (( expression1->type == EXPR_VALUE ) &&
        ( expression1->dataType == DATA_NUM ) &&
        ( expression2->type == EXPR_VALUE ) &&
        ( expression2->dataType == DATA_NUM ) && 
        ( (type == EXPR_PLUS) || (type == EXPR_MINUS) || 
          (type == EXPR_MULT) || (type == EXPR_DIV) ||
          (type == EXPR_EXP) ) &&
        ( ! ( (type == EXPR_DIV) && (expression2->val == 0)) ) ) {
      
      switch (type) {
      case EXPR_PLUS:  val = expression1->val + expression2->val; break;
      case EXPR_MINUS: val = expression1->val - expression2->val; break;
      case EXPR_MULT:  val = expression1->val * expression2->val; break;
      case EXPR_DIV:   val = expression1->val / expression2->val; break;
      case EXPR_EXP:   val = pow (expression1->val , expression2->val); break;    
      }

      this->type = EXPR_VALUE;
      this->isconst = true;
      dataType   = DATA_NUM;
      delete expression1; this->expression1 = 0;
      delete expression2; this->expression2 = 0;
    }

  if ( ( type == EXPR_NEG ) &&
       ( expression1 ) )
    if ( ( expression1->type == EXPR_VALUE ) &&
         ( expression1->dataType == DATA_NUM ) ) { 

      val = - expression1->val;

      this->type = EXPR_VALUE;
      this->isconst = true;
      dataType   = DATA_NUM;
      delete expression1; this->expression1 = 0;
    }   
}

//! UExpression destructor.
UExpression::~UExpression()
{ 
  FREEOBJ(UExpression);

  if (str) delete(str);
  if (id) delete(id);
  if (expression1)  delete expression1;
  if (expression2)  delete expression2;
  if (variablename) delete variablename;
  if (parameters)   delete parameters;
} 
 
//! UExpression hard copy function
UExpression*
UExpression::copy() 
{
  UExpression* ret = new UExpression(type,ufloat(0));  

  if (expression1)  ret->expression1 = expression1->copy(); 
  if (expression2)  ret->expression2 = expression2->copy(); 
  if (variablename) ret->variablename = variablename->copy(); 
  if (parameters)   ret->parameters = parameters->copy(); 
  if (str)          ret->str = new UString(str);
  if (id)           ret->id  = new UString(id);
  if (softtest_time) ret->softtest_time = softtest_time->copy();
  if (staticcache)  ret->staticcache = staticcache->copy();
  
  ret->dataType = dataType;
  ret->val      = val;
  ret->isconst  = isconst;
  ret->issofttest  = issofttest;
  ret->firsteval   = firsteval;
  ret->tmp_value   = tmp_value;

  return (ret);
}

//! Print the expression
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UExpression::print()
{
  ::urbiserver->debug("[Type:E%d ",type);
  if (isconst) ::urbiserver->debug("(const) ");
  if ((type == EXPR_VALUE) && (dataType == DATA_NUM)) {
    std::ostringstream tstr;
    tstr << "val="<<val<<" ";
    ::urbiserver->debug(tstr.str().c_str());
  }
  if (str) ::urbiserver->debug("str='%s' ",str->str());
  if (id) ::urbiserver->debug("id='%s' ",id->str());
  if (expression1) {::urbiserver->debug("expr1="); expression1->print(); ::urbiserver->debug(" ");}
  if (expression2) {::urbiserver->debug("expr2="); expression2->print(); ::urbiserver->debug(" ");}
  if (variablename){::urbiserver->debug("variablename="); variablename->print(); ::urbiserver->debug(" ");}
  if (parameters)  {::urbiserver->debug("parameters={"); parameters->print(); ::urbiserver->debug("} ");}
  ::urbiserver->debug("] ");  
}

//! UExpression evaluation.
/*! The connection parameter is necessary to access the variable hash table for
    expressions who contain variables. It is also used to display error 
    messages.
    The UCommand is used to retrieve a message tag if necessary.
*/
UValue*
UExpression::eval(UCommand *command, UConnection *connection, bool silent)
{
  const int errSize = 256;
  static char errorString[errSize]; // Max error message = 256 chars  
  UValue *e1;
  UValue *e2;
  UValue *e3;
  UValue *e4;
  UValue *ret;
  UValue *value;
  UVariable *variable;
  UString* method;
  UString* devicename;
  ufloat d1,d2;
  UCommand_EMIT* cmd;
  const char* vnamestr;
  UNamedParameters *pevent;
  UNamedParameters *pcatch;
  UString* funname;
  HMfunctiontab::iterator hmf; 
  HMgrouptab::iterator retr;
  list<UString*>::iterator it;
  UExpression *e;

  if ((issofttest) &&
      (softtest_time)) {
    
    UValue *ret = softtest_time->eval(command, connection);
    if (ret)
      softtest_time->val = ret->val;
    else 
      softtest_time->val = 0;
    delete ret;
  }

  switch (type) {

  case EXPR_LIST:

    ret = new UValue();
    ret->dataType = DATA_LIST;
    pevent = parameters;    
    e1 = ret;
    if (pevent) {
      e1->liststart = pevent->expression->eval(command, connection);
      e1 = e1->liststart;
      pevent = pevent->next;
    }
    
    while (pevent) {
      
      e1->next = pevent->expression->eval(command, connection);
      if (e1->next==0) {
        delete ret;
        return(0);
      }
      pevent = pevent->next;
      e1 = e1->next;
    }
    return(ret);

  case EXPR_GROUP: 
        
    ret = new UValue(); 
    retr = connection->server->grouptab.find(str->str());
    if (retr !=  connection->server->grouptab.end()) {
      
      ret->dataType = DATA_LIST;
      
      it = (*retr).second->members.begin();
      if (it !=  (*retr).second->members.end()) {

	e = new UExpression (EXPR_GROUP, (*it)->copy());
	e2 = e->eval(command, connection);
	delete e;
	if (e2->dataType == DATA_VOID) {
	  ret->liststart = new UValue((*it)->str());
	  delete e2;
	}
	else {
	  delete ret;
	  ret = e2;
	}
	  
	e1 = ret->liststart;
	while (e1->next) e1 = e1->next;
	it++;
      }
      
      while (it !=  (*retr).second->members.end()) {
	
	e = new UExpression (EXPR_GROUP, (*it)->copy());
	e2 = e->eval(command, connection);
	delete e;
	if (e2->dataType == DATA_VOID) {
	  e1->next = new UValue((*it)->str());
	  delete e2;
	  e1 = e1->next;
	}
	else {
	  e3 = e2;
	  e2 = e2->liststart;
	  while (e2) {
	    e1->next = e2->copy();
	    e1 = e1->next;
	    e2 = e2->next;
	  }
	  delete e3;
	}
	it++;
      }
    }  
    return(ret);
    
  case EXPR_VALUE:

    if (tmp_value) return tmp_value; // hack to be able to handle complex return types from function calls

    ret = new UValue();
    ret->dataType = dataType;
    if (dataType == DATA_NUM) ret->val = val; 
    if (dataType == DATA_STRING) ret->str = new UString(str);    
    return(ret);

  case EXPR_ADDR_VARIABLE:

    ret = new UValue();
    ret->dataType = DATA_STRING;    
    // hack here to be able to use objects pointeurs
    
    ret->str = new UString (variablename->buildFullname(command,connection)); 
    return(ret);

  case EXPR_VARIABLE: 

    variable = variablename->getVariable(command,connection); 
    if (!variablename->getFullname()) return (0); 
    method = variablename->getMethod();
    devicename = variablename->getDevice();
 	
    const char* varname; 
    if (!variable) {
    	
      varname = variablename->getFullname()->str();
      if (::urbiserver->eventtab.find(varname) !=
          ::urbiserver->eventtab.end()) {
        // this is an event
       
        ret = new UValue(ufloat(1));
        ret->val = 1;
        ret->eventid = ::urbiserver->eventtab[variablename->getFullname()->str()]->eventid;
        return(ret);
      }

      // virtual variables
      const char* devname = variablename->getDevice()->str();
      bool ambiguous;
      UVariable *vari = 0;
      HMobjtab::iterator itobj;
      if ((itobj = ::urbiserver->objtab.find(devname)) !=
	  ::urbiserver->objtab.end()) {
	vari = itobj->second->searchVariable(variablename->getMethod()->str(),
    	    ambiguous);
	if (ambiguous)  {  
	  snprintf(errorString,errSize,"!!! Ambiguous multiple inheritance on variable %s\n", 
	      variablename->getFullname()->str());
	  connection->send(errorString,command->tag->str()); 
          return new UValue();	            
	}
	
	variable = vari;     
	if (vari) {
  	  devicename->update(vari->method);
	  variablename->device->update(vari->method);
	  variablename->buildFullname(command,connection); 
	}
      }
    }
      
    if (!variable) {

      char* p = strstr(varname,"__");
      char* pnext = p;
      while (pnext) {
	p = pnext;
	pnext = strstr(p+2,"__");
      }
      char* p2;
      if (p) { // could be a list index.... (dirty hack)
	p[0]=0;

	HMvariabletab::iterator hmv = ::urbiserver->variabletab.find(varname);
	if (hmv != ::urbiserver->variabletab.end()) {
	  
	  UVariable* tmpvar = (*hmv).second;
	  if (tmpvar->value->dataType == DATA_LIST) {
	    // welcome to C-string hacking grand master area
	    // Don't let unaccompagnied children see this.
	    UValue* xval = tmpvar->value->liststart;
	    int index;
	    int curr;	    
	    p[0]='_';
	    p=p+2; // beginning of the index
	    p2 = strchr(p,'_');
	    while (p) {
	      if (p2) p2[0]=0;
	      index = atoi(p);
	      curr=0;
	      while ((curr!=index) && (xval)) {
		xval = xval->next;
		curr++;
	      }
	      if (!xval) {		
	        snprintf(errorString,errSize,"!!! Index out of range\n");
		connection->send(errorString,command->tag->str());
	        return new UValue();
	      }
	      else {
		if (p2) {
		  if (xval->dataType != DATA_LIST) {  
		    snprintf(errorString,errSize,"!!! Invalid index usage\n");
   		    connection->send(errorString,command->tag->str());
	            return new UValue();
		  }
		  else
		    xval = xval->liststart;
		}
	      }

	      if (p2) p2[0]='_';
	      if (p2) p = p2+2; else p=0;
	      if (p) p2 = strchr(p,'_');
	    }
	    
	    return xval->copy();
	  }
	}
	p[0]='_';
      }
      
      snprintf(errorString,errSize,"!!! Unknown identifier: %s\n",
	       variablename->getFullname()->str());     
      
      if (!silent)
	    connection->send(errorString,command->tag->str());
      
      return new UValue();
      
    }
 
    if ((!variablename->isstatic) || (firsteval)) {
      ret = variable->get()->copy();

      // error evaluation for variables (target-val)
      if ((variablename->varerror) && (variable->value->dataType == DATA_NUM)) {
        ret->val = variable->previous - ret->val;
      }
      
      // normalized variables
      if ((variablename->isnormalized) && (variable->rangemax != variable->rangemin)) {
                
        if ((variable->rangemin == -UINFINITY) ||
            (variable->rangemax ==  UINFINITY) || 
            (variable->value->dataType != DATA_NUM)) {

          snprintf(errorString,errSize,
                   "!!! Impossible to normalize: no range defined for variable %s\n",
                   variablename->getFullname()->str());     
      
          connection->send(errorString,command->tag->str());      
          return 0;
        }

        ret->val = (ret->val - variable->rangemin) / 
          (variable->rangemax - variable->rangemin);             
      }
      
      if (variablename->deriv != UNODERIV) {

        if (variable->autoUpdate) {
          if (variablename->deriv == UTRUEDERIV)  variablename->deriv = UDERIV;
          if (variablename->deriv == UTRUEDERIV2) variablename->deriv = UDERIV2;          
        }

        switch (variablename->deriv) {

          case UDERIV: ret->val = 1000. * (variable->previous - variable->previous2)/
            (::urbiserver->previousTime - ::urbiserver->previous2Time);
            break;
          case UDERIV2: ret->val = 1000000. * 2 * 
              ( variable->previous  * (::urbiserver->previous2Time - ::urbiserver->previous3Time) -
                variable->previous2 * (::urbiserver->previousTime  - ::urbiserver->previous3Time) +
                variable->previous3 * (::urbiserver->previousTime  - ::urbiserver->previous2Time)
                ) / (  (::urbiserver->previous2Time - ::urbiserver->previous3Time) *
                       (::urbiserver->previousTime  - ::urbiserver->previous3Time) *
                       (::urbiserver->previousTime  - ::urbiserver->previous2Time) );
                
            break;
        case UTRUEDERIV: ret->val = 1000. * (variable->get()->val - variable->valPrev)/
              (::urbiserver->currentTime - ::urbiserver->previousTime);
            break;
          case UTRUEDERIV2: ret->val = 1000000. * 2 * 
              ( variable->get()->val  * (::urbiserver->previousTime - ::urbiserver->previous2Time) -
                variable->valPrev     * (::urbiserver->currentTime  - ::urbiserver->previous2Time) +
                variable->valPrev2    * (::urbiserver->currentTime  - ::urbiserver->previousTime)
                ) / (  (::urbiserver->previousTime - ::urbiserver->previous2Time) *
                       (::urbiserver->currentTime  - ::urbiserver->previous2Time) *
                       (::urbiserver->currentTime  - ::urbiserver->previousTime) );
            break;
        }            
      }
    }
    
    // static variables
    if (variablename->isstatic) 
      if (firsteval) {
        firsteval = false;
        staticcache = ret->copy();
        if (!staticcache)  return (0); 
      }
      else
        ret = staticcache->copy();
      
    return(ret);
  /*
  case EXPR_GROUPLIST:
    variable = variablename->getVariable(command,connection); 
    if (!variablename->getFullname()) return (0); 
    method = variablename->getMethod();
    devicename = variablename->getDevice();

    if (::urbiserver->grouptab.find(devicename->str()) ==
	::urbiserver->grouptab.end()
	|| ::urbiserver->grouptab[devicename->str()]->members.empty()
	) {

      snprintf(errorString,errSize,"!!! Not a group: %s\n",
	       devicename->str());     
      
      if (!silent)
	connection->send(errorString,command->tag->str());
      
      return 0;
    }
    
    ret = ::urbiserver->grouptab[devicename->str()]->list(variablename);
    return ret;
    */




  case EXPR_PROPERTY:

    variable = variablename->getVariable(command,connection);  
    if (!variablename->getFullname()) return (0);
    if (!variable) {

      snprintf(errorString,errSize,"!!! Unknown identifier: %s\n",
               variablename->getFullname()->str());     
      
      connection->send(errorString,command->tag->str());
      return 0;
    }
    method = variablename->getMethod();
    devicename = variablename->getDevice();

    ret = new UValue();

    if (strcmp(str->str(),"rangemin")==0) {
      
      ret->dataType = DATA_NUM;
      ret->val = variable->rangemin;
      return(ret);
    }

    if (strcmp(str->str(),"rangemax")==0) {
      
      ret->dataType = DATA_NUM;
      ret->val = variable->rangemax;
      return(ret);
    }

    if (strcmp(str->str(),"speedmin")==0) {
      
      ret->dataType = DATA_NUM;
      ret->val = variable->speedmin;
      return(ret);
    }

    if (strcmp(str->str(),"speedmax")==0) {
      
      ret->dataType = DATA_NUM;
      ret->val = variable->speedmax;
      return(ret);
    }

    if (strcmp(str->str(),"delta")==0) {
      
      ret->dataType = DATA_NUM;
      ret->val = variable->delta;
      return(ret);
    }

    if (strcmp(str->str(),"unit")==0) {
      
      ret->dataType = DATA_STRING;
      ret->str = new UString( variable->unit );
      return(ret);
    }
    
    if (strcmp(str->str(),"blend")==0) {
      
      ret->dataType = DATA_STRING;
      switch (variable->blendType) {
        case UNORMAL:  ret->str = new UString("normal"); break;
        case UMIX:     ret->str = new UString("mix"); break;
        case UADD:     ret->str = new UString("add"); break;
        case UDISCARD: ret->str = new UString("discard"); break;
        case UQUEUE:   ret->str = new UString("queue"); break;
        case UCANCEL:  ret->str = new UString("cancel"); break;
        default: ret->str = new UString("unknown");
      }
      return (ret);
    }
    snprintf(errorString,errSize,"!!! Unknown property: %s\n",
             str->str());     
      
    connection->send(errorString,command->tag->str());
    return 0;


  case EXPR_FUNCTION:
    
    vnamestr = variablename->buildFullname(command,connection)->str();    

    // Event detection
    if (::urbiserver->eventtab.find(vnamestr) !=
        ::urbiserver->eventtab.end()) {
      // this is an event

      cmd = ::urbiserver->eventtab[vnamestr];   

      if ((!cmd->parameters) && (!parameters)) { 
        ret = new UValue(ufloat(1));
        ret->eventid = cmd->eventid;
        ret->val = 1;

        return(ret);
      }

      if ( ((cmd->parameters) && (parameters)) &&      
           (cmd->parameters->size() == parameters->size())) { // to optimize...
        
        pevent = cmd->parameters;
        pcatch = parameters;
        while ((pevent) && (pcatch)) {
          
          e1 = pevent->expression->eval(command, connection);
          if (e1==0) return(0);
          if (pcatch->expression->type == EXPR_VARIABLE) {
            variable = pcatch->expression->variablename->getVariable(command,connection); 
            if (!variable) 
              variable = new UVariable(pcatch->expression->variablename->buildFullname(command,connection)->str(),e1->copy());
            else {
              if ((variable->value->dataType == DATA_STRING) &&
                  (variable->value->str))
                delete variable->value->str;
              variable->value->dataType = DATA_VOID;
              variable->set(e1);
            }
          }
          else {
            e2 = pcatch->expression->eval(command, connection);
            if (!e1->equal(e2)) return(0);
            delete e2;
          }
            
          delete e1;
          pevent = pevent->next;
          pcatch = pcatch->next;
        }

        ret = new UValue(ufloat(1));
        ret->eventid = (long)cmd;
        ret->val = 1;

        return(ret);
      }
    }
    
    if ( /*(strcmp(variablename->device->str(),
           connection->connectionTag->str()) == 0) &&*/
         (parameters == 0) &&
         ( (variablename->id->equal("freemem")) || 
           (variablename->id->equal("power")) ||
           (variablename->id->equal("cpuload")) ||
           (variablename->id->equal("time")) ) ) {
      
      ret = new UValue();
      ret->dataType = DATA_NUM;

      if (strcmp(variablename->id->str(),"freemem")==0)
        ret->val = availableMemory - usedMemory;

      if (strcmp(variablename->id->str(),"time")==0)
        ret->val = connection->server->getTime();
 
      if (strcmp(variablename->id->str(),"cpuload")==0)
        ret->val = connection->server->cpuload;

      if (strcmp(variablename->id->str(),"power")==0) {
        ret->val = connection->server->getPower();        
      }
            
      return(ret);
    }

    if ( /*(strcmp(variablename->device->str(),
           connection->connectionTag->str()) == 0) &&*/
         (parameters!=0) &&
         (parameters->size() == 2)) {

      if (strcmp(variablename->id->str(),"save")==0) {
        
        e1 = parameters->expression->eval(command,connection);
        e2 = parameters->next->expression->eval(command,connection);

        if (e1==0) return 0;  
        if (e2==0) { delete e1; return 0;}
        if ((e1->dataType != DATA_STRING) ||
            (e2->dataType != DATA_STRING)) {
          delete e1;
          delete e2;
          return 0;
        }
        ret = new UValue();
        ret->dataType = DATA_VOID;

        // save to file

        if (connection->server->saveFile(e1->str->str(),
                                         e2->str->str()) == UFAIL) {
          
          snprintf(errorString,errSize,
                   "!!! Cannot save to the file %s\n",e1->str->str());
          connection->send(errorString,command->tag->str());
          delete ret;
          ret = 0;            
        }

        delete e1; 
        delete e2; 
        return(ret);
      } // save

      if (strcmp(variablename->id->str(),"getIndex")==0) {

	e1 = parameters->expression->eval(command,connection);
	e2 = parameters->next->expression->eval(command,connection);

	if (e1==0) return 0;
	if (e2==0) { delete e1; return 0;}
	if ((e1->dataType != DATA_LIST) ||
	    (e2->dataType != DATA_NUM)) {
	  delete e1;
	  delete e2;
	  return 0;
	}

	e3 = e1->liststart;
	int indx = 0;
	while ((e3) && (indx!=(int)e2->val)) {
	  e3 = e3->next;
	  indx++;
	}
	if (!e3) {
	 
          snprintf(errorString,errSize,
                   "!!! Index out of range\n");
          connection->send(errorString,command->tag->str());
          ret = 0; 
	}
	else
	  ret = e3->copy();

	delete e1;
	delete e2;
	return(ret);
      } // getIndex

      if (strcmp(variablename->id->str(),"cat")==0) {

	e1 = parameters->expression->eval(command,connection);
	e2 = parameters->next->expression->eval(command,connection);

	if (e1==0) return 0;
	if (e2==0) { delete e1; return 0;}
	if ((e1->dataType != DATA_LIST) ||
	    (e2->dataType != DATA_LIST)) {
	  delete e1;
	  delete e2;
	  return 0;
	}
	
	ret = e1->copy();
	e3 = ret->liststart;	
	while ((e3) && (e3->next)) 
	  e3 = e3->next;
	e4 = e2->liststart;
	
	if (e4)
	  if (!e3) {
	    ret->liststart = e4->copy();	
  	    e3 = ret->liststart; 
	    e4 = e4->next;
	    
  	    while (e4) {
	      e3->next = e4->copy();
  	      e3 = e3->next;
	      e4 = e4->next;
	    }
	  }
	else
	  while (e4) {
	    e3->next = e4->copy();
	    e3 = e3->next;
	    e4 = e4->next;
	  } 
	  
	delete e1;
	delete e2;
	return(ret);
      } // getIndex

    }

    if ( (parameters!=0) &&
         (parameters->size() == 1)) {
      
      if (strcmp(variablename->id->str(),"strlen")==0) {
        
        e1 = parameters->expression->eval(command,connection);

        if (e1==0) return 0;   
        if (e1->dataType != DATA_STRING) {
          delete e1;
          return 0;
        }
        ret = new UValue();
        ret->dataType = DATA_NUM;      
        ret->val = e1->str->len();

        // lourd...
        for (int i=0;i<e1->str->len()-1;i++)
          if ((e1->str->str()[i] == '\\') &&
              (e1->str->str()[i+1] == '"'))
            ret->val--;
        
        delete e1; 
        return(ret);
      } // strlen
 
      if (strcmp(variablename->id->str(),"head")==0) {
        
        e1 = parameters->expression->eval(command,connection);

        if (e1==0) return 0;   
        if (e1->dataType != DATA_LIST) {
          delete e1;
          return 0;
        }
	if (e1->liststart)
	  ret = e1->liststart->copy();
	else	   
          ret = new UValue();
	        
        delete e1; 
        return(ret);
      } // head

      if (strcmp(variablename->id->str(),"tail")==0) {
        
        e1 = parameters->expression->eval(command,connection);

        if (e1==0) return 0;   
        if (e1->dataType != DATA_LIST) {
          delete e1;
          return 0;
        }
	if (e1->liststart) {

          ret = new UValue();
          ret->dataType = DATA_LIST;
	  e2 = e1->liststart->next;
	  e3 = ret;
	  if (e2) {
	    e3->liststart = e2->copy();
	    e2 = e2->next;
	    e3 = e3->liststart;
	    while (e2) {
	      e3->next = e2->copy();
	      e2 = e2->next;
	      e3 = e3->next;
	    }
	  }	  
	}
	else	   
          ret = e1->copy();
	        
        delete e1; 
        return(ret);
      } // tail

      if (strcmp(variablename->id->str(),"size")==0) {
        
        e1 = parameters->expression->eval(command,connection);

        if (e1==0) return 0;   
        if (e1->dataType != DATA_LIST) {
          delete e1;
          return 0;
        }
	ret = new UValue(0.0);
	
	if (e1->liststart) {
	  
	  e2 = e1->liststart;
	  while (e2) {
	    e2 = e2->next;
	    ret->val = ret->val + 1;
	  }	  
	}

        delete e1; 
        return(ret);
      } // size


      if (strcmp(variablename->id->str(),"isdef")==0) {        
        
        ret = new UValue();
        ret->dataType = DATA_NUM;      
        ret->val = 0;

        if (parameters->expression->type == EXPR_VARIABLE)        
          if ((parameters->expression->variablename->getVariable(command,connection)) ||
              (parameters->expression->variablename->isFunction(command,connection)))
            ret->val = 1;
               
        return(ret);
      } // isdef

      if (strcmp(variablename->id->str(),"isvoid")==0) {        
        
        ret = new UValue();
        ret->dataType = DATA_NUM;      
        ret->val = 0;
	
        if (parameters->expression->type == EXPR_VARIABLE) {

	  UVariable* v = parameters->expression->variablename->getVariable(command,connection);
	  if ((v==0) || (v->value==0)) return ret;
	  if (v->value->dataType == DATA_VOID) ret->val = 1; 
	}
  	
  	return(ret);
      } // isvoid


      if (strcmp(variablename->id->str(),"load")==0) {
        
        e1 = parameters->expression->eval(command,connection);

        if (e1==0) return 0;   
        if (e1->dataType != DATA_STRING) {
          delete e1;
          return 0;
        }
        ret = new UValue();
        ret->dataType = DATA_VOID;

        // load file

        UCommand_LOAD *loadcmd = new UCommand_LOAD(command->up);
                                                 
        if (loadcmd == 0) { 
          delete ret;
          delete e1;
          return (0);
        }

        if (connection->server->loadFile(e1->str->str(),
                                         loadcmd->loadQueue) == UFAIL) {
          
          snprintf(errorString,errSize,
                   "!!! Cannot load the file %s\n",e1->str->str());
          connection->send(errorString,command->tag->str());
          delete ret;
          delete loadcmd;
          ret = 0;
        }
        else {
          
          command->morph = loadcmd;
          command->persistant = false;            

          snprintf(errorString,errSize,
                   "*** \"%s\" loaded.\n",e1->str->str());
          connection->send(errorString,command->tag->str());        
        }

        delete e1; 
        return(ret);
      } // load


      if (strcmp(variablename->id->str(),"loadwav")==0) {
        
        e1 = parameters->expression->eval(command,connection);

        if (e1==0) return 0;   
        if (e1->dataType != DATA_STRING) {
          delete e1;
          return 0;
        }
        ret = new UValue();
        ret->dataType = DATA_BINARY;
        UCommandQueue* loadQueue = new UCommandQueue (4096,1048576,false);
        // load file
        if (connection->server->loadFile(e1->str->str(),
                                         loadQueue) == UFAIL) {
          
          snprintf(errorString,errSize,
                   "!!! Cannot load the file %s\n",e1->str->str());
          connection->send(errorString,command->tag->str());
          delete ret;
          delete loadQueue;
          ret = 0;
        }
        else {
          UBinary *binaire = new UBinary(loadQueue->dataSize(),
                                         new UNamedParameters(new UExpression(EXPR_VALUE,new UString("wav")),
                                                              (UNamedParameters*)0));
          memcpy(binaire->buffer,
                 loadQueue->pop(loadQueue->dataSize()),
                 loadQueue->dataSize());

          ret->refBinary = new URefPt<UBinary>(binaire);
          delete loadQueue;
        }

        return(ret);
      } // loadwav



      if (strcmp(variablename->id->str(),"exec")==0) {
        
        e1 = parameters->expression->eval(command,connection);

        if (e1==0) return 0;   
        if (e1->dataType != DATA_STRING) {
          delete e1;
          return 0;
        }
        ret = new UValue();
        ret->dataType = DATA_VOID;

        // send string in the queue

        ::urbiserver->parser.commandTree = 0;
        errorMessage[0] = 0;
        ::urbiserver->systemcommands = false;	
	if (!connection->stack.empty())
	  connection->functionTag = new UString("__Funct__");
        int result = ::urbiserver->parser.process((ubyte*)e1->str->str(), 
                                                  e1->str->len(),                                                                                                connection); 
	if (connection->functionTag) {
	  delete connection->functionTag;
	  connection->functionTag=0;
	}     
        ::urbiserver->systemcommands = true;    

        if (errorMessage[0] != 0) { // a parsing error occured 
          
          if (::urbiserver->parser.commandTree) {
            delete ::urbiserver->parser.commandTree;
            ::urbiserver->parser.commandTree = 0;
          }
          
          connection->send(errorMessage,"error");                   
        }

        if (::urbiserver->parser.commandTree ) {
          
          command->morph = ::urbiserver->parser.commandTree;
          command->persistant = false; 
          ::urbiserver->parser.commandTree = 0;
        }
        else {  
          snprintf(errorString,errSize,
                   "!!! Error parsing the exec string\n");
          connection->send(errorString,command->tag->str());
          delete ret;
          ret = 0;
        }
        
        delete e1;
        return(ret);
      } // exec
    }

    if ( (parameters!=0) &&
         (parameters->size() == 3) &&
         (strcmp(variablename->id->str(),"strsub")==0) ) {
      
      e1 = parameters->expression->eval(command,connection);
      e2 = parameters->next->expression->eval(command,connection);
      e3 = parameters->next->next->expression->eval(command,connection);

      if ((e1==0) || (e2==0) || (e3==0)) return 0;   
      if (e1->dataType != DATA_STRING) {
        delete e1; delete e2; delete e3;        
        return 0;
      }   
      if (e2->dataType != DATA_NUM) {
        delete e1; delete e2; delete e3;
        return 0;
      }   
      if (e3->dataType != DATA_NUM) {
        delete e1; delete e2; delete e3;
        return 0;
      }
      ret = new UValue();
      ret->dataType = DATA_STRING;

      if (strcmp(variablename->id->str(),"strsub")==0)
        ret->str = new UString(e1->str->ext((int)e2->val,(int)e3->val));

      delete e1; delete e2; delete e3;
      return(ret);
    }

    if ( (parameters!=0  && 
	  (parameters->size() == 2) &&
	  (
	   (strcmp(variablename->id->str(),"atan2")==0) 
	  ))) {
       e1 = parameters->expression->eval(command,connection);

      if (e1==0) return 0;   
      if (e1->dataType != DATA_NUM) {
        delete e1;
        return 0;
      }
       e2 = parameters->next->expression->eval(command,connection);

      if (e2==0) return 0;   
      if (e2->dataType != DATA_NUM) {
        delete e1;
	delete e2;
        return 0;
      }
      ret = new UValue();
      ret->dataType = DATA_NUM;

      if (strcmp(variablename->id->str(),"atan2")==0)  ret->val = atan2(e1->val,e2->val);

      delete e1;
      delete e2;
      return ret;
    }
    if ( (parameters!=0) &&
         (parameters->size() == 1) &&
         (  (strcmp(variablename->id->str(),"sin")==0) ||
            (strcmp(variablename->id->str(),"asin")==0) ||
            (strcmp(variablename->id->str(),"cos")==0) ||
            (strcmp(variablename->id->str(),"acos")==0) ||
            (strcmp(variablename->id->str(),"tan")==0) ||
            (strcmp(variablename->id->str(),"atan")==0) ||
            (strcmp(variablename->id->str(),"sgn")==0) ||
            (strcmp(variablename->id->str(),"abs")==0) ||
            (strcmp(variablename->id->str(),"exp")==0) ||
            (strcmp(variablename->id->str(),"log")==0) ||
            (strcmp(variablename->id->str(),"round")==0) ||
            (strcmp(variablename->id->str(),"random")==0) ||
            (strcmp(variablename->id->str(),"trunc")==0) ||
            (strcmp(variablename->id->str(),"sqr")==0) ||
            (strcmp(variablename->id->str(),"sqrt")==0) ||
            (strcmp(variablename->id->str(),"string")==0) 
            ) ) {

      e1 = parameters->expression->eval(command,connection);

      if (e1==0) return 0;   
      if (e1->dataType != DATA_NUM) {
        delete e1;
        return 0;
      }
      
      if (strcmp(variablename->id->str(),"string")==0)  {
        
        ret = new UValue();
        ret->dataType = DATA_STRING;
        sprintf(errorString,"%d",(int)e1->val);
        ret->str = new UString(errorString);
        
        delete e1; 
        return(ret);
      }

      ret = new UValue();
      ret->dataType = DATA_NUM;

      if (strcmp(variablename->id->str(),"sin")==0)  ret->val = sin(e1->val);
      if (strcmp(variablename->id->str(),"asin")==0) ret->val = asin(e1->val);
      if (strcmp(variablename->id->str(),"cos")==0)  ret->val = cos(e1->val);
      if (strcmp(variablename->id->str(),"acos")==0) ret->val = acos(e1->val);
      if (strcmp(variablename->id->str(),"tan")==0)  ret->val = tan(e1->val);
      if (strcmp(variablename->id->str(),"atan")==0)  ret->val = atan(e1->val);
      if (strcmp(variablename->id->str(),"sgn")==0)  if (e1->val>0) ret->val =1; else if (e1->val<0) ret->val = -1;
      if (strcmp(variablename->id->str(),"abs")==0)  ret->val = fabs(e1->val);
      if (strcmp(variablename->id->str(),"random")==0)  ret->val = (rand()%(int)e1->val);
      if (strcmp(variablename->id->str(),"round")==0)  
        if (e1->val>=0)
          ret->val = (ufloat)(int)(e1->val+0.5);
        else
          ret->val = (ufloat)(int)(e1->val-0.5);
      if (strcmp(variablename->id->str(),"trunc")==0)  ret->val = (ufloat)(int)(e1->val);
      if (strcmp(variablename->id->str(),"exp")==0)  ret->val = exp(e1->val);
      if (strcmp(variablename->id->str(),"sqr")==0)  ret->val = e1->val*e1->val;
      if (strcmp(variablename->id->str(),"sqrt")==0) {
        if (e1->val<0) {
          snprintf(errorString,errSize,"!!! Negative square root\n");
          connection->send(errorString,command->tag->str());
          return 0;   
        }
        ret->val = sqrt(e1->val);
      }
      if (strcmp(variablename->id->str(),"log")==0) {
        if (e1->val<0) {
          snprintf(errorString,errSize,"!!! Negative logarithm\n");
          connection->send(errorString,command->tag->str());
          return 0;   
        }
        ret->val = log(e1->val);
      }

      delete e1; 
      return(ret);
    }
      
    // default = unknown.
    funname = variablename->buildFullname(command,connection);
    if (!variablename->getFullname()) return (0); 
    hmf = ::urbiserver->functiontab.find(funname->str());
    if (hmf != ::urbiserver->functiontab.end()) { 
	snprintf(errorString,errSize,"!!! Custom function call in expressions not allowed in kernel 1\n");
        connection->send(errorString,command->tag->str());
        return 0;   
    }

    
    if (parameters)
      snprintf(errorString,errSize,"!!! Error with function eval: %s [nb param=%d]\n",
               variablename->getFullname()->str(),parameters->size());
    else
      snprintf(errorString,errSize,"!!! Error with function eval: %s [no param]\n",
               variablename->getFullname()->str());
    if (!silent)
      connection->send(errorString,command->tag->str());
    return 0;   
    
  case EXPR_PLUS:
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
    
    if ((e1==0) || (e1->dataType == DATA_VOID) ||
        (e2==0) || (e2->dataType == DATA_VOID)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }
    ret = e1->add(e2);
    if ((expression1->isconst) && 
        (expression2->isconst))
      this->isconst = true;

    delete(e1);
    delete(e2);    
    return(ret);  

  case EXPR_MINUS:
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
    
    if ((e1==0) || (e1->dataType == DATA_VOID) ||
    	(e2==0) || (e2->dataType == DATA_VOID)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    if ((e1->dataType != DATA_NUM) ||
        (e2->dataType != DATA_NUM)) {

      delete e1;
      delete e2;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;
    ret->val = e1->val - e2->val;
    delete(e1);
    delete(e2);
    return(ret);
  

  case EXPR_MULT:
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
    
    if ((e1==0) || (e1->dataType == DATA_VOID) ||
    	(e2==0) || (e2->dataType == DATA_VOID)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    if ((e1->dataType != DATA_NUM) ||
        (e2->dataType != DATA_NUM)) {

      delete e1;
      delete e2;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;
    ret->val = e1->val * e2->val;
    delete(e1);
    delete(e2);
    return(ret);
  

  case EXPR_DIV:
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
    
    if ((e1==0) || (e1->dataType == DATA_VOID) ||
    	(e2==0) || (e2->dataType == DATA_VOID)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    if ((e1->dataType != DATA_NUM) ||
        (e2->dataType != DATA_NUM)) {

      delete e1;
      delete e2;
      return 0;
    }

    if (e2->val == 0) {
      snprintf(errorString,errSize,"!!! Division by zero\n");              
      connection->send(errorString,command->tag->str());
      return 0;
    }      

    ret = new UValue();
    ret->dataType = DATA_NUM;
    ret->val = e1->val / e2->val;
    delete(e1);
    delete(e2);
    return(ret);

  case EXPR_MOD:
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
    
    if ((e1==0) || (e2==0)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    if ((e1->dataType != DATA_NUM) ||
        (e2->dataType != DATA_NUM)) {

      delete e1;
      delete e2;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;
    ret->val = fmod(e1->val,e2->val);
    delete(e1);
    delete(e2);
    return(ret);
  
  case EXPR_EXP:
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
    
    if ((e1==0) || (e1->dataType == DATA_VOID) ||
    	(e2==0) || (e2->dataType == DATA_VOID)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    if ((e1->dataType != DATA_NUM) ||
        (e2->dataType != DATA_NUM)) {

      delete e1;
      delete e2;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;
    ret->val = pow(e1->val,e2->val);
    delete(e1);
    delete(e2);
    return(ret);
  

  case EXPR_NEG:
    
    e1 = expression1->eval(command,connection);
    
    if ((e1==0) || (e1->dataType == DATA_VOID)) return 0;   
    if (e1->dataType != DATA_NUM) {
      delete e1;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;
    ret->val = -e1->val;
    delete(e1);
    return(ret);

  case EXPR_COPY:
    
    e1 = expression1->eval(command,connection);   

    if (e1==0) return 0;  

    ret = e1->copy();
    if (ret->dataType == DATA_BINARY) {
      
      UBinary *binaire = new UBinary(ret->refBinary->ref()->bufferSize,0);
      if ((!binaire) || (binaire->buffer == 0)) return 0;
      binaire->parameters = ret->refBinary->ref()->parameters->copy();

      URefPt<UBinary> *ref = new URefPt<UBinary>(binaire);
      if (!ref) return 0;
      
      memcpy(binaire->buffer,
             ret->refBinary->ref()->buffer,
             ret->refBinary->ref()->bufferSize);

      LIBERATE(ret->refBinary);
      ret->refBinary = ref; 
    }
    delete(e1);
    return(ret);

  case EXPR_TEST_EQ:  
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
    
    if ((e1==0) ||
    	(e2==0)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;
    ret->val = e1->equal(e2);
    delete(e1);
    delete(e2);
    return(ret);

  case EXPR_TEST_REQ:
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
    
    if ((e1==0) || (e1->dataType == DATA_VOID) ||
    	(e2==0) || (e2->dataType == DATA_VOID)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    if ((e1->dataType != DATA_NUM) ||
        (e2->dataType != DATA_NUM)) {

      snprintf(errorString,errSize,"!!! Approximate comparisons must be between numerical values\n");              
      connection->send(errorString,command->tag->str());
      delete e1;
      delete e2;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;

    variable = ::urbiserver->getVariable(MAINDEVICE,"epsilontilde");
    if (variable) 
      ret->val = (ABSF(e1->val - e2->val) <= variable->value->val ); 
    else
      ret->val = 0;
    
    delete(e1);
    delete(e2);
    return(ret);

  case EXPR_TEST_DEQ:

    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
    
    if ((e1==0) || (e1->dataType == DATA_VOID) ||
    	(e2==0) || (e2->dataType == DATA_VOID)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    if ((e1->dataType != DATA_NUM) ||
        (e2->dataType != DATA_NUM)) {

      snprintf(errorString,errSize,"!!! Approximate comparisons must be between numerical values\n");              
      connection->send(errorString,command->tag->str());
      delete e1;
      delete e2;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;

    d1 = 0;
    if (expression1->type == EXPR_VARIABLE) {
      
      variable = expression1->variablename->getVariable(command,connection); 
      if (variable) d1 = variable->delta;
    }
    d2 = 0;
    if (expression2->type == EXPR_VARIABLE) {
      
      variable = expression2->variablename->getVariable(command,connection); 
      if (variable) d2 = variable->delta;
    }
  
    ret->val = (ABSF(e1->val - e2->val) <= d1+d2 );      
    
    delete(e1);
    delete(e2);
    return(ret);
  
  case EXPR_TEST_PEQ:
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
     
    if ((e1==0) || (e1->dataType == DATA_VOID) ||
    	(e2==0) || (e2->dataType == DATA_VOID)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    if ((e1->dataType != DATA_NUM) ||
        (e2->dataType != DATA_NUM)) {

      snprintf(errorString,errSize,"!!! Approximate comparisons must be between numerical values\n");              
      connection->send(errorString,command->tag->str());
      delete e1;
      delete e2;
      return 0;
    }

    if ((e2->val == 0) || (e1->val == 0)) {
      snprintf(errorString,errSize,"!!! Division by zero\n");              
      connection->send(errorString,command->tag->str());
      return 0;
    }     

    ret = new UValue();
    ret->dataType = DATA_NUM;

    variable = ::urbiserver->getVariable(MAINDEVICE,"epsilonpercent");
    if (variable) 
      ret->val = (ABSF( 1 - (e1->val / e2->val)) <= variable->value->val ); 
    else
      ret->val = 0;
    
    delete(e1);
    delete(e2);
    return(ret);

  case EXPR_TEST_NE: 
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
     
    if ((e1==0) ||
    	(e2==0)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;
    ret->val = !e1->equal(e2);
    delete(e1);
    delete(e2);
    return(ret);

  case EXPR_TEST_GT:
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
     
    if ((e1==0) || (e1->dataType == DATA_VOID) ||
    	(e2==0) || (e2->dataType == DATA_VOID)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    if ((e1->dataType != DATA_NUM) ||
        (e2->dataType != DATA_NUM)) {

      snprintf(errorString,errSize,"!!! Numerical comparisons must be between numerical values\n");              
      connection->send(errorString,command->tag->str());
      delete e1;
      delete e2;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;
   
    ret->val = (e1->val > e2->val);
    
    delete(e1);
    delete(e2);
    return(ret);

  case EXPR_TEST_GE:
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
     
    if ((e1==0) || (e1->dataType == DATA_VOID) ||
    	(e2==0) || (e2->dataType == DATA_VOID)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    if ((e1->dataType != DATA_NUM) ||
        (e2->dataType != DATA_NUM)) {

      snprintf(errorString,errSize,"!!! Numerical comparisons must be between numerical values\n");              
      connection->send(errorString,command->tag->str());
      delete e1;
      delete e2;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;
   
    ret->val = (e1->val >= e2->val);
    
    delete(e1);
    delete(e2);
    return(ret);

  case EXPR_TEST_LT:
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
     
    if ((e1==0) || (e1->dataType == DATA_VOID) ||
    	(e2==0) || (e2->dataType == DATA_VOID)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    if ((e1->dataType != DATA_NUM) ||
        (e2->dataType != DATA_NUM)) {

      snprintf(errorString,errSize,"!!! Numerical comparisons must be between numerical values\n");              
      connection->send(errorString,command->tag->str());
      delete e1;
      delete e2;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;
   
    ret->val = (e1->val < e2->val);
    
    delete(e1);
    delete(e2);
    return(ret);

  case EXPR_TEST_LE:
    
    e1 = expression1->eval(command,connection);
    e2 = expression2->eval(command,connection);
     
    if ((e1==0) || (e1->dataType == DATA_VOID) ||
    	(e2==0) || (e2->dataType == DATA_VOID)) {
      if (e1) delete e1;
      if (e2) delete e2;
      return 0;
    }

    if ((e1->dataType != DATA_NUM) ||
        (e2->dataType != DATA_NUM)) {

      snprintf(errorString,errSize,"!!! Numerical comparisons must be between numerical values\n");              
      connection->send(errorString,command->tag->str());
      delete e1;
      delete e2;
      return 0;
    }

    ret = new UValue();
    ret->dataType = DATA_NUM;
   
    ret->val = (e1->val <= e2->val);
    
    delete(e1);
    delete(e2);
    return(ret);

  case EXPR_TEST_BANG:
    
    e1 = expression1->eval(command,connection,silent);
    
    if (e1==0)  {
      if (e1) delete e1;
      return 0;
    }
    /*
    if (e1->dataType != DATA_NUM) {

      snprintf(errorString,errSize,"!!! Non boolean value\n");              
      connection->send(errorString,command->tag->str());
      delete e1;
      return 0;
    }
    */
    ret = new UValue();
    ret->dataType = DATA_NUM;
    ret->eventid = e1->eventid;
   
    if (e1->val == 0) ret->val = 1;
    else ret->val = 0;
    
    delete(e1);  
    return(ret);

  case EXPR_TEST_AND:
    
    e1 = expression1->eval(command,connection,silent);   
    if (e1==0)  { 
      if (e1) delete e1;
      return 0;
    }
    /*  
    if (e1->dataType != DATA_NUM) {

      snprintf(errorString,errSize,"!!! Non boolean value\n");   
      connection->send(errorString,command->tag->str());
      delete e1;     
      return 0;
    }
    */
    
    ret = new UValue();
    if (!ret) return 0;
    ret->dataType = DATA_NUM;
    ret->eventid = e1->eventid;
    
    if (!((int)e1->val)) ret->val = 0;
    else {

      e2 = expression2->eval(command,connection,silent);  
      if (!e2) {
        delete ret;
        delete e1;
        return 0;
      }

      if (e2->dataType != DATA_NUM) {
        /*
        snprintf(errorString,errSize,"!!! Non boolean value\n");   
        connection->send(errorString,command->tag->str());   
        delete e1;
        delete e2;
        delete ret;
        return 0;
        */
      }

      ret->eventid = ret->eventid +  e2->eventid;
      ret->val = (ufloat) ( ((int)e1->val) && ((int)e2->val) );
      delete(e2);
    }

    delete(e1);    
    return(ret);

  case EXPR_TEST_OR:
    
    
    e1 = expression1->eval(command,connection,silent);   
    if (!e1) return 0;    
    /*    
if (e1->dataType != DATA_NUM) {

      snprintf(errorString,errSize,"!!! Non boolean value\n");   
      connection->send(errorString,command->tag->str());
      delete e1;     
      return 0;
    }
    */
    
    ret = new UValue();
    if (!ret) return 0;
    ret->dataType = DATA_NUM;
    ret->eventid = e1->eventid;
    
    if (((int)e1->val)) ret->val = 1;
    else {

      e2 = expression2->eval(command,connection,silent);  
      if (!e2) {
        delete ret;
        delete e1;
        return 0;
      }

      if (e2->dataType != DATA_NUM) {
        /*
        snprintf(errorString,errSize,"!!! Non boolean value\n");   
        connection->send(errorString,command->tag->str());   
        delete e1;
        delete e2;
        delete ret;
        return 0;
        */
      }

      ret->eventid = ret->eventid +  e2->eventid;
      ret->val = (ufloat) ( ((int)e1->val) || ((int)e2->val) );
      delete(e2);
    }

    delete(e1);    
    return(ret);

  default: 
    return 0;
  }
}
