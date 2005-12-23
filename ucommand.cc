/*! \file ucommand.cc
 *******************************************************************************

 File: ucommand.cc\n
 Implementation of the UCommand class.

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
#include <stdlib.h>

#include "ucommand.h"
#include "uconnection.h"
#include "udevice.h"
#include "ugroupdevice.h"
#include "userver.h"
#include "ucallid.h"

char tmpbuffer[UCommand::MAXSIZE_TMPMESSAGE];  ///< temporary global string                                              

// **************************************************************************
//! UCommand constructor.
/*! The parameter 'type' is required here to describe the type of the command.
    
    \param type is the command type
*/
UCommand::UCommand(UCommandType _type)
{
  if (::urbiserver->systemcommands)
    tag = new UString("__system__");
  else
    tag = new UString("notag");
  flags             = 0;
  type              = _type;
  status            = UONQUEUE;
  morph             = 0;
  persistant        = false;
  toDelete          = false;
  background        = false;
  startTime         = -1;
  flagType          = 0;
  flagExpr1         = 0;
  flagExpr2         = 0;
  flagExpr4         = 0;
  flag_nbTrue2       = 0;
  flag_nbTrue4       = 0;
  morphed           = false;
}

//! UCommand destructor.
UCommand::~UCommand()
{
  if (tag)        delete(tag);
  if (flags)      delete(flags);
}

UCommandStatus
UCommand::execute(UConnection *connection) 
{
  return UCOMPLETED;
}

//! UCommand hard copy function
UCommand*
UCommand::copy() 
{  
  UCommand *ret = new UCommand(type);
  copybase(ret);
  return ret;
}

//! UCommand base of hard copy function
UErrorValue
UCommand::copybase(UCommand *command) 
{   
  if (tag) {    
    command->tag->update(tag->str());
    if (!command->tag) return (UMEMORYFAIL);
  }

  if (flags) {
    command->flags = flags->copy();
    if (!command->flags) return (UMEMORYFAIL);
  } 
}

//! Marks commands for deletion, in a stop command.
void
UCommand::mark(UString *stopTag)
{
  if (stopTag->equal(tag))
    toDelete = true;
}

//! Deletes commands marked for deletion after a stop command
void
UCommand::deleteMarked()
{
  // General commands do nothing. Only trees try to delete leaves.
}

//! Print command
void 
UCommand::print(int l)
{
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
    The background parameter lets the tree execute in background.
    This is useful for the LOAD command which should be run in bg and
    still cannot be persistant (like a AT or WHENEVER).
*/
UCommand_TREE::UCommand_TREE( UNodeType node,
                              UCommand* command1,
                              UCommand* command2) :
  UCommand( CMD_TREE )
{  
  ADDOBJ(UCommand_TREE);
  this->command1    = command1;
  this->command2    = command2;
  this->node        = node;

  if (command1) {
    command1->up = this;
    command1->position = &(this->command1);
  }
  if (command2) {
    command2->up = this;
    command2->position = &(this->command2);
  }

  runlevel1  = UWAITING;
  runlevel2  = UWAITING;
  background = false;
  callid     = 0;
  connection = 0; // unknown unless there is a context  
}

//! UCommand subclass destructor.
UCommand_TREE::~UCommand_TREE()
{
  FREEOBJ(UCommand_TREE);
 
  if (command1) delete(command1);
  if (command2) delete(command2); 
  if (callid) delete(callid); // this frees the local variable for the function call, including
                              // the function parameters
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_TREE::execute(UConnection *connection)
{  
  return URUNNING;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_TREE::copy() 
{  
  UCommand* copy_command1;
  UCommand* copy_command2;
  
  if (command1) copy_command1 = command1->copy(); else copy_command1 =0;
  if (command2) copy_command2 = command2->copy(); else copy_command2 =0;

  UCommand_TREE *ret = new UCommand_TREE(node,
                                         copy_command1,
                                         copy_command2);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Marks commands for deletion, in a stop command.
void
UCommand_TREE::mark(UString *stopTag)
{  
  int go_to = 1;
  UCommand_TREE *tree = this;

  if ((stopTag->equal(tag)) &&
      ((status != UONQUEUE) || (morphed))) {
    toDelete = true;
    return;
  }

  while (tree != up) {

    if ((tree->command1) && (go_to == 1)) 
      if ( (stopTag->equal(tree->command1->tag)) &&
           ((tree->command1->status != UONQUEUE) || 
            (tree->command1->morphed))) 
        tree->command1->toDelete = true;           
      else 
        if (tree->command1->type == CMD_TREE) {
          tree = (UCommand_TREE*) tree->command1;          
          go_to = 1;
          continue;
        }
    if ((tree->command2) && (go_to >= 1)) 
      if ((stopTag->equal(tree->command2->tag)) &&
          ((tree->command2->status != UONQUEUE) ||
           (tree->command2->morphed)))
        tree->command2->toDelete = true;            
      else 
        if (tree->command2->type == CMD_TREE) {
          tree = (UCommand_TREE*) tree->command2;          
          go_to = 1;
          continue;
        }
    
    go_to = 2;
    if (tree->up)
      if (*(tree->position) == tree->up->command2)
        go_to = 0;
    
    tree = tree->up;
  } 
}

//! Deletes sub commands marked for deletion after a stop command
void
UCommand_TREE::deleteMarked()
{
  int go_to = 1;
  UCommand_TREE *tree = this;   

  while (tree != up) {

    if ((tree->command1) && (go_to == 1))
      if (tree->command1->toDelete) { 
        delete tree->command1;
        tree->command1 = 0;    
      }
      else
        if (tree->command1->type == CMD_TREE) {
          tree = (UCommand_TREE*) tree->command1;          
          go_to = 1;
          continue;
        }
      
    if ((tree->command2) && (go_to >= 1))
      if (tree->command2->toDelete) {
        delete tree->command2;
        tree->command2 = 0;
      }
      else
        if (tree->command2->type == CMD_TREE) {
          tree = (UCommand_TREE*) tree->command2;          
          go_to = 1;
          continue;
        }

    go_to = 2;
    if (tree->up)
      if (*(tree->position) == tree->up->command2)
        go_to = 0;
      
    tree = tree->up;
  }
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_TREE::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%sTag:[%s] toDelete=%d ",tabb,tag->str(),toDelete);}
  else ::urbiserver->debug("%s",tabb);

  if (node == UAND) ::urbiserver->debug("Tree AND ");
  else
    if (node == UPIPE) ::urbiserver->debug("Tree PIPE ");
    else
      if (node == USEMICOLON) ::urbiserver->debug("Tree SEMICOLON ");
      else
        if (node == UCOMMA) ::urbiserver->debug("Tree COMMA ");
        else
          ::urbiserver->debug("UNKNOWN TREE!\n");

  ::urbiserver->debug("(%d:%d) :\n",(int)this,(int)status);
  if (command1) { ::urbiserver->debug("%s  Com1 (%d:%d) up=%d:\n",tabb,(int)command1,command1->status,(int)command1->up); command1->print(l+3);};
  if (command2) { ::urbiserver->debug("%s  Com2 (%d:%d) up=%d:\n",tabb,(int)command2,command2->status,(int)command2->up); command2->print(l+3);};  

  ::urbiserver->debug("%sEND TREE ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_ASSIGN_VALUE::UCommand_ASSIGN_VALUE(UVariableName *variablename, 
                                             UExpression* expression, 
                                             UNamedParameters *parameters,
                                             bool defkey) :
  UCommand(CMD_ASSIGN_VALUE)
{	
  ADDOBJ(UCommand_ASSIGN_VALUE);
  this->variablename= variablename;
  this->expression  = expression;
  this->parameters  = parameters;
  finished          = false;
  this->method      = 0;
  this->devicename  = 0;
  this->dev         = 0;
  this->tmp_phase   = 0;
  this->tmp_time    = 0;
  this->variable    = 0;
  this->assigned    = false;
  this->defkey      = defkey;
  

  endtime           = -1;
  modif_time        = 0;
  modif_sin         = 0;
  modif_phase       = 0;
  modif_smooth      = 0;
  modif_speed       = 0;
  modif_accel       = 0;
  modif_adaptive    = 0;
  modif_ampli       = 0;
  modif_getphase    = 0;
  profileDone       = false;
}

//! UCommand subclass destructor.
UCommand_ASSIGN_VALUE::~UCommand_ASSIGN_VALUE()
{
  FREEOBJ(UCommand_ASSIGN_VALUE);
  if (expression) delete expression;
  if (variablename)  delete variablename;
  if (parameters) delete parameters;  
  if (tmp_phase)  delete tmp_phase;
  if (tmp_time)   delete tmp_time;

  if (assigned) {
    variable->nbAssigns--;
    if (variable->cancel == this)
      variable->cancel = 0;
  }
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_ASSIGN_VALUE::execute(UConnection *connection)
{ 
  UValue *target;
  UValue *modificator;
  UValue *value;
  double currentTime;
  UNamedParameters *modif;
  UVariable *vari;
  HMgrouptab::iterator hmg;

  // General initializations
  if (!variable) {
    variable = variablename->getVariable(this,connection); 
    if (!variablename->getFullname()) return ( status = UCOMPLETED );  
    method = variablename->getMethod();
    devicename = variablename->getDevice();
    dev = variablename->getDev(this,connection);
  }
  currentTime = connection->server->lastTime();

  // Wait in queue if needed
  if (variable)
    if ((variable->blendType == UQUEUE) && (variable->nbAverage > 0))
      return(status);  

  // Handling of groups and implicit multi assignments
  // (using morphing and & conjunction)
  if ((!variablename->rooted) && (devicename)) {

    UGroup *gp = 0;
    if ((hmg = ::urbiserver->grouptab.find(devicename->str())) !=
        ::urbiserver->grouptab.end()) 
      gp = (*hmg).second;
        
    if ((gp) && (gp->members.size() > 0)) {
        
      UCommand *grouplist = 0;
      UCommand *grouplist_prev = 0;
      UCommand_ASSIGN_VALUE *clone;
      UNamedParameters *varindex;

      for (list<UGroup*>::iterator retr = gp->members.begin();
           retr != gp->members.end();
           retr++) {


	clone = (UCommand_ASSIGN_VALUE*)this->copy();
	delete clone->variablename;
	
        if (variablename->index) 
          varindex = variablename->index->copy();
        else
          varindex = 0;
	
        clone->variablename = new UVariableName((*retr)->device->copy(),
                                                method->copy(),
                                                false,
                                                varindex);
	
        clone->variablename->isnormalized = variablename->isnormalized;
        clone->variablename->deriv = variablename->deriv;
        clone->variablename->varerror = variablename->varerror;

        grouplist = (UCommand*)
          new UCommand_TREE(UAND,
                            (UCommand*)clone,
                            grouplist_prev);
        grouplist_prev = grouplist;        
      }
      morph = (UCommand*)
        new UCommand_TREE(UAND,
                          this,
                          grouplist);
      
      variablename->rooted = true;
      variablename->fromGroup = true;
      persistant = true;
      return( status = UMORPH );
      
    }
  }
 

  // Function call
  // morph into the function code
  if (expression->type == EXPR_FUNCTION) {
    
    UString* functionname = expression->variablename->buildFullname(this,connection);
    if (!functionname) return ( status = UCOMPLETED );
    
    UFunction *fun;
    HMfunctiontab::iterator hmf;
    
    if ( (hmf = connection->server->functiontab.find(functionname->str())) !=
         connection->server->functiontab.end()) {
      
      fun = (*hmf).second;
      
      if ( ( (expression->parameters) && 
             (fun->nbparam()) && 
             (expression->parameters->size() != fun->nbparam())) ||
           ( (expression->parameters) && (!fun->nbparam())) ||
           ( (!expression->parameters) && (fun->nbparam())) ) {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "!!! invalid number of arguments for %s (should be %d params)\n",
                 variablename->getFullname()->str(),fun->nbparam());
        connection->send(tmpbuffer,tag->str()); 
        
        return( status = UCOMPLETED );
      }
      
      persistant = false;
      UVariableName* resultContainer = new UVariableName(
                                           new UString("__UFnct"),
                                           new UString("__result__"), 
                                           true, 
                                           (UNamedParameters*)0);
      
      morph = (UCommand*) 
        new UCommand_TREE(UPIPE,
                          fun->cmdcopy(),
                          new UCommand_ASSIGN_VALUE(
                              variablename->copy(),
                              new UExpression(EXPR_VARIABLE,
                                              resultContainer),
                              (UNamedParameters*)0));
      
      
      if (morph) {
        
        sprintf(tmpbuffer,"__UFnct%d",(int)morph);
        ((UCommand_TREE*)morph)->callid = new UCallid(tmpbuffer,(UCommand_TREE*)morph);
        resultContainer->nameUpdate(((UCommand_TREE*)morph)->callid->str(),
                                    "__result__");
        if (!((UCommand_TREE*)morph)->callid) return (status = UCOMPLETED);
        ((UCommand_TREE*)morph)->connection = connection;
        
        UNamedParameters *pvalue = expression->parameters;
        UNamedParameters *pname  = fun->parameters;
        for (;
             pvalue != 0;
             pvalue = pvalue->next, pname = pname->next) {
          
          UValue* valparam = pvalue->expression->eval(this,connection);
          if (!valparam) {
            
            connection->send("!!! EXPR evaluation failed\n",tag->str());
            return (status = UCOMPLETED);
          }
          
          ((UCommand_TREE*)morph)->callid->store(
              new UVariable(((UCommand_TREE*)morph)->callid->str(),
                            pname->name->str(),
                            valparam)
              );
          
        }
      }
      
      return ( status = UMORPH );
    } // fi: function exists
  }


  ////////////////////////////////////////
  // Initialization phase (first pass)
  ////////////////////////////////////////

  if (status == UONQUEUE) {

    if ((!variable) && (connection->server->defcheck) && (!defkey)) {

      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Unknown identifier: %s\n",variablename->getFullname()->str());
      connection->send(tmpbuffer,tag->str());            
    }
     
    // Check the +error flag
    UNamedParameters *param = flags;
    errorFlag = false;
    while (param) {
      
      if ((param->name) && 
          ( param->name->equal("flag")) &&
          ( param->expression) &&
          ( param->expression->val == 2)) // 2 = +error           
        errorFlag = true;
      
      param = param->next;
    }    
        
    // UCANCEL mode
    if ((variable) && (variable->blendType == UCANCEL)) {
      
      variable->nbAverage = 0;
      variable->cancel = this;
    }
    
    // eval the right side of the assignment and check for errors
    target = expression->eval(this,connection);
    if ((target == 0) || (target->dataType == DATA_VOID))
      return( status = UCOMPLETED);

    // Check type compatibility if the left side variable already exists   
    if ((variable) &&
        (variable->value->dataType != DATA_VOID) &&
        (target->dataType != variable->value->dataType)) {

      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Warning: %s type mismatch\n",variablename->getFullname()->str());
      if (::urbiserver->defcheck)
        connection->send(tmpbuffer,tag->str());
      delete variable;
      variable = 0;
      //delete target;
      //return( status = UCOMPLETED );
    }   


    // STRING init ///////////////////
    //////////////////////////////////
    if (target->dataType == DATA_STRING) { // STRING     

      // Handle String Composition
      if (parameters != 0) {
        
        char *result  = (char*)malloc(sizeof(char) * (65000+target->str->len()));       
        char *possub;

        if (result) {

          strcpy (result, target->str->str());
          modif = parameters;

          while (modif) {
           
            modificator = modif->expression->eval(this,connection);
            if (!modificator) {
              
              snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                       "!!! String composition failed\n");
              connection->send(tmpbuffer,tag->str());
              delete target;
              
              return( status = UCOMPLETED );
            }

            if (modificator->dataType == DATA_NUM) {
              modificator->dataType = DATA_STRING;
              
              char *tmp_String = new char[255];
              if (tmp_String==0) return( status = UCOMPLETED ); 
                
              snprintf(tmp_String,255,
                       "%f",modificator->val);

              modificator->str = new UString(tmp_String);
              delete[] (tmp_String);
            }
                       
            snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                     "$%s",modif->name->str());

            if (strstr(modificator->str->str(), tmpbuffer) == 0)
              while ( possub = strstr(result, tmpbuffer) ) {
                          
                memmove(possub + modificator->str->len(),
                        possub + strlen(tmpbuffer),
                        strlen(result) - 
                        (int)(possub - result) - 
                        strlen(tmpbuffer)+1);
                
                strncpy (possub, 
                         modificator->str->str(),
                         modificator->str->len());                      
              }
                        
            delete modificator;
            modif = modif->next;
          }
          target->str->update(result);
          free(result);
        }
      } // end of string composition: target is up to date

      // Assignment
      if (variable) // the variable already exists 
        variable->set(target);
      else {
        variable = new UVariable(variablename->getFullname()->str(),target->copy());
        if (!variable) return ( status = UCOMPLETED );
        connection->localVariableCheck(variable);
      }

      delete (target);
      return( status = UCOMPLETED );
    }

    // BINARY init ///////////////////
    //////////////////////////////////
    if (target->dataType == DATA_BINARY) { // BINARY
     
      // Assignment
      if (variable) // the variable already exists 
        variable->set(target);
      else {
        variable = new UVariable(variablename->getFullname()->str(),target->copy());
        if (!variable) return ( status = UCOMPLETED );
        connection->localVariableCheck(variable);
      }

      delete (target);
      return( status = UCOMPLETED );
    }  

    // LIST init ///////////////////
    //////////////////////////////////

    if (target->dataType == DATA_LIST) { // LIST
      
      // Assignment
      if (variable) // the variable already exists 
        variable->set(target);
      else {
        variable = new UVariable(variablename->getFullname()->str(),target->copy());
        if (!variable) return ( status = UCOMPLETED );
        connection->localVariableCheck(variable);
      }

      delete (target);
      return( status = UCOMPLETED );
    }

    // NUM init ///////////////////
    //////////////////////////////////
    if (target->dataType == DATA_NUM) { // NUM
      
      bool controlled = false; // is a virtual "time:0" needed?
      targetval = target->val;

      // Handling normalized correction
      if ((variable) && (variablename->isnormalized)) {
      
        if ((variable->rangemin == -UINFINITY) ||
            (variable->rangemax ==  UINFINITY)) {

          if (!variablename->fromGroup) {
            snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                     "!!! Impossible to normalize: no range defined for variable %s\n",
                     variablename->getFullname()->str());
            connection->send(tmpbuffer,tag->str());
          }
          delete target;
          return( status = UCOMPLETED );
        }

        if (targetval < 0) targetval = 0;
        if (targetval > 1) targetval = 1;
        
        targetval = variable->rangemin + targetval * 
          (variable->rangemax - variable->rangemin);              
      }

      // Store init time
      starttime = currentTime;

      // Handling FLAGS
      if (parameters) {

        // Check if sinusoidal (=> no start value needed = no integrity check)
        modif = parameters;  
        bool sinusoidal = false;
        while (modif) {
          if ((modif->name->equal("sin")) ||
              (modif->name->equal("cos")))
            sinusoidal = true;  
          modif = modif->next;
        }

        // Checking integrity (variable exists), if not sinusoidal   
        if ((variable == 0) && (!sinusoidal)) {
          snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                   "!!! Modificator error: %s unknown (no start value)\n",
                   variablename->getFullname()->str());
          if (!variablename->fromGroup)
            connection->send(tmpbuffer,tag->str());
          delete target;
          return( status = UCOMPLETED );   
        }  

        speed    = 0;

        // Initialize modificators
        
        bool found;
        modif = parameters;        
        
        while (modif) {

          if ((!modif->expression) ||
              (!modif->name)) {
            snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                     "!!! Invalid modificator\n");
            connection->send(tmpbuffer,tag->str());
            
            delete target;
            return( status = UCOMPLETED );   
          }
          
          found = false;

          if (modif->name->equal("sin")) {
            modif_sin = modif->expression;
            found = true;
            controlled = true;
          }

          if (modif->name->equal("cos")) {
            modif_sin = modif->expression;
            tmp_phase = new UExpression(EXPR_VALUE,PI/2);
            modif_phase = tmp_phase;
            found = true;
            controlled = true;
          }
          
          if (modif->name->equal("ampli")) {
            modif_ampli = modif->expression;
            found = true;
          }

          if (modif->name->equal("smooth")) {
            modif_smooth = modif->expression;
            found = true;
            controlled = true;
          }

          if (modif->name->equal("time")) {
            modif_time = modif->expression;
            found = true;
            controlled = true;
          }

          if (modif->name->equal("speed")) {
            modif_speed = modif->expression;
            found = true;
            controlled = true;
          }
                 
          if (modif->name->equal("accel")) {
            modif_accel = modif->expression;
            found = true;
            controlled = true;
          }

          if (modif->name->equal("adaptive")) {
            modif_adaptive = modif->expression;
            
            found = true;
          }

          if (modif->name->equal("phase")) {
            modif_phase = modif->expression;
            found = true;
          }

          if (modif->name->equal("getphase")) {
            if (modif->expression->type != EXPR_VARIABLE) {
              snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                       "!!! a variable is expected for the 'getphase' modificator\n");
              connection->send(tmpbuffer,tag->str());
              return( status = UCOMPLETED );  
            }
            modif_getphase = modif->expression->variablename;
            found = true;
          }

          
          if (modif->name->equal("timelimit")) {
            modificator = modif->expression->eval(this,connection);
            if ( (!modificator) ||
                 (modificator->dataType != DATA_NUM) ) {
              snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                     "!!! Invalid modificator value\n");
              connection->send(tmpbuffer,tag->str());
            
              if (modificator) delete modificator;
              delete target;
              return( status = UCOMPLETED );   
            }
            endtime = currentTime + modificator->val;
            delete modificator;
            found = true;
          }            
          
          if (!found) { 
            snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                     "!!! Unkown modificator name\n");
              connection->send(tmpbuffer,tag->str());
                          
              delete target;
              return( status = UCOMPLETED );   
          }          

          modif = modif->next;
        }    
      } // end FLAGS handling

      // create var if it does not already exist
      if (!variable) {
        variable = new UVariable(variablename->getFullname()->str(),target->copy());
        if (!variable) return ( status = UCOMPLETED );   
        connection->localVariableCheck(variable);
      }

      // correct the type of VOID variables (comming from a def)
      if (variable->value->dataType == DATA_VOID)
        variable->value->dataType = DATA_NUM;

      // virtual "time:0" if no modificator specified (controlled == false)
      if (!controlled) {// no controlling modificator => time:0
          tmp_time = new UExpression(EXPR_VALUE,0.0);
          modif_time = tmp_time;
        }

      // clean the temporary target UValue
      delete target;
      
      // UDISCARD mode
      if ((variable->blendType == UDISCARD) && 
          (variable->nbAssigns > 0))
        return( status = UCOMPLETED );  
      
      // init valarray for a "val" assignment
      double *targetvalue =  &(variable->get()->val);
    
      if (variable->autoUpdate)
        valtmp = targetvalue;         // &variable->value->val
      else
        valtmp = &(variable->target); // &variable->target

      variable->nbAssigns++;
      assigned = true;
      startval = *targetvalue;  
      first = true;      
      status = URUNNING;
    }
  } 
  

  /////////////////////////////////////////////////
  // Execution phase (second pass and next passes)
  /////////////////////////////////////////////////
  
  if (status == URUNNING) {    
    
    if (finished)
      if (variable->reloop)         
        finished = false;      
      else
        return (status = UCOMPLETED);

    /*
      // This function is removed because it cannot give interesting results on Aibo
      // the delay is always crossed if given a sufficient speed
    if ((errorFlag) && 
        (fabs(variable->previous - variable->value->val) > 2*variable->delta)) {
      
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! delayed, is %f, should be %f\n",variable->value->val,variable->previous);
      connection->send(tmpbuffer,tag->str());      
    }
    */

    double deltaTime = connection->server->getFrequency();
    
    // Cancel if needed
    if ((variable->blendType == UCANCEL) && (variable->cancel != this))      
      return(status = UCOMPLETED);

    // Discard if needed
    if ((variable->blendType == UDISCARD) && (variable->nbAverage > 0))
      return(status = UCOMPLETED);    

    // In normal mode, there is always only one value to consider
    if (variable->blendType == UNORMAL)
      variable->nbAverage = 0;
   
    // In add mode, the current value is always added
    if ((variable->blendType == UADD) && (variable->nbAverage > 1))
      variable->nbAverage = 1;

    ///////////////////////////////
    // Process the active modifiers
    if (processModifiers(connection, currentTime) == UFAIL)
      return (status = UCOMPLETED);
    ///////////////////////////////

    // absorb average and set reinit list to set nbAverage back to 0 after work()
    if (variable->blendType != UADD)
      *valtmp = *valtmp / (double)(variable->nbAverage+1);          
    variable->nbAverage++;

    if (variable->activity == 0)   
      connection->server->reinitList.push_front(variable);
    variable->activity = 1;
    
    // Variable updating or signal for update (modified)
    // UMIX and UADD are treated separatly in the processing of the reinit list
    // because we don't want to have several calls to notifyWrite (when the 
    // variable is with a notifyWrite==true flag) for each intermediary step
    // of the UADD and UMIX aggregation, but only at the end. Hence the report to
    // reinit list processing.

    if ((variable->blendType != UMIX) && (variable->blendType != UADD))
      variable->selfSet(valtmp);

    first = false;
    
    if (finished)
      if (variable->speedmax != UINFINITY)        
        return (status = URUNNING);
      else
        return (status = UCOMPLETED); 
    else
      return (status = URUNNING);
  }  
}

// Processing the modifiers in a URUNNING assignment
UErrorValue
UCommand_ASSIGN_VALUE::processModifiers(UConnection* connection, double currentTime) 
{
  double deltaTime = connection->server->getFrequency();
  double currentVal = variable->get()->val;
  double phase,amplitude;
  
  // Adaptive mode? (only for "speed" and "time")
  double adaptive = false;
  if (modif_adaptive)
    if (tmpeval = modif_adaptive->eval(this,connection)) {
      adaptive = (tmpeval->val != 0);
      delete tmpeval;
    }
      
  // timeout     
  if ( (endtime != -1) && (currentTime >= endtime) ) {  
    finished = true;
    *valtmp = variable->nbAverage * *valtmp + currentVal;
    return(USUCCESS);
  }

  // speedmin conversion for convenience
  speedmin = variable->speedmin / 1000.;

  // time
  if (modif_time) {
    
    if (adaptive)
      if (ABSF(currentVal - targetval) <= variable->delta) {
        finished = true;
        *valtmp = variable->nbAverage * *valtmp +
          targetval;
      };
    
    if (tmpeval = modif_time->eval(this,connection)) {
      targettime = ABSF(tmpeval->val);
      delete tmpeval;
    }
    
    // check for speedmin        
    if ( (targettime > (currentTime - starttime)) &&
         (ABSF((targetval - currentVal) / 
               (targettime - (currentTime - starttime))) < speedmin)) {
      
      targettime = currentTime - starttime + 
        ABSF(targetval - currentVal)/ speedmin;
      
      if ((errorFlag) && (first))
        connection->send("!!! low speed: increased to speedmin\n",tag->str());
    }
    
    if (currentTime - starttime + deltaTime >= targettime) {           
      if (!adaptive) finished = true;
      *valtmp = variable->nbAverage * *valtmp +
        targetval;
    }
    else 
      if (adaptive)
        *valtmp = variable->nbAverage * *valtmp +
          currentVal +
          deltaTime*
          ( (targetval - currentVal) / 
            (targettime - (currentTime - starttime)) );          
      else
        *valtmp = variable->nbAverage * *valtmp +
          startval + 
          (currentTime - starttime + deltaTime)*
          ( (targetval - startval) / 
            targettime );
    
    return(USUCCESS);
  }
  
  // smooth
  if (modif_smooth) {
    
    if (tmpeval = modif_smooth->eval(this,connection)) {
      targettime = ABSF(tmpeval->val);
      delete tmpeval;
    }
    
    // test for speedmin (with linear mvt approximation)
    if ( (targettime > (currentTime - starttime)) &&
         (ABSF((targetval - currentVal) / 
               (targettime - (currentTime - starttime))) < speedmin)) {
      
      targettime = currentTime - starttime + 
        ABSF(targetval - currentVal)/speedmin;
      
      if ((errorFlag) && (first))
        connection->send("!!! low speed: increased to speedmin\n",tag->str());
    }
    
    if (currentTime - starttime + deltaTime >= targettime) {           
      finished = true;
      *valtmp = variable->nbAverage * *valtmp +
        targetval;
    }
    else  
      *valtmp = variable->nbAverage * *valtmp +
        startval + 
        ( (targetval - startval) * 0.5 *
          (1+sin(-0.5*PI + PI*(currentTime - starttime + deltaTime) /
                 targettime 
                 ))
          );        
    return(USUCCESS);
  }
  
  //speed
  if (modif_speed) {
    
    if (adaptive)
      if (ABSF(currentVal - targetval) <= variable->delta) {
        finished = true;        
        *valtmp = variable->nbAverage * *valtmp +
          targetval;
      };
    
    if (tmpeval = modif_speed->eval(this,connection)) {
      speed = ABSF(tmpeval->val);
      delete tmpeval;
    }
    
    if (speed == 0) speed = 0.001;
    
    if (variablename->isnormalized)
      speed = speed * (variable->rangemax - variable->rangemin);              
    
    if (adaptive)
      targettime = currentTime - starttime + 
        ABSF(targetval - currentVal) / (speed/1000.);
    else
      targettime = ABSF(targetval - startval) / (speed/1000.);
    
    
    // test for speedmin
    if ( (targettime > (currentTime - starttime)) &&
         (ABSF((targetval - currentVal) / 
               (targettime - (currentTime - starttime))) < speedmin)) {
      
      targettime = currentTime - starttime + 
        ABSF(targetval - currentVal)/ speedmin;
      
      if ((errorFlag) && (first))
        connection->send("!!! low speed: increased to speedmin\n",tag->str());
    }
    
    if (currentTime - starttime + deltaTime >= targettime) {           
      if (!adaptive) finished = true;
      *valtmp = variable->nbAverage * *valtmp +
        targetval;
    }
    else    
      if (adaptive)
        *valtmp = variable->nbAverage * *valtmp +
          currentVal + 
          deltaTime*
          ( (targetval - currentVal) / 
            (targettime - (currentTime - starttime)) ); 
      else
        *valtmp = variable->nbAverage * *valtmp +
          startval + 
          (currentTime - starttime + deltaTime)*
          ( (targetval - startval) / 
            targettime );
    
    return(USUCCESS);
  }
  
  //accel
  if (modif_accel) {
    
    if (tmpeval = modif_accel->eval(this,connection)) {
      accel = ABSF(tmpeval->val/1000.);
      delete tmpeval;
    }
    
    if (targetval < startval) accel = -accel;
    
    if (accel == 0) accel = 0.001;
    
    if (variablename->isnormalized)
      accel = accel * (variable->rangemax - variable->rangemin);              
    
    targettime = sqrt ( 2 * ABSF(targetval - startval) / (ABSF(accel)/1000.));        
    
    if (currentTime - starttime + deltaTime >= targettime) {           
      finished = true;
      *valtmp = variable->nbAverage * *valtmp +
        targetval;
    }
    else    
      *valtmp = variable->nbAverage * *valtmp +
        startval + 0.5 * (accel/1000.) * 
        (currentTime - starttime + deltaTime)*
        (currentTime - starttime + deltaTime);        
  }
  
  //sin
  if (modif_sin) {
    
    targettime = 0;
    if (tmpeval = modif_sin->eval(this,connection)) {
      targettime = ABSF(tmpeval->val);
      delete tmpeval;
    }
    if (targettime == 0) targettime = 0.1;
    
    phase = 0;        
    if ((modif_phase) && 
        (tmpeval = modif_phase->eval(this,connection))) {
      phase = tmpeval->val;
      delete tmpeval;
    }
    
    amplitude = 0;
    if ((modif_ampli) && 
        (tmpeval = modif_ampli->eval(this,connection))) {
      amplitude = tmpeval->val; 
      delete tmpeval;
    }
    if (variablename->isnormalized)
      amplitude = amplitude * (variable->rangemax - variable->rangemin);
    
    if ((expression) && 
        (tmpeval = expression->eval(this,connection))) {
      targetval = tmpeval->val; 
      if (variablename->isnormalized) {
        
        if (targetval < 0) targetval = 0;
        if (targetval > 1) targetval = 1;
        
        targetval = variable->rangemin + targetval * 
          (variable->rangemax - variable->rangemin);              
      }
      delete tmpeval;
    }
    
    double intermediary;
    intermediary = targetval + amplitude * sin(phase + 
                                               2*PI*( (currentTime - starttime + deltaTime) / 
                                                      targettime ));
    if (modif_getphase) {
      
      UVariable *phasevari = modif_getphase->getVariable(this,connection);          
      if (!phasevari) {
        if (!modif_getphase->getFullname()) {  
          connection->send("!!! invalid phase variable name\n",tag->str()); 
          
          return( UFAIL );
        }
        phasevari = new UVariable(modif_getphase->getFullname()->str(),0.0);
        connection->localVariableCheck(phasevari);
      }
      
      UValue *phaseval = phasevari->value;         
      
      phaseval->val = (phase + 
                       2*PI*( (currentTime - starttime + deltaTime) / 
                              targettime ));          
      int n = (int)(phaseval->val / (2*PI));
      if (n<0) n--;
      phaseval->val = phaseval->val - n * 2 * PI;
    }    
    
    *valtmp = variable->nbAverage * *valtmp + intermediary;
    
    return(USUCCESS);
  }
}


//! UCommand subclass hard copy function
UCommand*
UCommand_ASSIGN_VALUE::copy() 
{  
  UVariableName*    copy_variable;
  UExpression*      copy_expression;
  UNamedParameters* copy_parameters;
  
  if (variablename) copy_variable = variablename->copy(); else copy_variable = 0;
  if (expression) copy_expression = expression->copy(); else copy_expression = 0;
  if (parameters) copy_parameters = parameters->copy(); else copy_parameters = 0;

  UCommand_ASSIGN_VALUE *ret = new UCommand_ASSIGN_VALUE(copy_variable,
                                                         copy_expression,
                                                         copy_parameters);
  
  copybase(ret);
  ret->defkey = defkey;
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_ASSIGN_VALUE::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] toDelete=%d ",tabb,tag->str(),toDelete);}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("ASSIGN VALUE:\n");

  if (variablename) { ::urbiserver->debug("%s  Variable:",tabb); variablename->print(); ::urbiserver->debug("\n");};    
  if (expression) { ::urbiserver->debug("%s  Expr:",tabb); expression->print(); ::urbiserver->debug("\n");}; 
  if (parameters)  { ::urbiserver->debug("%s  Param:{",tabb); parameters->print(); ::urbiserver->debug("}\n");};  
  ::urbiserver->debug("%sEND ASSIGN VALUE ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_ASSIGN_BINARY::UCommand_ASSIGN_BINARY(UVariableName *variablename,  
                                               URefPt<UBinary> *refBinary) :
  UCommand(CMD_ASSIGN_BINARY)
{	
  ADDOBJ(UCommand_ASSIGN_BINARY);
  this->variablename = variablename;
  this->refBinary    = refBinary;
  variable           = 0;
  method             = 0;
  devicename         = 0;
}

//! UCommand subclass destructor.
UCommand_ASSIGN_BINARY::~UCommand_ASSIGN_BINARY()
{ 
  FREEOBJ(UCommand_ASSIGN_BINARY);
  if (variablename) delete variablename;
  LIBERATE(refBinary);
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_ASSIGN_BINARY::execute(UConnection *connection)
{
  HMgrouptab::iterator hmg;

  // General initializations
  if (!variable) {
    variable = variablename->getVariable(this,connection); 
    if (!variablename->getFullname()) return ( status = UCOMPLETED );  
    method = variablename->getMethod();
    devicename = variablename->getDevice();      
  }

  // Handling of groups and implicit multi assignments
  // (using morhping and & conjunction)
  if ((!variablename->rooted) && (devicename)) {

    UGroup *gp = 0;
    if ((hmg = ::urbiserver->grouptab.find(devicename->str())) !=
        ::urbiserver->grouptab.end()) 
      gp = (*hmg).second;    
        
    if ((gp) && (gp->members.size() > 0)) {
        
      UCommand *grouplist;
      UCommand *grouplist_prev = 0;
      UCommand_ASSIGN_BINARY *clone;
      UNamedParameters *varindex;

      for (list<UGroup*>::iterator retr = gp->members.begin();
           retr != gp->members.end();
           retr++) {

        clone = (UCommand_ASSIGN_BINARY*)this->copy();
        delete clone->variablename;
        if (variablename->index) 
          varindex = variablename->index->copy();
        else
          varindex = 0;

        clone->variablename = new UVariableName((*retr)->device->copy(),
                                        method->copy(),
                                        false,
                                        varindex);
        grouplist = (UCommand*)
          new UCommand_TREE(UAND,
                            (UCommand*)clone,
                            grouplist_prev);
        grouplist_prev = grouplist;
      }

      morph = (UCommand*) 
        new UCommand_TREE(UAND,
                          this,
                          grouplist);
      
      variablename->rooted = true;
      variablename->fromGroup = true;
      persistant = true;
      return( status = UMORPH );
    }
  }

  // Type checking
  UValue *value;

  if ((variable) && (variable->value->dataType != DATA_BINARY)) {

      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! %s type mismatch\n",variablename->getFullname()->str());
      connection->send(tmpbuffer,tag->str()); 
      
      return( status = UCOMPLETED );
    }   

  // Create variable if it doesn't exist
  if (!variable) {           

    value = new UValue();
    value->dataType = DATA_BINARY; 
    variable = new UVariable(variablename->getFullname()->str(), value);
    if (!variable) return( status = UCOMPLETED );
    variable->blendType = UQUEUE;

    connection->localVariableCheck(variable);
  }
  else 
    LIBERATE(variable->value->refBinary);

  variable->value->refBinary = refBinary->copy();

  if ((variable->dev) && (variable->notifyWrite))
    variable->dev->notifyWrite(variable);

  return( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_ASSIGN_BINARY::copy() 
{  
  UVariableName*        copy_variable;
  
  if (variablename)   copy_variable = variablename->copy(); else copy_variable = 0;

  UCommand_ASSIGN_BINARY *ret = new UCommand_ASSIGN_BINARY(copy_variable,
                                                           refBinary->copy());
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_ASSIGN_BINARY::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("ASSIGN BINARY:\n");

  if (variablename) { ::urbiserver->debug("%s  Variable:",tabb); variablename->print(); ::urbiserver->debug("\n");};     
  if (refBinary) { ::urbiserver->debug("%s  Binary:",tabb); refBinary->ref()->print(); ::urbiserver->debug("\n");};   
  ::urbiserver->debug("%sEND ASSIGN BINARY ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_ASSIGN_PROPERTY::UCommand_ASSIGN_PROPERTY(UVariableName *variablename,  
                                                   UString *oper,
                                                   UExpression *expression) :
  UCommand(CMD_ASSIGN_PROPERTY)
{	
  ADDOBJ(UCommand_ASSIGN_PROPERTY);
  this->variablename = variablename;
  this->oper         = oper;
  this->expression   = expression;
  variable           = 0;
  method             = 0;
  devicename         = 0;
}

//! UCommand subclass destructor.
UCommand_ASSIGN_PROPERTY::~UCommand_ASSIGN_PROPERTY()
{ 
  FREEOBJ(UCommand_ASSIGN_PROPERTY);
  if (variablename) delete variablename; 
  if (expression) delete expression; 
  if (oper)       delete oper;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_ASSIGN_PROPERTY::execute(UConnection *connection)
{
  HMgrouptab::iterator hmg;

  UVariable* variable = variablename->getVariable(this,connection); 
  if (!variablename->getFullname()) return ( status = UCOMPLETED );  
  UString* method = variablename->getMethod();
  UString* devicename = variablename->getDevice();

  // Handling of groups and implicit multi assignments
  // (using morhping and & conjunction)
  if ((!variablename->rooted) && (devicename))  {

    UGroup *gp = 0;
    if ((hmg = ::urbiserver->grouptab.find(devicename->str())) !=
        ::urbiserver->grouptab.end()) 
      gp = (*hmg).second; 
        
    if ((gp) && (gp->members.size() > 0)) {
        
      UCommand *grouplist;
      UCommand *grouplist_prev = 0;
      UCommand_ASSIGN_PROPERTY *clone;
      UNamedParameters *varindex;

      for (list<UGroup*>::iterator retr = gp->members.begin();
           retr != gp->members.end();
           retr++) {

        clone = (UCommand_ASSIGN_PROPERTY*)this->copy();
        delete clone->variablename;
        if (variablename->index) 
          varindex = variablename->index->copy();
        else
          varindex = 0;

        clone->variablename = new UVariableName((*retr)->device->copy(),
                                        method->copy(),
                                        false,
                                        varindex);
        grouplist = (UCommand*)
          new UCommand_TREE(UAND,
                            (UCommand*)clone,
                            grouplist_prev);
        grouplist_prev = grouplist;
      }

      morph = (UCommand*) 
        new UCommand_TREE(UAND,
                          this,
                          grouplist);
      
      variablename->rooted = true;
      variablename->fromGroup = true;
      persistant = true;
      return( status = UMORPH );
    }
  }

  // variable existence checking
  if (!variable) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Variable %s does not exist\n",variablename->getFullname()->str()); 
      if (!variablename->fromGroup)
        connection->send(tmpbuffer,tag->str());
      return( status = UCOMPLETED );
  }
  
  // Property handling


  // blend
  if (strcmp(oper->str(),"blend")==0) {
        
    UValue *blendmode = expression->eval(this,connection); 
    if (blendmode == 0) 
      return( status = UCOMPLETED );
    
    if (blendmode->dataType != DATA_STRING) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Invalid blend mode.\n");
      connection->send(tmpbuffer,tag->str());
      return( status = UCOMPLETED );
    }

    if ((variable->value->dataType != DATA_NUM) &&
        (variable->value->dataType != DATA_BINARY)) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! %s type is invalid for mixing\n",
                variablename->getFullname()->str());
      connection->send(tmpbuffer,tag->str());
      return( status = UCOMPLETED );   
    } 

    if (strcmp(blendmode->str->str(),"normal")==0)     variable->blendType = UNORMAL;
    else
    if (strcmp(blendmode->str->str(),"mix")==0)        variable->blendType = UMIX;
    else
    if (strcmp(blendmode->str->str(),"add")==0)        variable->blendType = UADD;
    else
    if (strcmp(blendmode->str->str(),"discard")==0)    variable->blendType = UDISCARD;
    else
    if (strcmp(blendmode->str->str(),"queue")==0)      variable->blendType = UQUEUE;
    else
    if (strcmp(blendmode->str->str(),"cancel")==0)     variable->blendType = UCANCEL;    
    else {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Unknown blend mode: %s\n",
                blendmode->str->str());
      connection->send(tmpbuffer,tag->str());
      return( status = UCOMPLETED );   
    }

    return( status = UCOMPLETED );   
  }

  // rangemax
  if (strcmp(oper->str(),"rangemax")==0) {
        
    UValue *nb = expression->eval(this,connection); 
    if (nb == 0) 
      return( status = UCOMPLETED );
    
    if (nb->dataType != DATA_NUM) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Invalid range type. NUM expected.\n");
      connection->send(tmpbuffer,tag->str());
      return( status = UCOMPLETED );
    }

    variable->rangemax = nb->val;
    return( status = UCOMPLETED );
  }

  // delta
  if (strcmp(oper->str(),"delta")==0) {
        
    UValue *nb = expression->eval(this,connection); 
    if (nb == 0) 
      return( status = UCOMPLETED );
    
    if (nb->dataType != DATA_NUM) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Invalid delta type. NUM expected.\n");
      connection->send(tmpbuffer,tag->str());
      return( status = UCOMPLETED );
    }

    variable->delta = nb->val;
    return( status = UCOMPLETED );
  }


  // unit
  if (strcmp(oper->str(),"unit")==0) {
        
    UValue *unitval = expression->eval(this,connection); 
    if (unitval == 0) 
      return( status = UCOMPLETED );
    
    if (unitval->dataType != DATA_STRING) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Invalid unit type (must be a string).\n");
      connection->send(tmpbuffer,tag->str());
      return( status = UCOMPLETED );
    }

    if ((variable->value->dataType != DATA_NUM) &&
        (variable->value->dataType != DATA_BINARY)) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! %s type is invalid for unit attribution\n",
                variablename->getFullname()->str());
      connection->send(tmpbuffer,tag->str());
      return( status = UCOMPLETED );   
    } 

    if (variable->unit) variable->unit->update(unitval->str->str());
    else
      variable->unit = new UString(unitval->str->str());

    return( status = UCOMPLETED );   
  }

  // rangemin
  if (strcmp(oper->str(),"rangemin")==0) {
        
    UValue *nb = expression->eval(this,connection); 
    if (nb == 0) 
      return( status = UCOMPLETED );
    
    if (nb->dataType != DATA_NUM) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Invalid range type. NUM expected.\n");
      connection->send(tmpbuffer,tag->str());
      return( status = UCOMPLETED );
    }

    variable->rangemin = nb->val;
    return( status = UCOMPLETED );
  }

  // speedmax
  if (strcmp(oper->str(),"speedmax")==0) {
        
    UValue *nb = expression->eval(this,connection); 
    if (nb == 0) 
      return( status = UCOMPLETED );
    
    if (nb->dataType != DATA_NUM) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Invalid speed type. NUM expected.\n");
      connection->send(tmpbuffer,tag->str());
      return( status = UCOMPLETED );
    }

    variable->speedmax = nb->val;
    return( status = UCOMPLETED );
  }

  // speedmin
  if (strcmp(oper->str(),"speedmin")==0) {
        
    UValue *nb = expression->eval(this,connection); 
    if (nb == 0) 
      return( status = UCOMPLETED );
    
    if (nb->dataType != DATA_NUM) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Invalid speed type. NUM expected.\n");
      connection->send(tmpbuffer,tag->str());
      return( status = UCOMPLETED );
    }

    variable->speedmin = nb->val;
    return( status = UCOMPLETED );
  }

  snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
           "!!! Unknown property: %s\n",oper->str());
  connection->send(tmpbuffer,tag->str());
  return( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_ASSIGN_PROPERTY::copy() 
{  
  UVariableName*        copy_variable;
  UExpression*          copy_expression;
  UString*              copy_oper;
  
  if (variablename)   copy_variable = variablename->copy(); else copy_variable = 0;
  if (expression) copy_expression = expression->copy(); else copy_expression = 0;
  if (oper) copy_oper = new UString(oper); else copy_oper = 0;

  UCommand_ASSIGN_PROPERTY *ret = new UCommand_ASSIGN_PROPERTY(copy_variable,
                                                               copy_oper,
                                                               copy_expression);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_ASSIGN_PROPERTY::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("ASSIGN PROPERTY [%s]:\n",oper->str());

  if (variablename) { ::urbiserver->debug("%s  Variable:",tabb); variablename->print(); ::urbiserver->debug("\n");};     
  if (expression) { ::urbiserver->debug("%s  Expr:",tabb); expression->print(); ::urbiserver->debug("\n");}; 

  ::urbiserver->debug("%sEND ASSIGN PROPERTY ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_EXPR::UCommand_EXPR(UExpression* expression) :
  UCommand(CMD_EXPR)
{	
  ADDOBJ(UCommand_EXPR);
  this->expression  = expression;
}

//! UCommand subclass destructor.
UCommand_EXPR::~UCommand_EXPR()
{  
  FREEOBJ(UCommand_EXPR);
  if (expression) delete expression;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_EXPR::execute(UConnection *connection)
{
  HMfunctiontab::iterator hmf;

  if (expression->type == EXPR_FUNCTION) {
    // implement group-morphing

    // Execution & morphing
    UString* funname = expression->variablename->buildFullname(this,connection);
    if (!funname) return( status = UCOMPLETED );

    UFunction *fun;
    
    if ((hmf = ::urbiserver->functiontab.find(funname->str())) !=
        ::urbiserver->functiontab.end()) {
  
      fun = (*hmf).second;

      if ( ( (expression->parameters) && 
             (fun->nbparam()) && 
             (expression->parameters->size() != fun->nbparam())) ||
           ( (expression->parameters) && (!fun->nbparam())) ||
           ( (!expression->parameters) && (fun->nbparam())) ) {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "!!! invalid number of arguments for %s (should be %d params)\n",
                 funname->str(),fun->nbparam());
        connection->send(tmpbuffer,tag->str()); 
        
        return( status = UCOMPLETED );
      }

      persistant = false;
      morph = (UCommand*) 
        new UCommand_TREE(UPIPE,
                          fun->cmdcopy(tag),
                          new UCommand_NOOP(true)); 
      if (morph) {
        
        morph->morphed = true;
        if (tag) {
          morph->tag->update(tag->str());
          if (flags)
            morph->flags = flags->copy();
        }
        
        sprintf(tmpbuffer,"__UFnct%d",(int)morph);
        ((UCommand_TREE*)morph)->callid = new UCallid(tmpbuffer,(UCommand_TREE*)morph);
        if (!((UCommand_TREE*)morph)->callid) return (status = UCOMPLETED);
        ((UCommand_TREE*)morph)->connection = connection;
        
        UNamedParameters *pvalue = expression->parameters;
        UNamedParameters *pname  = fun->parameters;
        for (;
             pvalue != 0;
             pvalue = pvalue->next, pname = pname->next) {

          UValue* valparam = pvalue->expression->eval(this,connection);
          if (!valparam) {
              
            connection->send("!!! EXPR evaluation failed\n",tag->str());
            return (status = UCOMPLETED);
          }
          snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                   "%s.%s",((UCommand_TREE*)morph)->callid->str(),pname->name->str());         

          ((UCommand_TREE*)morph)->callid->store(
                   new UVariable(((UCommand_TREE*)morph)->callid->str(),
                                 pname->name->str(),
                                 valparam)
                   );         
        }
      }
      
      return( status = UMORPH );
    }
    else
      if ((connection->receiving) &&
          (expression->variablename->id->equal("exec")))
        return ( status = URUNNING);
  }

  // Normal expression (no function)

  UValue* ret = expression->eval(this,connection); 
  
  // Expression morphing (currently used for load only)
  if (morph) {
    if (ret) delete ret;
    return( status = UMORPH);
  }

  if (ret==0) {   
    connection->send("!!! EXPR evaluation failed\n",tag->str());
    return (status = UCOMPLETED);
  }  

  if (ret->dataType == DATA_VOID) {
    delete ret;    
    return(status = UCOMPLETED);
  }

  if (ret->dataType == DATA_LIST) {
    
    UValue *scanlist = ret->list;
    char tmpexpr[255];
    sprintf(tmpbuffer,"[");
    while (scanlist) {
      tmpexpr[0]=0;
      if (scanlist->dataType == DATA_NUM) snprintf(tmpexpr,254,"%f",scanlist->val); 
      if (scanlist->dataType == DATA_STRING) snprintf(tmpexpr,254,"\"%s\"",scanlist->str->str()); 
      if (scanlist->dataType == DATA_BINARY) snprintf(tmpexpr,254,"BIN %d",scanlist->refBinary->ref()->bufferSize); 
      scanlist = scanlist->list;
      strcat(tmpbuffer,tmpexpr);
      if (scanlist) strcat(tmpbuffer,",");
    }
    strcat(tmpbuffer,"]\n");
  }

  if (ret->dataType == DATA_NUM)
    sprintf(tmpbuffer,"%f\n",ret->val); 

  if (ret->dataType == DATA_STRING)
    snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
             "\"%s\"\n",ret->str->str());

  if (ret->dataType == DATA_FILE)
    snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
             "FILE %s\n",ret->str->str());

  if (ret->dataType == DATA_BINARY) {
    if (ret->refBinary) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "BIN %d ",ret->refBinary->ref()->bufferSize);
      UNamedParameters *param = ret->refBinary->ref()->parameters;
      char tmpparam[1024];
      while (param) {     
        if (param->expression) {
          if (param->expression->dataType == DATA_NUM)
            sprintf(tmpparam,"%d ",(int)param->expression->val);
          if (param->expression->dataType == DATA_STRING)
            sprintf(tmpparam,"%s ",param->expression->str->str());
                      
          strcat(tmpbuffer,tmpparam);
        }
        param = param->next;
      }
      strcat(tmpbuffer,"\n");
      
      if (connection->availableSendQueue() > 
          strlen(tmpbuffer) + 
          ret->refBinary->ref()->bufferSize +1) {
          
        connection->send(tmpbuffer,tag->str());
        connection->send(ret->refBinary->ref()->buffer,
                         ret->refBinary->ref()->bufferSize);
      }
      else
        ::urbiserver->debug("Send queue full for binary... Drop command.\n");
      
      delete(ret);
      return(status = UCOMPLETED); 
    }
    else
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "BIN 0 null\n");
  }

  delete(ret);

  connection->send(tmpbuffer,tag->str());

  return(status = UCOMPLETED);  
}

//! UCommand subclass hard copy function
UCommand*
UCommand_EXPR::copy() 
{  
  UExpression*      copy_expression;
  
  if (expression) copy_expression = expression->copy(); else copy_expression = 0;

  UCommand_EXPR *ret = new UCommand_EXPR(copy_expression);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_EXPR::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("EXPR:\n"); 

  if (expression) { ::urbiserver->debug("%s  Expr:",tabb); expression->print(); ::urbiserver->debug("\n");}; 

  ::urbiserver->debug("%sEND EXPR ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_RETURN::UCommand_RETURN(UExpression* expression) :
  UCommand(CMD_RETURN)
{	
  ADDOBJ(UCommand_RETURN);
  this->expression  = expression;
}

//! UCommand subclass destructor.
UCommand_RETURN::~UCommand_RETURN()
{
  FREEOBJ(UCommand_RETURN);
  if (expression) delete expression;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_RETURN::execute(UConnection *connection)
{
  if (!connection->stack.empty()) {
    connection->returnMode = true;
    if (expression) {
      UValue *value = expression->eval(this,connection);
      if (!value) {
              
        connection->send("!!! EXPR evaluation failed\n",tag->str());
        return (status = UCOMPLETED);
      }
      
      new UVariable(connection->stack.front()->str(),
                    "__result__", 
                    value);   
    }
  }
  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_RETURN::copy() 
{  
  UExpression*      copy_expression;
  
  if (expression) copy_expression = expression->copy(); else copy_expression = 0;

  UCommand_RETURN *ret = new UCommand_RETURN(copy_expression);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_RETURN::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("RETURN:\n"); 

  if (expression) { ::urbiserver->debug("%s  Expr:",tabb); expression->print(); ::urbiserver->debug("\n");}; 

  ::urbiserver->debug("%sEND RETURN ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_ECHO::UCommand_ECHO(UExpression* expression, 
                             UNamedParameters *parameters,
                             UString *connectionTag) :
  UCommand(CMD_ECHO)
{	
  ADDOBJ(UCommand_ECHO);
  this->expression  = expression;
  this->parameters  = parameters;
  this->connectionTag = connectionTag;
}

//! UCommand subclass destructor.
UCommand_ECHO::~UCommand_ECHO()
{
  FREEOBJ(UCommand_ECHO);
  if (expression) delete expression;
  if (parameters) delete parameters;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_ECHO::execute(UConnection *connection)
{
  UValue* ret = expression->eval(this,connection); 
  
  if (ret==0) {   
    connection->send("!!! EXPR evaluation failed\n",tag->str()); 
    
    return (status = UCOMPLETED);
  }

  if (ret->dataType == DATA_NUM)
    sprintf(tmpbuffer,"*** %f\n",ret->val); 

  if (ret->dataType == DATA_LIST) {
    
    UValue *scanlist = ret->list;
    char tmpexpr[255];
    sprintf(tmpbuffer,"*** [");
    while (scanlist) {
      tmpexpr[0]=0;
      if (scanlist->dataType == DATA_NUM) snprintf(tmpexpr,254,"%f",scanlist->val); 
      if (scanlist->dataType == DATA_STRING) snprintf(tmpexpr,254,"\"%s\"",scanlist->str->str());    
      if (scanlist->dataType == DATA_BINARY) snprintf(tmpexpr,254,"BIN %d",scanlist->refBinary->ref()->bufferSize);   
      scanlist = scanlist->list;
      strcat(tmpbuffer,tmpexpr);
      if (scanlist) strcat(tmpbuffer,",");
    }
    strcat(tmpbuffer,"]\n");
  }

  if (ret->dataType == DATA_STRING) {
   
    snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
             "*** %s\n",ret->str->str());
  }
  if (ret->dataType == DATA_FILE)
    snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
             "*** FILE %s\n",ret->str->str());
  if (ret->dataType == DATA_BINARY) {
    if (ret->refBinary) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "*** BIN %d ",ret->refBinary->ref()->bufferSize);
      UNamedParameters *param = ret->refBinary->ref()->parameters;
      char tmpparam[1024];
      while (param) {     
        if (param->expression) {
          if (param->expression->dataType == DATA_NUM)
            sprintf(tmpparam,"%d ",(int)param->expression->val);
          if (param->expression->dataType == DATA_STRING)
            sprintf(tmpparam,"%s ",param->expression->str->str());
                      
          strcat(tmpbuffer,tmpparam);
        }
        param = param->next;
      }
      strcat(tmpbuffer,"\n");
    }
    else
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "*** BIN 0 null\n");
  }

  delete(ret);
  
  UNamedParameters *param = parameters;
  while (param) {
    if (param->name->equal("connection")) {
      UValue *e1 = param->expression->eval(this, connection);
      if ((e1) && (e1->dataType == DATA_STRING)) 
        connectionTag = new UString(e1->str);
      
      if (e1) delete e1;
    }
    param = param->next;
  }

  if (!connectionTag)
    connection->send(tmpbuffer,tag->str());
  else {

    bool ok = false;

    // Scan currently opened connections to locate the connection with the 
    // appropriate tag (connectionTag)
    for (list<UConnection*>::iterator retr = connection->server->connectionList.begin();
         retr != connection->server->connectionList.end();
         retr++) 
      if  ( ((*retr)->isActive()) &&
            ( ((*retr)->connectionTag->equal(connectionTag)) ||
              (connectionTag->equal("all")) ||
              ( (!(*retr)->connectionTag->equal(connection->connectionTag)) &&
                (connectionTag->equal("other")) ) )
            ) {            
        ok = true;
        (*retr)->send(tmpbuffer,tag->str());    
      }

    if (!ok) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! %s: no such connection\n",connectionTag->str());
      connection->send(tmpbuffer,tag->str());
    
      return (status = UCOMPLETED);
    }
  }
    
  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_ECHO::copy() 
{  
  UExpression*      copy_expression;
  UNamedParameters* copy_parameters;
  UString*          copy_connectionTag;
  
  if (connectionTag)  copy_connectionTag = new UString(connectionTag); else copy_connectionTag = 0;
  if (expression) copy_expression = expression->copy(); else copy_expression = 0;
  if (parameters) copy_parameters = parameters->copy(); else copy_parameters = 0;
  

  UCommand_ECHO *ret = new UCommand_ECHO(copy_expression,
                                         copy_parameters,
                                         copy_connectionTag);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_ECHO::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("ECHO:\n"); 

  if (expression) { ::urbiserver->debug("%s  Expr:",tabb); expression->print(); ::urbiserver->debug("\n");}; 
  if (parameters) { ::urbiserver->debug("%s  Param:{",tabb); parameters->print(); ::urbiserver->debug("}\n");};  

  ::urbiserver->debug("%sEND ECHO ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_GROUP::UCommand_GROUP(UString* id,  
                               UNamedParameters *parameters) :
  UCommand(CMD_GROUP)
{	
  ADDOBJ(UCommand_GROUP);
  this->id          = id;
  this->parameters  = parameters;
}

//! UCommand subclass destructor.
UCommand_GROUP::~UCommand_GROUP()
{
  FREEOBJ(UCommand_GROUP);
  if (id)         delete id;
  if (parameters) delete parameters;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_GROUP::execute(UConnection *connection)
{
  if (!id) return ( status = UCOMPLETED );
  UGroup *gp,*mbr;
  UNamedParameters *param;

  if (parameters) {
   
    if (connection->server->grouptab.find(id->str()) !=
        connection->server->grouptab.end()) {
      
      gp = connection->server->grouptab[id->str()];
      gp->members.clear();
    }
    else {
      
      gp = new UGroup(id);
      connection->server->grouptab[gp->device->str()] = gp;
    }

    param = parameters;
    while (param) {
      
      if (connection->server->grouptab.find(param->name->str()) !=
          connection->server->grouptab.end()) 
      
        mbr = connection->server->grouptab[param->name->str()];      
      else {
      
        mbr = new UGroup(param->name);
        connection->server->grouptab[mbr->device->str()] = mbr;
      }
      
      gp->members.push_back(mbr);
      param = param->next;
    }
  }
  else {
    
    if (connection->server->grouptab.find(id->str()) !=
        connection->server->grouptab.end()) 
      gp = connection->server->grouptab[id->str()];
    else {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Unkown device %s\n",id->str());
      connection->send(tmpbuffer,tag->str());
      return ( status = UCOMPLETED );
    }

    for (list<UGroup*>::iterator retr = gp->members.begin();
         retr != gp->members.end();
         retr++) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "*** %s\n",(*retr)->device->str());
      connection->send(tmpbuffer,tag->str());
    }      
  }

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_GROUP::copy() 
{  
  UNamedParameters* copy_parameters;
  UString* copy_id;
  
  if (id)   copy_id = new UString(id); else copy_id = 0;
  if (parameters) copy_parameters = parameters->copy(); else copy_parameters = 0;

  UCommand_GROUP *ret = new UCommand_GROUP(copy_id,
                                           copy_parameters);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_GROUP::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("GROUP:\n"); 

  if (id)  { ::urbiserver->debug("%s  Id:[%s]\n",tabb,id->str());}  
  if (parameters) { ::urbiserver->debug("%s  Param:{",tabb); parameters->print(); ::urbiserver->debug("}\n");};  

  ::urbiserver->debug("%sEND GROUP ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_ALIAS::UCommand_ALIAS(UVariableName* id,
                               UVariableName* variablename) :
  UCommand(CMD_ALIAS)
{	
  ADDOBJ(UCommand_ALIAS);
  this->id           = id;
  this->variablename = variablename;
}

//! UCommand subclass destructor.
UCommand_ALIAS::~UCommand_ALIAS()
{
  FREEOBJ(UCommand_ALIAS);
  if (id)            delete id;
  if (variablename)  delete variablename;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_ALIAS::execute(UConnection *connection)
{
  if ((id) && (variablename)) {

    const char* idname = id->buildFullname(this,connection,false)->str(); // false = no alias following
    const char* variablenamestr = variablename->buildFullname(this,connection)->str();

    if (connection->server->aliastab.find(idname) !=
        connection->server->aliastab.end()) {
      
      UString *alias = connection->server->aliastab[idname];
      alias->update(variablenamestr);
    }
    else {
      
      UString *ids = new UString(idname); // persistant, no delete associated
      connection->server->aliastab[ids->str()] = new UString(variablenamestr);
    }
  }
  else {
    
     for ( hash_map<const char*,
            UString*,
            hash<const char*>,
            eqStr>::iterator retr = 
            connection->server->aliastab.begin();
          retr != connection->server->aliastab.end();
          retr++) {

       snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                "*** %s -> %s\n",
                (*retr).first,(*retr).second->str());
              
       connection->send(tmpbuffer,tag->str());
     }
  }

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_ALIAS::copy() 
{  
  UVariableName* copy_variable;
  UVariableName* copy_id;
  
  if (variablename) copy_variable = variablename->copy(); else copy_variable = 0;
  if (id) copy_id = id->copy(); else copy_id = 0;

  UCommand_ALIAS *ret = new UCommand_ALIAS(copy_id,
                                           copy_variable);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_ALIAS::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("ALIAS :\n");
  if (id) { ::urbiserver->debug("  %s  ID:",tabb); id->print(); ::urbiserver->debug("\n");};      
  if (variablename) { ::urbiserver->debug("  %s  Variablename:",tabb); variablename->print(); ::urbiserver->debug("\n");};      

  ::urbiserver->debug("%sEND ALIAS ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_OPERATOR_ID::UCommand_OPERATOR_ID(UString* oper,
                                           UString* id) :
  UCommand(CMD_GENERIC)
{	
  ADDOBJ(UCommand_OPERATOR_ID);
  this->oper        = oper;
  this->id          = id;
}

//! UCommand subclass destructor.
UCommand_OPERATOR_ID::~UCommand_OPERATOR_ID()
{
  FREEOBJ(UCommand_OPERATOR_ID);
  if (oper)       delete oper;
  if (id)         delete id;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_OPERATOR_ID::execute(UConnection *connection)
{
  if (strcmp(oper->str(),"stop")==0) {
    
    if (status == URUNNING)
      return ( status = UCOMPLETED);
    connection->server->mark(id);
    connection->server->somethingToDelete = true;
    return( status = URUNNING );
  }

  if (strcmp(oper->str(),"killall")==0) {
          
    bool ok = false;

    // Scan currently opened connections to locate the connection with the 
    // appropriate tag (connectionTag)
    for (list<UConnection*>::iterator retr = connection->server->connectionList.begin();
         retr != connection->server->connectionList.end();
         retr++) 
      if  ( ((*retr)->isActive()) &&
            ((*retr)->connectionTag->equal(id))) {
        ok = true;
        (*retr)->killall = true;
      }
    
    if (!ok) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! %s: no such connection\n",id->str());
      connection->send(tmpbuffer,tag->str());
    
      return (status = UCOMPLETED);
    }
    return( status = UCOMPLETED );
  }

  if (strcmp(oper->str(),"disconnect")==0) {
          
    bool ok = false;

    // Scan currently opened connections to locate the connection with the 
    // appropriate tag (connectionTag)
    for (list<UConnection*>::iterator retr = connection->server->connectionList.begin();
         retr != connection->server->connectionList.end();
         retr++) 
      if  ( ((*retr)->isActive()) &&
            ((*retr)->connectionTag->equal(id))) {
        ok = true;
        (*retr)->disactivate();
        (*retr)->closeConnection();
      }

    if (!ok) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! %s: no such connection\n",id->str());
      connection->send(tmpbuffer,tag->str());
    
      return (status = UCOMPLETED);
    }
    return( status = UCOMPLETED );
  }

  if (strcmp(oper->str(),"block")==0) {
    
    if (status == URUNNING)
      return ( status = UCOMPLETED);
          
    if (strcmp(id->str(),UNKNOWN_TAG)==0) {
      
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! cannot block 'notag'\n",id->str());
      connection->send(tmpbuffer,tag->str());
    }
    else {
      UString* id_permanent = new UString(id->str()); // no delete, to keep the index
      connection->server->blocktab[id_permanent->str()] = true;
    }

    return( status = URUNNING );
  }

  if (strcmp(oper->str(),"unblock")==0) {
          
    if (connection->server->blocktab.find(id->str()) !=
        connection->server->blocktab.end())
      connection->server->blocktab[id->str()] = false;

    return( status = UCOMPLETED );
  }

  if (strcmp(oper->str(),"freeze")==0) {
          
    if (status == URUNNING)
      return ( status = UCOMPLETED);

    if (strcmp(id->str(),UNKNOWN_TAG)==0) {
      
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! cannot freeze 'notag'\n",id->str());
      connection->send(tmpbuffer,tag->str());
    }
    else {
      UString* id_permanent = new UString(id->str()); // no delete, to keep the index
      connection->server->freezetab[id_permanent->str()] = true;
    }

    return( status = URUNNING );
  }

  if (strcmp(oper->str(),"unfreeze")==0) {
          
    if (connection->server->freezetab.find(id->str()) !=
        connection->server->freezetab.end())
      connection->server->freezetab[id->str()] = false;

    return( status = UCOMPLETED );
  }

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_OPERATOR_ID::copy() 
{  
  UString* copy_id;
  UString* copy_oper;
  
  if (id)   copy_id   = new UString(id); else copy_id = 0;
  if (oper) copy_oper = new UString(oper); else copy_oper = 0;

  UCommand_OPERATOR_ID *ret = new UCommand_OPERATOR_ID(copy_oper,
                                                       copy_id);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_OPERATOR_ID::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("OPERATOR_ID %s:\n",oper->str()); 
 
  if (id)  { ::urbiserver->debug("%s  Id:[%s]\n",tabb,id->str());}  

  ::urbiserver->debug("%sEND OPERATOR_ID ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_DEVICE_CMD::UCommand_DEVICE_CMD( UString* device,
                                          UString* cmd) :
  UCommand(CMD_GENERIC)
{	
  ADDOBJ(UCommand_DEVICE_CMD);
  this->device        = device;
  this->cmd           = cmd;
}

//! UCommand subclass destructor.
UCommand_DEVICE_CMD::~UCommand_DEVICE_CMD()
{
  FREEOBJ(UCommand_DEVICE_CMD);
  if (device)      delete device;
  if (cmd)         delete cmd;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_DEVICE_CMD::execute(UConnection *connection)
{
  if (strcmp(cmd->str(),"on")==0) {
    if (connection->receiving) return (status = URUNNING);
    
    connection->server->motorstate = true;
    connection->server->motor(connection->server->motorstate);
    return( status = UCOMPLETED );
  }
 
  if (strcmp(cmd->str(),"off")==0) {

    connection->server->motorstate = false;
    connection->server->motor(connection->server->motorstate);
    return( status = UCOMPLETED );    
  }

  if (strcmp(cmd->str(),"switch")==0) {
    
    if (connection->receiving) return (status = URUNNING);

    if (connection->server->motorstate)
      connection->server->motorstate = false;
    else
      connection->server->motorstate = true;

    connection->server->motor(connection->server->motorstate);    
    return( status = UCOMPLETED );
  }

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_DEVICE_CMD::copy() 
{  
  UString* copy_device;
  UString* copy_cmd;
  
  if (device)   copy_device   = new UString(device); else copy_device = 0;
  if (cmd) copy_cmd = new UString(cmd); else copy_cmd = 0;

  UCommand_DEVICE_CMD *ret = new UCommand_DEVICE_CMD(copy_device,
                                                       copy_cmd);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_DEVICE_CMD::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("DEVICE_CMD %s:\n",device->str()); 
 
  if (cmd)  { ::urbiserver->debug("%s  Cmd:[%s]\n",tabb,cmd->str());}  

  ::urbiserver->debug("%sEND DEVICE_CMD ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_OPERATOR_VAR::UCommand_OPERATOR_VAR(UString* oper,
                                             UVariableName* variablename) :
  UCommand(CMD_GENERIC)
{	
  ADDOBJ(UCommand_OPERATOR_VAR);
  this->oper         = oper;
  this->variablename = variablename;
}

//! UCommand subclass destructor.
UCommand_OPERATOR_VAR::~UCommand_OPERATOR_VAR()
{
  FREEOBJ(UCommand_OPERATOR_VAR);
  if (oper)         delete oper;
  if (variablename) delete variablename;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_OPERATOR_VAR::execute(UConnection *connection)
{
  UString *fullname = variablename->buildFullname(this,connection);
  if (!fullname) return( status = UCOMPLETED );

  if (strcmp(oper->str(),"undef")==0) {
    
    if (status != URUNNING) {

      variable = 0;
      fun      = variablename->getFunction(this,connection);
      if (!fun)
        variable = variablename->getVariable(this,connection);
            
      if ((!fun) && (!variable)) {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "!!! identifier %s does not exist\n",fullname->str());
        connection->send(tmpbuffer,tag->str());
        return( status = UCOMPLETED );
      }
    }
     
    if (variable) {// undef variable

      if (variable->toDelete) {
        delete variable;
        return( status = UCOMPLETED );
      }

      if ((variable->nbAssigns == 0) && (variable->uservar)) {
        
        variable->toDelete = true;
        return( status = URUNNING );
      }
      else { 
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "!!! variable %s already in use or is a system var. Cannot delete.\n",
                 fullname->str());
        connection->send(tmpbuffer,tag->str());
        return( status = UCOMPLETED );
      }    
    }

    if (fun) { //undef function
        
      connection->server->functiontab.erase(
             connection->server->functiontab.find(fullname->str()));
      
      delete fun;
      return( status = UCOMPLETED );
    }

    return( status = UCOMPLETED );
  }    

  if (strcmp(oper->str(),"info")==0) {
             
    variable = variablename->getVariable(this,connection); 
    if (!variablename->getFullname()) return ( status = UCOMPLETED );  
    UString* method = variablename->getMethod();
    UString* devicename = variablename->getDevice();    
    UDevice* dev = 0;

    if (connection->server->devicetab.find(devicename->str()) !=
        connection->server->devicetab.end())
      dev = connection->server->devicetab[devicename->str()];

    if ((dev==0) && (devicename->equal(connection->connectionTag->str())))
      if (connection->server->devicetab.find(method->str()) !=
          connection->server->devicetab.end())
        dev = connection->server->devicetab[method->str()];

    if ((!variable) && (!dev)) {
      
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Unknown identifier: %s\n",
               variablename->getFullname()->str());     
    
      connection->send(tmpbuffer,tag->str());
      return ( status = UCOMPLETED );
    }

    if ((dev) && (!variable)) 
      variable = dev->device_val;

    if (dev) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "*** device description: %s\n",
               dev->detail->str());
      connection->send(tmpbuffer,tag->str());
    
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "*** device name: %s\n",
               dev->device->str());
      connection->send(tmpbuffer,tag->str());
    }
    
    if (variable) {
      switch (variable->value->dataType) {

      case DATA_NUM: snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                              "*** current value: %f\n",
                              variable->value->val);
                     break;

      case DATA_STRING: snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                                 "*** current value: \"%s\"\n",
                                 variable->value->str->str());
                     break;

      case DATA_BINARY: snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                                 "*** current value: binary\n");                              
                     break;
      }
      connection->send(tmpbuffer,tag->str());
    }
    
    if (dev) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "*** current device load: %f\n",
               dev->device_load->value->val);
      connection->send(tmpbuffer,tag->str());
    }
    
    if (variable) {
      if (variable->rangemin != -UINFINITY)
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "*** rangemin: %f\n",
                 variable->rangemin);
      else        
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "*** rangemin: -INF\n");                     
      connection->send(tmpbuffer,tag->str());
    
      if (variable->rangemax != UINFINITY)
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "*** rangemax: %f\n",
               variable->rangemax);
      else        
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "*** rangemax: +INF\n");
      connection->send(tmpbuffer,tag->str());
    

      if (variable->speedmin != -UINFINITY)
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "*** speedmin: %f\n",
                 variable->speedmin);
      else        
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "*** speedmin: -INF\n");                     
      connection->send(tmpbuffer,tag->str());
    
      if (variable->speedmax != UINFINITY)
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "*** speedmax: %f\n",
               variable->speedmax);
      else        
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "*** speedmax: +INF\n");
      connection->send(tmpbuffer,tag->str());

      if (variable->unit) 
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "*** unit: %s\n",
                 variable->unit->str());
      else
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "*** unit: unspecified\n");
        connection->send(tmpbuffer,tag->str());
      
    }
    
    return(status = UCOMPLETED);
  }

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_OPERATOR_VAR::copy() 
{  
  UVariableName* copy_variable;
  UString* copy_oper;
  
  if (variablename) copy_variable = variablename->copy(); else copy_variable = 0;
  if (oper) copy_oper = new UString(oper); else copy_oper = 0;

  UCommand_OPERATOR_VAR *ret = new UCommand_OPERATOR_VAR(copy_oper,
                                                         copy_variable);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_OPERATOR_VAR::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("OPERATOR_VAR %s:\n",oper->str()); 
  if (variablename) { ::urbiserver->debug("  %s  Variablename:",tabb); variablename->print(); ::urbiserver->debug("\n");};      

  ::urbiserver->debug("%sEND OPERATOR_VAR ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_OPERATOR::UCommand_OPERATOR(UString* oper) :                                        
  UCommand(CMD_GENERIC)
{	
  ADDOBJ(UCommand_OPERATOR);
  this->oper       = oper;  
}

//! UCommand subclass destructor.
UCommand_OPERATOR::~UCommand_OPERATOR()
{
  FREEOBJ(UCommand_OPERATOR);
  if (oper)       delete oper;  
}

//! UCommand subclass execution function
UCommandStatus UCommand_OPERATOR::execute(UConnection *connection)
{
  if (strcmp(oper->str(),"ping")==0) {
    snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
             "*** pong time=%f\n",connection->server->getTime());
    connection->send(tmpbuffer,tag->str());
    return( status = UCOMPLETED );
  }  

  if (strcmp(oper->str(),"commands")==0) {
    if (connection->activeCommand)
      connection->activeCommand->print(0);
    ::urbiserver->debug("*** LOCAL TREE ***\n");
    if (connection->server->parser.commandTree)
      connection->server->parser.commandTree->print(0);
    return( status = UCOMPLETED );
  }  

  if (strcmp(oper->str(),"defcheckon")==0) {
    connection->server->defcheck = true;
    return( status = UCOMPLETED );
  }  

  if (strcmp(oper->str(),"defcheckoff")==0) {
    connection->server->defcheck = false;
    return( status = UCOMPLETED );
  }  

  if (strcmp(oper->str(),"motoron")==0) {
    
    if (connection->receiving) return (status = URUNNING);

    snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
             "!!! This command is no longer valid. Please use \"motor on\" instead\n");
    connection->send(tmpbuffer,tag->str());

    connection->server->motor(true);
    return( status = UCOMPLETED );
  }  

  if (strcmp(oper->str(),"motoroff")==0) {
    
    snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
             "!!! This command is no longer valid. Please use \"motor off\" instead\n");
    connection->send(tmpbuffer,tag->str());
        
    connection->server->motor(false);
    return( status = UCOMPLETED );
  }  

  if (strcmp(oper->str(),"stopall")==0) {
        
    snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
             "*** All commands cleared\n",connection->server->getTime());
    connection->send(tmpbuffer,tag->str());
    connection->server->stopall = true;
    return( status = UCOMPLETED );
  }  

  if (strcmp(oper->str(),"undefall")==0) {
        
    snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
             "*** All variables and functions cleared\n",connection->server->getTime());
    connection->send(tmpbuffer,tag->str());

    for ( hash_map<const char*,
            UVariable*,
            hash<const char*>,
            eqStr>::iterator retr = 
            connection->server->variabletab.begin();
          retr != connection->server->variabletab.end();) {
      if ((*retr).second->uservar)
        connection->server->variabletab.erase(retr++);
      else
        retr++;
    }
    
    connection->server->functiontab.clear();
        
    return( status = UCOMPLETED );
  }  
  if (strcmp(oper->str(),"reset")==0) {
        
    snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
             "*** Reset\n",connection->server->getTime());
    connection->send(tmpbuffer,tag->str());

    persistant = false;
    morph = (UCommand*) 
      new UCommand_EXPR(
          new UExpression(
            EXPR_FUNCTION,
            new UVariableName(new UString("global"),new UString("exec"),false,(UNamedParameters *)0),
            new UNamedParameters(
                new UExpression(
                  EXPR_VALUE,
                  new UString("undefall;stopall;")
                  )
                )
            )
          );
    ::urbiserver->reloadURBIINI = true;

    return( status = UMORPH );
  }  

  if (strcmp(oper->str(),"devices")==0) {

    for ( hash_map<const char*,
            UDevice*,
            hash<const char*>,
            eqStr>::iterator retr = 
            connection->server->devicetab.begin();
          retr != connection->server->devicetab.end();
          retr++) {
      
      if (strstr( (*retr).second->device_val->unit->str(),"bin")==0)
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "*** %-13s unit=%-8s range=[%f,%f] : %s\n",
                 (*retr).second->device->str(),
                 (*retr).second->device_val->unit->str(),
                 (*retr).second->device_val->rangemin,
                 (*retr).second->device_val->rangemax,
                 (*retr).second->detail->str());       
      else        
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "*** %-13s unit=%-8s : %s\n",
                 (*retr).second->device->str(),
                 (*retr).second->device_val->unit->str(),
                 (*retr).second->detail->str());      
      
      connection->send(tmpbuffer,tag->str());
    }

    // Output in debug mode by device types

    for ( hash_map<const char*,
            UDevice*,
            hash<const char*>,
            eqStr>::iterator retr = 
            connection->server->devicetab.begin();
          retr != connection->server->devicetab.end();
          retr++) 
      if (strcmp( (*retr).second->device_val->unit->str(),"deg")==0) {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                "[%f,%f]",
                (*retr).second->device_val->rangemin,
                (*retr).second->device_val->rangemax);
        ::urbiserver->debug("%-13s range=%-26s unit=%s : %s\n", (*retr).second->device->str(), tmpbuffer,
                            (*retr).second->device_val->unit->str(),
                            (*retr).second->detail->str());
      }
               
    ::urbiserver->debug("\n");
    for ( hash_map<const char*,
            UDevice*,
            hash<const char*>,
            eqStr>::iterator retr = 
            connection->server->devicetab.begin();
          retr != connection->server->devicetab.end();
          retr++) 
      if (strcmp( (*retr).second->device_val->unit->str(),"bool")==0)  {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                "[%f,%f]",(*retr).second->device_val->rangemin,
                (*retr).second->device_val->rangemax);
        ::urbiserver->debug("%-13s range=%-26s unit=%s : %s\n", (*retr).second->device->str(),tmpbuffer ,
                            (*retr).second->device_val->unit->str(),
                            (*retr).second->detail->str());
      }
                                   
    for ( hash_map<const char*,
            UDevice*,
            hash<const char*>,
            eqStr>::iterator retr = 
            connection->server->devicetab.begin();
          retr != connection->server->devicetab.end();
          retr++) 
      if (strcmp( (*retr).second->device_val->unit->str(),"lum")==0)  {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                "[%f,%f]",(*retr).second->device_val->rangemin,
                (*retr).second->device_val->rangemax);
        ::urbiserver->debug("%-13s range=%-26s unit=%s : %s\n", (*retr).second->device->str(), tmpbuffer,
                            (*retr).second->device_val->unit->str(),
                            (*retr).second->detail->str());
      }

    ::urbiserver->debug("\n");
    for ( hash_map<const char*,
            UDevice*,
            hash<const char*>,
            eqStr>::iterator retr = 
            connection->server->devicetab.begin();
          retr != connection->server->devicetab.end();
          retr++) 
      if (strcmp( (*retr).second->device_val->unit->str(),"uPa")==0)  {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                "[%f,%f]",(*retr).second->device_val->rangemin,
                (*retr).second->device_val->rangemax);
        ::urbiserver->debug("%-13s range=%-26s unit=%s : %s\n", (*retr).second->device->str(), tmpbuffer,
                            (*retr).second->device_val->unit->str(),
                            (*retr).second->detail->str());                              
      }

    for ( hash_map<const char*,
            UDevice*,
            hash<const char*>,
            eqStr>::iterator retr = 
            connection->server->devicetab.begin();
          retr != connection->server->devicetab.end();
          retr++) 
      if (strcmp( (*retr).second->device_val->unit->str(),"cm")==0)  {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                "[%f,%f]",(*retr).second->device_val->rangemin,
                (*retr).second->device_val->rangemax);
        ::urbiserver->debug("%-13s range=%-26s unit=%s : %s\n", (*retr).second->device->str(), tmpbuffer,
                            (*retr).second->device_val->unit->str(),
                            (*retr).second->detail->str());
      }
                                   
    for ( hash_map<const char*,
            UDevice*,
            hash<const char*>,
            eqStr>::iterator retr = 
            connection->server->devicetab.begin();
          retr != connection->server->devicetab.end();
          retr++) 
      if ( (strcmp( (*retr).second->device_val->unit->str(),"m/s2")==0) ||
           (strcmp( (*retr).second->device_val->unit->str(),"m")==0) || 
           (strcmp( (*retr).second->device_val->unit->str(),"C")==0) ) {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                "[%f,%f]",(*retr).second->device_val->rangemin,
                (*retr).second->device_val->rangemax);
        ::urbiserver->debug("%-13s range=%-26s unit=%s : %s\n", (*retr).second->device->str(), tmpbuffer,
                            (*retr).second->device_val->unit->str(),
                            (*retr).second->detail->str());
      }
                               


    return( status = UCOMPLETED );
  }

  UString* fullname;

  if (strcmp(oper->str(),"vars")==0) {
    for ( hash_map<const char*,
            UVariable*,
            hash<const char*>,
            eqStr>::iterator retr = 
            connection->server->variabletab.begin();
          retr != connection->server->variabletab.end();
          retr++) {

      fullname = (*retr).second->varname;
      if (fullname) {
        switch ((*retr).second->value->dataType) {

        case DATA_NUM:
          snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                   "*** %s = %f\n",
                   fullname->str(),(*retr).second->value->val);
          break;
          
        case DATA_STRING:
          snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                   "*** %s = \"%s\"\n",
                   fullname->str(),(*retr).second->value->str->str());
          break;
          
        case DATA_BINARY:
          if ((*retr).second->value->refBinary)
            snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                     "*** %s = BIN %d\n",
                     fullname->str(),
                     (*retr).second->value->refBinary->ref()->bufferSize);
          else
            snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                     "*** %s = BIN 0 null\n",
                     fullname->str());
          break;
          
        default:
          snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                   "*** %s = UNKNOWN TYPE\n",
                   fullname->str());
        } // end switch
        
        connection->send(tmpbuffer,tag->str());
      }
    }

    return( status = UCOMPLETED );
  }  

  if (strcmp(oper->str(),"uservars")==0) {
    for ( hash_map<const char*,
            UVariable*,
            hash<const char*>,
            eqStr>::iterator retr = 
            connection->server->variabletab.begin();
          retr != connection->server->variabletab.end();
          retr++) {

      fullname = (*retr).second->varname;
      if ((*retr).second->uservar) {
      
        switch ((*retr).second->value->dataType) {
          
        case DATA_NUM:
          
          snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                   "*** %s = %f\n",
                   fullname->str(),(*retr).second->value->val);
          break;
          
        case DATA_STRING:
         
          snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                   "*** %s = \"%s\"\n",
                   fullname->str(),(*retr).second->value->str->str());
          break;
          
        case DATA_BINARY:
          if ((*retr).second->value->refBinary)
            snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                     "*** %s = BIN %d\n",
                     fullname->str(),
                     (*retr).second->value->refBinary->ref()->bufferSize);
          else
            snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                     "*** %s = BIN 0 null\n",
                     fullname->str());
          break;
          
        default:
          snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                   "*** %s = UNKNOWN TYPE\n",
                   fullname->str());
        } // end switch
        
        connection->send(tmpbuffer,tag->str());
      }
    }
    
    return( status = UCOMPLETED );
  }  

  if (strcmp(oper->str(),"debugon")==0) {
    connection->server->debugOutput = true;
    return( status = UCOMPLETED );
  }  

  if (strcmp(oper->str(),"connections")==0) {

    for (list<UConnection*>::iterator retr = ::urbiserver->connectionList.begin();
         retr != ::urbiserver->connectionList.end();
         retr++) 
      if ((*retr)->isActive())  {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "*** %s (%d.%d.%d.%d)\n",
                 (*retr)->connectionTag->str(),
                 (int) (((*retr)->clientIP>>24) % 256),
                 (int) (((*retr)->clientIP>>16) % 256),
                 (int) (((*retr)->clientIP>>8) % 256),
                 (int) ( (*retr)->clientIP     % 256)
                 );
        
        connection->send(tmpbuffer,tag->str());
      }

    
    return( status = UCOMPLETED );
  }  

  if (strcmp(oper->str(),"debugoff")==0) {
    connection->server->debugOutput = false;    
    return( status = UCOMPLETED );
  }  
 
  if (strcmp(oper->str(),"quit")==0) {
    
    connection->closeConnection();
    return( status = UCOMPLETED );
  }

  if (strcmp(oper->str(),"reboot")==0) {
    
    connection->server->reboot();
    return( status = UCOMPLETED );
  }

  if (strcmp(oper->str(),"shutdown")==0) {
    
    connection->server->shutdown();
    return( status = UCOMPLETED );
  }

  return( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_OPERATOR::copy() 
{  
  UString* copy_oper;
  
  if (oper) copy_oper = new UString(oper); else copy_oper = 0;

  UCommand_OPERATOR *ret = new UCommand_OPERATOR(copy_oper);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_OPERATOR::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("OPERATOR %s:\n",oper->str()); 
  ::urbiserver->debug("%sEND OPERATOR ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_WAIT::UCommand_WAIT(UExpression* expression) :
  UCommand(CMD_WAIT)
{	
  ADDOBJ(UCommand_WAIT);
  this->expression  = expression;

  endtime = 0;
}

//! UCommand subclass destructor.
UCommand_WAIT::~UCommand_WAIT()
{
  FREEOBJ(UCommand_WAIT);
  if (expression) delete expression;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_WAIT::execute(UConnection *connection)
{
  
  if (status == UONQUEUE) {
    
    UValue *nb = expression->eval(this,connection); 
    if (nb == 0) 
      return( status = UCOMPLETED );
    
    if (nb->dataType != DATA_NUM) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Invalid type. NUM expected.\n");
      connection->send(tmpbuffer,tag->str());
      return( status = UCOMPLETED );
    }
    if (nb->val == 0) return (status = UCOMPLETED);
 
    endtime = connection->server->lastTime() + nb->val;

    delete nb;
    status = URUNNING;
  }
  else  
    if (connection->server->lastTime() >= endtime)      
      status = UCOMPLETED;

  return(status);
}

//! UCommand subclass hard copy function
UCommand*
UCommand_WAIT::copy()
{
  UExpression*      copy_expression;
  
  if (expression) copy_expression = expression->copy(); else copy_expression = 0;

  UCommand_WAIT *ret = new UCommand_WAIT(copy_expression);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_WAIT::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("WAIT:\n"); 

  if (expression) { ::urbiserver->debug("%s  Expr:",tabb); expression->print(); ::urbiserver->debug("\n");}; 

  ::urbiserver->debug("%sEND WAIT ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_EMIT::UCommand_EMIT(UVariableName *eventname, UNamedParameters *parameters, UExpression *duration) :
  UCommand(CMD_EMIT)
{	
  ADDOBJ(UCommand_EMIT);
  this->eventname    = eventname;
  this->parameters   = parameters;
  this->duration     = duration;

  firsttime = true;
  eventid = 100+rand();
}

//! UCommand subclass destructor.
UCommand_EMIT::~UCommand_EMIT()
{
  FREEOBJ(UCommand_EMIT);
  ::urbiserver->eventtab.erase(::urbiserver->eventtab.find(eventnamestr));
  if (eventname) delete eventname;
  if (parameters) delete parameters;
  if (duration) delete duration;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_EMIT::execute(UConnection *connection)
{  
  if (connection->receiving) 
    return( status = UONQUEUE);
  
  double thetime = connection->server->lastTime();  

  if (firsttime) {
    if (duration) {
      UValue *dur = duration->eval(this, connection);
      if (!dur) { 
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "!!! invalid event duration\n",
                 eventnamestr);
        connection->send(tmpbuffer,tag->str()); 
        return ( status = UCOMPLETED);
      }
      targetTime = thetime + dur->val;
      delete dur;
    }
    else {
      targetTime = thetime;
      
      UNamedParameters *pevent = parameters;
      while (pevent) {
        UValue *e1 = pevent->expression->eval(this,connection);
        if (e1) {
          delete pevent->expression;
          pevent->expression = new UExpression(EXPR_VALUE,e1);
          delete e1;
        }

        pevent = pevent->next;
      }      
    }

    eventnamestr = eventname->buildFullname(this,connection)->str();
    /*
    if (::urbiserver->eventtab.find(eventnamestr) != ::urbiserver->eventtab.end()) {
      
      if (eventnamestr) {
        snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
                 "!!! event %s already running\n",
                 eventnamestr);
        connection->send(tmpbuffer,tag->str()); 
      }
      
      return( status = UCOMPLETED );
    } 
    */        
    // register event
   
    ::urbiserver->eventtab[eventnamestr] = this;
  }
 
  if ((thetime > targetTime) && (!firsttime)) {
    // unregister event
   
    ::urbiserver->eventtab.erase(::urbiserver->eventtab.find(eventnamestr));
    return(status = UCOMPLETED);
  }

  firsttime = false;
  return (status = UBACKGROUND);
}

//! UCommand subclass hard copy function
UCommand*
UCommand_EMIT::copy()
{
  UVariableName*      copy_eventname;
  UNamedParameters*      copy_parameters;
  
  if (eventname) copy_eventname = eventname->copy(); else copy_eventname = 0;
  if (parameters) copy_parameters = parameters->copy(); else copy_parameters = 0;

  UCommand_EMIT *ret = new UCommand_EMIT(copy_eventname, copy_parameters);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_EMIT::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("EMIT:\n"); 

  if (eventname) { ::urbiserver->debug("%s  Event:",tabb); eventname->print(); ::urbiserver->debug("\n");}; 

  ::urbiserver->debug("%sEND EMIT ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_WAIT_TEST::UCommand_WAIT_TEST(UExpression* test) :
  UCommand(CMD_WAIT_TEST)
{	
  ADDOBJ(UCommand_WAIT_TEST);
  this->test  = test;
  nbTrue  = 0;
}

//! UCommand subclass destructor.
UCommand_WAIT_TEST::~UCommand_WAIT_TEST()
{
  FREEOBJ(UCommand_WAIT_TEST);
  if (test)       delete test;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_WAIT_TEST::execute(UConnection *connection)
{
  if (!test) return (status = UCOMPLETED);

  UTestResult testres = booleval(test->eval(this, connection,true));

  if (testres == UTESTFAIL) 
    return ( status = URUNNING );

  if (testres == UTRUE) {
    if (nbTrue == 0) 
      startTrue = connection->server->lastTime();
    nbTrue++;
  }
  else 
    nbTrue = 0;  
  
  if ( ( (test->softtest_time) && 
         (nbTrue > 0) && 
         (connection->server->lastTime() - startTrue >= test->softtest_time->val)) ||

       ( (nbTrue >0) &&          
         (test->softtest_time==0)) )

    return ( status = UCOMPLETED );
  else
    return ( status = URUNNING );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_WAIT_TEST::copy() 
{  
  UExpression*      copy_test;
  
  if (test) copy_test = test->copy(); else copy_test = 0;

  UCommand_WAIT_TEST *ret = new UCommand_WAIT_TEST(copy_test);
  copybase(ret);
  ret->nbTrue  = 0; 
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_WAIT_TEST::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("WAIT_TEST:\n"); 
 
  if (test)   { ::urbiserver->debug("%s  Test:",tabb); test->print(); ::urbiserver->debug("\n");};    

  ::urbiserver->debug("%sEND WAIT_TEST ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_INCDECREMENT::UCommand_INCDECREMENT(UCommandType type, UVariableName *variablename) :
  UCommand(type)
{	
  ADDOBJ(UCommand_INCDECREMENT);
  this->variablename  = variablename;
}

//! UCommand subclass destructor.
UCommand_INCDECREMENT::~UCommand_INCDECREMENT()
{
  FREEOBJ(UCommand_INCDECREMENT);
  if (variablename)   delete variablename;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_INCDECREMENT::execute(UConnection *connection)
{      
  HMgrouptab::iterator hmg;

  UVariable* variable = variablename->getVariable(this,connection); 
  if (!variablename->getFullname()) return ( status = UCOMPLETED );  
  UString* method = variablename->getMethod();
  UString* devicename = variablename->getDevice();

  // Handling of groups and implicit multi assignments
  // (using morhping and & conjunction)

  if ((!variablename->rooted) && (devicename)) {
  
    UGroup *gp = 0;
    if ((hmg = ::urbiserver->grouptab.find(devicename->str())) !=
        ::urbiserver->grouptab.end()) 
      gp = (*hmg).second; 
        
    if ((gp) && (gp->members.size() > 0)) {
        
      UCommand *grouplist;
      UCommand *grouplist_prev = 0;
      UCommand_INCDECREMENT *clone;
      UNamedParameters *varindex;

      for (list<UGroup*>::iterator retr = gp->members.begin();
           retr != gp->members.end();
           retr++) {

        clone = (UCommand_INCDECREMENT*)this->copy();
        delete clone->variablename;
        if (variablename->index) 
          varindex = variablename->index->copy();
        else
          varindex = 0;

        clone->variablename = new UVariableName((*retr)->device->copy(),
                                        method->copy(),
                                        false,
                                        varindex);

        clone->variablename->isnormalized = variablename->isnormalized;
        clone->variablename->deriv = variablename->deriv;
        clone->variablename->varerror = variablename->varerror;

        grouplist = (UCommand*)
          new UCommand_TREE(UAND,
                            (UCommand*)clone,
                            grouplist_prev);
        grouplist_prev = grouplist;
      }

      morph = (UCommand*) 
        new UCommand_TREE(UAND,
                          this,
                          grouplist);
      
      variablename->rooted = true;
      variablename->fromGroup = true;
      persistant = true;
      return( status = UMORPH );
    }
  }

  // Main execution
  if (type == CMD_INCREMENT) {
   
    morph = (UCommand*) 
      new UCommand_ASSIGN_VALUE(
              variablename->copy(),
              new UExpression(
                         EXPR_PLUS,
                         new UExpression(EXPR_VARIABLE,variablename->copy()),
                         new UExpression(EXPR_VALUE,1.0)),0);          
        
    persistant = false;
    return( status = UMORPH );                                                
  }

  if (type == CMD_DECREMENT) {
    
    morph = (UCommand*) 
      new UCommand_ASSIGN_VALUE(
              variablename->copy(),
              new UExpression(
                         EXPR_MINUS,
                         new UExpression(EXPR_VARIABLE,variablename->copy()),
                         new UExpression(EXPR_VALUE,1.0)),0);
    
    persistant = false;
    return( status = UMORPH );                                                
  }

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_INCDECREMENT::copy() 
{  
  UVariableName*        copy_variable;
  
  if (variablename)   copy_variable = variablename->copy(); else copy_variable = 0;

  UCommand_INCDECREMENT *ret = new UCommand_INCDECREMENT(type,copy_variable);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_INCDECREMENT::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("INCDECREMENT:");
  if (type == CMD_INCREMENT) ::urbiserver->debug("INC\n");
  else
    if (type == CMD_DECREMENT) ::urbiserver->debug("DEC\n");
    else 
      ::urbiserver->debug("UNKNOWN TYPE\n");

  if (variablename) { ::urbiserver->debug("%s  Variable:",tabb); variablename->print(); ::urbiserver->debug("\n");}; 
  ::urbiserver->debug("%sEND INCDECREMENT ------\n",tabb);
}

// **************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_DEF::UCommand_DEF(UVariableName *variablename,           
                           UNamedParameters *parameters,
                           UCommand* command) :
  UCommand(CMD_DEF)
{	
  ADDOBJ(UCommand_DEF);
  this->variablename = variablename;
  this->parameters   = parameters;
  this->command      = command;
  this->device       = 0;
  this->variablelist = 0;
}

//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_DEF::UCommand_DEF(UString *device,           
                           UNamedParameters *parameters) :
  UCommand(CMD_DEF)
{	
  ADDOBJ(UCommand_DEF);
  this->variablename = 0;
  this->parameters   = parameters;
  this->command      = 0;
  this->device       = device;
  this->variablelist = 0;
}

//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_DEF::UCommand_DEF(UVariableList *variablelist) :
  UCommand(CMD_DEF)
{	
  ADDOBJ(UCommand_DEF);
  this->variablename = 0;
  this->parameters   = 0;
  this->command      = 0;
  this->device       = 0;
  this->variablelist = variablelist;
}

//! UCommand subclass destructor.
UCommand_DEF::~UCommand_DEF()
{
  FREEOBJ(UCommand_DEF);
  if (variablename) delete variablename;
  if (variablelist) delete variablelist;
  if (device) delete device;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_DEF::execute(UConnection *connection)
{
  // Def list query
  if ( (!variablename) &&
       (!command) &&
       (!parameters) && 
       (!variablelist)) {
    
    for ( hash_map<const char*,
            UFunction*,
            hash<const char*>,
            eqStr>::iterator retr = 
            connection->server->functiontab.begin();
          retr != connection->server->functiontab.end();
          retr++) {
          
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "*** %s : %d param(s)\n",
               (*retr).second->name()->str(),
               (*retr).second->nbparam());       
      
      connection->send(tmpbuffer,tag->str());
    }
    return (status = UCOMPLETED);
  }

  // Function definition
  if ((variablename) && (command)) {

    UString* funname = variablename->buildFullname(this,connection);
    if (!funname) return (status = UCOMPLETED);
    
    if (connection->server->functiontab.find(funname->str()) !=
        connection->server->functiontab.end()) {
      
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! function %s already exists\n",funname->str());
      connection->send(tmpbuffer,tag->str()); 
      
      return( status = UCOMPLETED );
    }
    
    UFunction *fun = new UFunction(new UString(funname),
                                   parameters,
                                   command);
    
    if (fun) 
      connection->server->functiontab[fun->name()->str()] = fun;
  }

  // Single Variable definition
  if ((variablename) && (!command) && (!parameters)) {  
     
    HMgrouptab::iterator hmg;
    
    UVariable* variable = variablename->getVariable(this,connection); 
    if (!variablename->getFullname()) return ( status = UCOMPLETED );   
    UString* method = variablename->getMethod();
    UString* devicename = variablename->getDevice();
        
    // Handling of groups and implicit multi assignments
    // (using morhping and & conjunction)
    
    if ((!variablename->rooted) && (devicename)) {
      
      UGroup *gp = 0;
      if ((hmg = ::urbiserver->grouptab.find(devicename->str())) !=
          ::urbiserver->grouptab.end()) 
        gp = (*hmg).second; 
      
      if ((gp) && (gp->members.size() > 0)) {
        
        UCommand *grouplist;
        UCommand *grouplist_prev = 0;
        UCommand_DEF *clone;
        UNamedParameters *varindex;
        
        for (list<UGroup*>::iterator retr = gp->members.begin();
             retr != gp->members.end();
             retr++) {
          
          clone = (UCommand_DEF*)this->copy();
          delete clone->variablename;
          if (variablename->index) 
            varindex = variablename->index->copy();
          else
            varindex = 0;
          
          clone->variablename = new UVariableName((*retr)->device->copy(),
                                                  method->copy(),
                                                  false,
                                                  varindex);
          grouplist = (UCommand*)
            new UCommand_TREE(UAND,
                              (UCommand*)clone,
                              grouplist_prev);
          grouplist_prev = grouplist;
        }
        
        morph = (UCommand*) 
          new UCommand_TREE(UAND,
                            this,
                            grouplist);
        
        variablename->rooted = true;
        variablename->fromGroup = true;
        persistant = true;
        return( status = UMORPH );
      }
    }

    // Variable definition
    
    variable = new UVariable(variablename->getFullname()->str(), new UValue());
    connection->localVariableCheck(variable);
  }

  // Device variable set definition  
  if ((device) && (!command) && (parameters)) {

    UNamedParameters * param = parameters;    
    UCommand_DEF *cdef = new UCommand_DEF (new UVariableName(device->copy(),param->name, true,0),
                             (UNamedParameters*) 0,
                             (UCommand*) 0);
    cdef->tag->update(tag->str());
    morph = cdef;
    param = param->next;

    while (param) {
      if (param->name) {
        cdef = new UCommand_DEF (new UVariableName(device->copy(),param->name, true,0),
                                 (UNamedParameters*) 0,
                                 (UCommand*) 0);
        cdef->tag->update(tag->str());
        morph = (UCommand*) new UCommand_TREE(UAND, cdef, morph);
      }
      param = param->next;
    }
    persistant = false;
    return( status = UMORPH );    
  }

  // Multi Variable definition  
  if (variablelist) {

    UVariableList *list = variablelist;
    UCommand_DEF *cdef = new UCommand_DEF (list->variablename->copy(),
                                           (UNamedParameters*) 0,
                                           (UCommand*) 0);
    cdef->tag->update(tag->str());
    morph = cdef;
    list = list->next;

    while (list) {
      if (list->variablename) {
        cdef = new UCommand_DEF (list->variablename->copy(),
                                 (UNamedParameters*) 0,
                                 (UCommand*) 0);
        cdef->tag->update(tag->str()); 
        morph = (UCommand*) new UCommand_TREE(UAND, cdef, morph);
      }
      list = list->next;
    }

    persistant = false;
    return( status = UMORPH );             
  }

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_DEF::copy() 
{  
  UVariableName*    copy_variable;
  UNamedParameters* copy_parameters;
  UCommand*         copy_command;
  UVariableList*    copy_variablelist;
  
  if (command)     copy_command = command->copy(); else copy_command =0;
  if (variablename) copy_variable = variablename->copy(); else copy_variable = 0;
  if (variablelist) copy_variablelist = variablelist->copy(); else copy_variablelist = 0;
  if (parameters) copy_parameters = parameters->copy(); else copy_parameters = 0;

  UCommand_DEF *ret = new UCommand_DEF(copy_variable,
                                       copy_parameters,
                                       copy_command);
  ret->variablelist = copy_variablelist;
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_DEF::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("DEF:\n");


  if (variablename) { ::urbiserver->debug("%s  Variablename:",tabb); variablename->print(); ::urbiserver->debug("\n");}; 
  if (variablelist) { ::urbiserver->debug("%s  Variablelist: {",tabb); variablelist->print(); ::urbiserver->debug("}\n");}; 

  if (parameters) { ::urbiserver->debug("%s  Param:{",tabb); parameters->print(); ::urbiserver->debug("}\n");};  
  if (command) { ::urbiserver->debug("%s  Command:\n",tabb); command->print(l+3);};
  ::urbiserver->debug("%sEND DEF ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_IF::UCommand_IF(UExpression *test,
                         UCommand* command1, 
                         UCommand* command2) :
  UCommand(CMD_IF)
{	
  ADDOBJ(UCommand_IF);
  this->test        = test;
  this->command1    = command1;
  this->command2    = command2;
}

//! UCommand subclass destructor.
UCommand_IF::~UCommand_IF()
{
  FREEOBJ(UCommand_IF);
  if (command1)   delete command1;
  if (command2)   delete command2;
  if (test)       delete test;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_IF::execute(UConnection *connection)
{
  if (!test) return (status = UCOMPLETED);

  UTestResult testres = booleval(test->eval(this, connection));

  if (testres == UTESTFAIL) 
    return ( status = UCOMPLETED );

  if (testres == UTRUE) {
    
    morph = command1;
    command1 = 0; // avoid delete of command when this is deleted
    persistant = false;
    return( status = UMORPH );
  }
  else 
    if (command2) {
      
      morph = command2;
      command2 = 0; // avoid delete of command when this is deleted
      persistant = false;
      return( status = UMORPH );
    }
    else
      return ( status = UCOMPLETED );        
}

//! UCommand subclass hard copy function
UCommand*
UCommand_IF::copy() 
{  
  UCommand*   copy_command1;
  UCommand*   copy_command2;
  UExpression*      copy_test;
  
  if (test)     copy_test = test->copy(); else copy_test = 0;
  if (command1) copy_command1 = command1->copy(); else copy_command1 =0;
  if (command2) copy_command2 = command2->copy(); else copy_command2 =0;

  UCommand_IF *ret = new UCommand_IF(copy_test,
                                     copy_command1,
                                     copy_command2);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_IF::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("IF:\n");

  if (test)   { ::urbiserver->debug("%s  Test:",tabb); test->print(); ::urbiserver->debug("\n");}; 
  if (command1) { ::urbiserver->debug("%s  Command1:\n",tabb); command1->print(l+3);};
  if (command2) { ::urbiserver->debug("%s  Command2:\n",tabb); command2->print(l+3);};
  ::urbiserver->debug("%sEND IF ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_EVERY::UCommand_EVERY( UExpression *duration,
                                UCommand* command) :
  UCommand(CMD_EVERY)
{	
  ADDOBJ(UCommand_EVERY);
  this->duration    = duration;
  this->command     = command; 

  firsttime = true;
  starttime = 0;
}

//! UCommand subclass destructor.
UCommand_EVERY::~UCommand_EVERY()
{
  FREEOBJ(UCommand_EVERY);
  if (command)    delete command;
  if (duration)   delete duration;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_EVERY::execute(UConnection *connection)
{
  double thetime = connection->server->lastTime();  

  if (command == 0) return ( status = UCOMPLETED );  

  UValue *interval = duration->eval(this, connection);
  if (!interval) return ( status = UCOMPLETED);

  if ((starttime + interval->val <= thetime) ||
      (firsttime)) {

    persistant = true;
    morph = (UCommand*) 
      new UCommand_TREE( UAND,                           
                         command->copy(),
                         this
                         );
    starttime = thetime;
    firsttime = false;
    delete interval;
    return( status = UMORPH );
  }
  
  delete interval;  
  return ( status = UBACKGROUND );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_EVERY::copy() 
{  
  UCommand*     copy_command;
  UExpression*  copy_duration;
  
  if (duration) copy_duration = duration->copy(); else copy_duration = 0;
  if (command)  copy_command = command->copy(); else copy_command =0;

  UCommand_EVERY *ret = new UCommand_EVERY(copy_duration,
                                           copy_command);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_EVERY::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("EVERY:");

  if (duration)     { ::urbiserver->debug("%s  Duration:",tabb); duration->print(); ::urbiserver->debug("\n");}; 
  if (command) { ::urbiserver->debug("%s  Command:\n",tabb); command->print(l+3);};
  ::urbiserver->debug("%sEND EVERY ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_TIMEOUT::UCommand_TIMEOUT( UExpression *duration,
                                    UCommand* command) :
  UCommand(CMD_TIMEOUT)
{	
  ADDOBJ(UCommand_TIMEOUT);
  this->duration    = duration;
  this->command     = command; 

  snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,"__TAG_timeout_%d",(int)this);
  this->tagRef      = new UString(tmpbuffer);
}

//! UCommand subclass destructor.
UCommand_TIMEOUT::~UCommand_TIMEOUT()
{
  FREEOBJ(UCommand_TIMEOUT);
  if (command)    delete command;
  if (duration)   delete duration;
  if (tagRef)     delete tagRef;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_TIMEOUT::execute(UConnection *connection)
{
  if (command == 0) return ( status = UCOMPLETED ); 

  persistant = false;
  morph = (UCommand*) 
    new UCommand_TREE( UAND,                              
                       new UCommand_TREE( UPIPE,  
                                          new UCommand_WAIT(duration->copy()),
                                          new UCommand_OPERATOR_ID(new UString("stop"),
                                                                   tagRef->copy())),
                       command->copy()
                       );    
  
  morph->tag->update(tagRef);
  return( status = UMORPH );    
}

//! UCommand subclass hard copy function
UCommand*
UCommand_TIMEOUT::copy() 
{  
  UCommand*     copy_command;
  UExpression*  copy_duration;
  
  if (duration) copy_duration = duration->copy(); else copy_duration = 0;
  if (command)  copy_command = command->copy(); else copy_command =0;

  UCommand_TIMEOUT *ret = new UCommand_TIMEOUT(copy_duration,
                                               copy_command);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_TIMEOUT::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("TIMEOUT:");

  if (duration)     { ::urbiserver->debug("%s  Duration:",tabb); duration->print(); ::urbiserver->debug("\n");}; 
  if (command) { ::urbiserver->debug("%s  Command:\n",tabb); command->print(l+3);};
  ::urbiserver->debug("%sEND TIMEOUT ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_STOPIF::UCommand_STOPIF( UExpression *condition,
                                  UCommand* command) :
  UCommand(CMD_STOPIF)
{	
  ADDOBJ(UCommand_STOPIF);
  this->condition   = condition;
  this->command     = command; 

  snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,"__TAG_stopif_%d",(int)100+rand());
  this->tagRef      = new UString(tmpbuffer);
}

//! UCommand subclass destructor.
UCommand_STOPIF::~UCommand_STOPIF()
{
  FREEOBJ(UCommand_STOPIF);
  if (command)    delete command;
  if (condition)  delete condition;
  if (tagRef)     delete tagRef;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_STOPIF::execute(UConnection *connection)
{
  if ((!command) || (!condition)) return ( status = UCOMPLETED ); 
  
  UTestResult testres = booleval(condition->eval(this, connection));
  
  if (testres == UTRUE) 
    return ( status = UCOMPLETED );
  
  persistant = false;
  morph = (UCommand*) 
    new UCommand_TREE( UAND, 
                       new UCommand_AT(CMD_AT,
                                       condition->copy(),
                                       new UCommand_OPERATOR_ID(new UString("stop"),
                                                                tagRef->copy()),
                                       (UCommand*)0),                                       
                       command->copy()
                       );    
  morph->tag->update(tagRef);
  return( status = UMORPH );    


  if (command == 0) return ( status = UCOMPLETED ); 

  persistant = false;
  morph = (UCommand*) 
    new UCommand_TREE( UAND,                              
                       new UCommand_TREE( UPIPE,  
                                          new UCommand_WAIT_TEST(condition->copy()),
                                          new UCommand_OPERATOR_ID(new UString("stop"),
                                                                   tagRef->copy())),
                       command->copy()
                       );    
  
  morph->tag->update(tagRef);
  return( status = UMORPH );    
}

//! UCommand subclass hard copy function
UCommand*
UCommand_STOPIF::copy() 
{  
  UCommand*     copy_command;
  UExpression*  copy_condition;
  
  if (condition) copy_condition = condition->copy(); else copy_condition = 0;
  if (command)  copy_command = command->copy(); else copy_command =0;

  UCommand_STOPIF *ret = new UCommand_STOPIF(copy_condition,
                                             copy_command);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_STOPIF::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("STOPIF:");

  if (condition)     { ::urbiserver->debug("%s  Condition:",tabb); condition->print(); ::urbiserver->debug("\n");}; 
  if (command) { ::urbiserver->debug("%s  Command:\n",tabb); command->print(l+3);};
  ::urbiserver->debug("%sEND STOPIF ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_FREEZEIF::UCommand_FREEZEIF( UExpression *condition,
                                  UCommand* command) :
  UCommand(CMD_FREEZEIF)
{	
  ADDOBJ(UCommand_FREEZEIF);
  this->condition   = condition;
  this->command     = command; 

  snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,"__TAG_stopif_%d",(int)100+rand());
  this->tagRef      = new UString(tmpbuffer);
}

//! UCommand subclass destructor.
UCommand_FREEZEIF::~UCommand_FREEZEIF()
{
  FREEOBJ(UCommand_FREEZEIF);
  if (command)    delete command;
  if (condition)  delete condition;
  if (tagRef)     delete tagRef;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_FREEZEIF::execute(UConnection *connection)
{
  if (command == 0) return ( status = UCOMPLETED ); 

  persistant = false;
  UCommand* cmd = new UCommand_TREE( UPIPE,
                                     command->copy(),
                                     new UCommand_NOOP()
                                     );
  cmd->tag->update(tagRef);
  morph = (UCommand*) 
    new UCommand_TREE( UAND, 
                       new UCommand_AT(CMD_AT,
                                       condition->copy(),
                                       new UCommand_OPERATOR_ID(new UString("freeze"),
                                                                tagRef->copy()),
                                       new UCommand_OPERATOR_ID(new UString("unfreeze"),
                                                                tagRef->copy())),                                       
                       cmd
                       );    
    
  return( status = UMORPH );    
}

//! UCommand subclass hard copy function
UCommand*
UCommand_FREEZEIF::copy() 
{  
  UCommand*     copy_command;
  UExpression*  copy_condition;
  
  if (condition) copy_condition = condition->copy(); else copy_condition = 0;
  if (command)  copy_command = command->copy(); else copy_command =0;

  UCommand_FREEZEIF *ret = new UCommand_FREEZEIF(copy_condition,
                                             copy_command);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_FREEZEIF::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("FREEZEIF:");

  if (condition)     { ::urbiserver->debug("%s  Condition:",tabb); condition->print(); ::urbiserver->debug("\n");}; 
  if (command) { ::urbiserver->debug("%s  Command:\n",tabb); command->print(l+3);};
  ::urbiserver->debug("%sEND FREEZEIF ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_AT::UCommand_AT( UCommandType type,
                          UExpression *test,
                          UCommand* command1, 
                          UCommand* command2) :
  UCommand(type)
{	
  ADDOBJ(UCommand_AT);
  this->test        = test;
  this->command1    = command1;
  this->command2    = command2;

  nbTrue  = 0;
  mode    = true;
}

//! UCommand subclass destructor.
UCommand_AT::~UCommand_AT()
{
  FREEOBJ(UCommand_AT);
  if (command1)   delete command1;
  if (command2)   delete command2;
  if (test)       delete test;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_AT::execute(UConnection *connection)
{
  if (!test) return (status = UCOMPLETED);

  UValue *testeval = test->eval(this, connection, true);
  UTestResult testres = booleval(testeval,false);
  static int eventtestid = 0;

  if (testres == UTESTFAIL) 
    testres = UFALSE; 

  if ((testeval) && 
      (testeval->eventid) && 
      (testeval->eventid != eventtestid) && 
      (mode == false))
    mode = true;          

  if (testres == mode) {
    if ((testeval) &&
        (testeval->eventid) && 
        (testeval->eventid != eventtestid)) {
      nbTrue =0;
      eventtestid = testeval->eventid;
    }
    if (nbTrue == 0)
      startTrue = connection->server->lastTime();
    nbTrue++;
  }
  else 
    nbTrue = 0;  
  
  if (testeval) delete testeval;

  if ( ( (nbTrue>0) && 
         (test->softtest_time) && 
         (connection->server->lastTime() - startTrue >= test->softtest_time->val)) ||
       ( (nbTrue >0) &&          
         (test->softtest_time==0)) ) {

    nbTrue = 0;
    
    UNodeType     nodeType;

    if (type == CMD_AT)      nodeType = USEMICOLON;
    if (type == CMD_AT_AND)  nodeType = UAND;

    if (mode == true) {

      mode = false;
      morph = (UCommand*) 
        new UCommand_TREE(
              nodeType,
              new UCommand_TREE(
                    UAND,                                                          
                    command1->copy(),
                    new UCommand_NOOP()
                  ), 
              this                
            );
             
      morph->background = true;
      persistant = true;
      return( status = UMORPH );
    }
    else {
      mode = true;

      if (command2) {

        morph = (UCommand*) 
          new UCommand_TREE(
                nodeType,
                new UCommand_TREE(
                      UAND,                                                          
                      command2->copy(),
                      new UCommand_NOOP()
                    ), 
                this                
              );
             
        morph->background = true;
        persistant = true;
        return( status = UMORPH );        
      }

      return ( status = UBACKGROUND ); 
      //return (status = URUNNING);
    }
  }
  else 
    return ( status = UBACKGROUND ); 
  //return (status = URUNNING);
}

//! UCommand subclass hard copy function
UCommand*
UCommand_AT::copy() 
{  
  UCommand*   copy_command1;
  UCommand*   copy_command2;
  UExpression*      copy_test;
  
  if (test)     copy_test = test->copy(); else copy_test = 0;
  if (command1) copy_command1 = command1->copy(); else copy_command1 =0;
  if (command2) copy_command2 = command2->copy(); else copy_command2 =0;

  UCommand_AT *ret = new UCommand_AT(type,
                                     copy_test,
                                     copy_command1,
                                     copy_command2);
  copybase(ret);

  ret->nbTrue  = 0; 
  ret->mode    = true; 

  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_AT::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] toDelete=%d ",tabb,tag->str(),toDelete);}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("AT:");
  if (type == CMD_AT) ::urbiserver->debug("\n");
  else
    if (type == CMD_AT_AND) ::urbiserver->debug("(AND)\n");    
    else
      ::urbiserver->debug("UNKNOWN TYPE!\n");

  if (test)   { ::urbiserver->debug("%s  Test:",tabb); test->print(); ::urbiserver->debug("\n");}; 
  if (command1) { ::urbiserver->debug("%s  Command1:\n",tabb); command1->print(l+3);};
  if (command2) { ::urbiserver->debug("%s  Command2:\n",tabb); command2->print(l+3);};
  ::urbiserver->debug("%sEND AT ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_WHILE::UCommand_WHILE(UCommandType type,
                               UExpression *test,
                               UCommand* command) :
  UCommand(type)
{	
  ADDOBJ(UCommand_WHILE);
  this->test        = test;
  this->command     = command;  
}

//! UCommand subclass destructor.
UCommand_WHILE::~UCommand_WHILE()
{
  FREEOBJ(UCommand_WHILE);
  if (command)    delete command;
  if (test)       delete test;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_WHILE::execute(UConnection *connection)
{
  if (command == 0) return ( status = UCOMPLETED );
    
  persistant = false;

  if (!test) return (status = UCOMPLETED);
  UTestResult testres = booleval(test->eval(this, connection));

  if (testres == UTESTFAIL) 
    return ( status = UCOMPLETED );
  
  if (testres == UTRUE) {
    
    UNodeType     nodeType;

    if (type == CMD_WHILE)      nodeType = USEMICOLON;
    if (type == CMD_WHILE_PIPE) nodeType = UPIPE;

    if (nodeType == UPIPE)
      morph = (UCommand*) 
        new UCommand_TREE(                          
              UPIPE,  
              command->copy(),
              this
              );
    else
      morph = (UCommand*) 
        new UCommand_TREE(
              nodeType,                                           
              new UCommand_TREE(
                    UAND,
                    command->copy(),
                    new UCommand_NOOP()
                  ),
              this
            );
          
    persistant = true;
    return( status = UMORPH );
  }
  else
    return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_WHILE::copy() 
{  
  UCommand*   copy_command;
  UExpression*      copy_test;
  
  if (test)    copy_test = test->copy(); else copy_test = 0;
  if (command) copy_command = command->copy(); else copy_command =0;

  UCommand_WHILE *ret = new UCommand_WHILE(type,
                                           copy_test,
                                           copy_command);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_WHILE::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("WHILE:");
  if (type == CMD_WHILE) ::urbiserver->debug("\n");
  else
    if (type == CMD_WHILE_AND) ::urbiserver->debug("(AND)\n");    
    else
      if (type == CMD_WHILE_PIPE) ::urbiserver->debug("(PIPE)\n");    
      else
        ::urbiserver->debug("UNKNOWN TYPE!\n");

  if (test)     { ::urbiserver->debug("%s  Test:",tabb); test->print(); ::urbiserver->debug("\n");}; 
  if (command) { ::urbiserver->debug("%s  Command:\n",tabb); command->print(l+3);};
  ::urbiserver->debug("%sEND WHILE ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_WHENEVER::UCommand_WHENEVER(UExpression *test,
                                     UCommand* command1, 
                                     UCommand* command2) :
  UCommand(CMD_WHENEVER)
{	
  ADDOBJ(UCommand_WHENEVER);
  this->test        = test;
  this->command1    = command1;
  this->command2    = command2;

  nbTrue = 0;
  nbFalse = 0;
}

//! UCommand subclass destructor.
UCommand_WHENEVER::~UCommand_WHENEVER()
{
  FREEOBJ(UCommand_WHENEVER);
  if (command1)   delete command1;
  if (command2)   delete command2;
  if (test)       delete test;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_WHENEVER::execute(UConnection *connection)
{  
  if (command1 == 0) return ( status = UCOMPLETED );
   
  if (!test) return (status = UCOMPLETED);
  UTestResult testres = booleval(test->eval(this, connection,true));

  if (testres == UTESTFAIL) 
    testres = UFALSE;  
  
  if (testres == true) {
    nbFalse = 0;
    if (nbTrue == 0) 
      startTrue = connection->server->lastTime();
    nbTrue++;
  }
  else {
    nbTrue = 0;  
    if (nbFalse == 0) 
      startFalse = connection->server->lastTime();
    nbFalse++;
  }  

  if ( ( (nbTrue>0) && 
         (test->softtest_time) && 
         (connection->server->lastTime() - startTrue >= test->softtest_time->val)) ||
       ( (nbTrue >0) &&          
         (test->softtest_time==0)) ) {   

    morph = (UCommand*) 
        new UCommand_TREE(
              USEMICOLON,
              new UCommand_TREE(
                    UAND,                                                          
                    command1->copy(),
                    new UCommand_NOOP()
                  ),
              this
            );
           
    morph->background = true;
    persistant = true;
    return( status = UMORPH );
  }
  else
    if (command2) {
      
      if ( ( (nbFalse>0) && 
             (test->softtest_time) && 
             (connection->server->lastTime() - startFalse >= test->softtest_time->val)) ||
           ( (nbFalse >0) &&             
             (test->softtest_time==0)) ) {
               
        morph = (UCommand*) 
          new UCommand_TREE(
              USEMICOLON,
              new UCommand_TREE(
                    UAND,                                                          
                    command2->copy(),
                    new UCommand_NOOP()
                  ),
              this
            );
           
        morph->background = true;
        persistant = true;
        return( status = UMORPH );
      }
      else
        return ( status = UBACKGROUND );
      //return ( status = URUNNING );
    }
    else
      return ( status = UBACKGROUND );
      //return ( status = URUNNING );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_WHENEVER::copy() 
{  
  UCommand*   copy_command1;
  UCommand*   copy_command2;
  UExpression*      copy_test;
  
  if (test)     copy_test = test->copy(); else copy_test = 0;
  if (command1) copy_command1 = command1->copy(); else copy_command1 =0;
  if (command2) copy_command2 = command2->copy(); else copy_command2 =0;

  UCommand_WHENEVER *ret = new UCommand_WHENEVER(copy_test,
                                                 copy_command1,
                                                 copy_command2);
  copybase(ret);
  ret->nbTrue = 0;
  ret->nbFalse = 0;
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_WHENEVER::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("WHENEVER:\n");

  if (test)   { ::urbiserver->debug("%s  Test:",tabb); test->print(); ::urbiserver->debug("\n");}; 
  if (command1) { ::urbiserver->debug("%s  Command1:\n",tabb); command1->print(l+3);};
  if (command2) { ::urbiserver->debug("%s  Command2:\n",tabb); command2->print(l+3);};
  ::urbiserver->debug("%sEND WHENEVER ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_LOOP::UCommand_LOOP(UCommand* command) :
  UCommand(CMD_LOOP)
{	
  ADDOBJ(UCommand_LOOP);  
  this->command     = command;  
}

//! UCommand subclass destructor.
UCommand_LOOP::~UCommand_LOOP()
{
  FREEOBJ(UCommand_LOOP);
  if (command)    delete command;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_LOOP::execute(UConnection *connection)
{
  if (command == 0) return ( status = UCOMPLETED );

  morph = (UCommand*) 
    new UCommand_TREE(USEMICOLON,
                      new UCommand_TREE(UAND,                                                          
                                        command->copy(),
                                        new UCommand_NOOP()),
                      this);
  persistant = true;
  return( status = UMORPH );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_LOOP::copy() 
{  
  UCommand*   copy_command;
  
  if (command) copy_command = command->copy(); else copy_command =0;

  UCommand_LOOP *ret = new UCommand_LOOP(copy_command);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_LOOP::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] toDelete=%d",tabb,tag->str(),toDelete);}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("LOOP:\n");

  if (command) { ::urbiserver->debug("%s  Command:\n",tabb); command->print(l+3);};
  ::urbiserver->debug("%sEND LOOP ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_LOOPN::UCommand_LOOPN(UCommandType type,
                               UExpression* expression,
                               UCommand* command) :
  UCommand(type)
{	
  ADDOBJ(UCommand_LOOPN);  
  this->expression  = expression;
  this->command     = command;  
}

//! UCommand subclass destructor.
UCommand_LOOPN::~UCommand_LOOPN()
{
  FREEOBJ(UCommand_LOOPN);
  if (command)  delete command;
  if (expression) delete expression;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_LOOPN::execute(UConnection *connection)
{
  if (command == 0) return ( status = UCOMPLETED );

  //if (status == UONQUEUE)
  //  command->status = UONQUEUE;
    
  if (expression->type != EXPR_VALUE) {
    
    UValue *nb = expression->eval(this,connection); 

    if (nb == 0) 
      return( status = UCOMPLETED );

    if (nb->dataType != DATA_NUM) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! number of loops is non numeric\n");
      connection->send(tmpbuffer,tag->str());
      delete nb;
      return( status = UCOMPLETED );  
    }

    expression->type = EXPR_VALUE;
    expression->dataType = DATA_NUM;
    expression->val = nb->val;
    delete nb;
  }  
  
  if (expression->val < 1) 
    return( status = UCOMPLETED);
  
  expression->val = expression->val - 1;
  UNodeType     nodeType;

  if (type == CMD_LOOPN)      nodeType = USEMICOLON;
  if (type == CMD_LOOPN_PIPE) nodeType = UPIPE;
  if (type == CMD_LOOPN_AND)  nodeType = UAND;

  if ((nodeType == UPIPE) ||
      (nodeType == UAND))
    morph = (UCommand*) 
      new UCommand_TREE(nodeType,                                                         
                        command->copy(),
                        this);
  else
    morph = (UCommand*) 
      new UCommand_TREE(nodeType,
                      new UCommand_TREE(UAND,                                                          
                                        command->copy(),
                                        new UCommand_NOOP()),
                      this);
  persistant = true;
  return( status = UMORPH );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_LOOPN::copy() 
{  
  UExpression*      copy_expression;
  UCommand*         copy_command;
  
  if (expression) copy_expression = expression->copy(); else copy_expression = 0;
  if (command)    copy_command = command->copy(); else copy_command =0;

  UCommand_LOOPN *ret = new UCommand_LOOPN(type,
                                           copy_expression,
                                           copy_command);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_LOOPN::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("LOOPN:");
  if (type == CMD_LOOPN) ::urbiserver->debug("\n");
  else
    if (type == CMD_LOOPN_AND) ::urbiserver->debug("(AND)\n");    
    else
      if (type == CMD_LOOPN_PIPE) ::urbiserver->debug("(PIPE)\n");    
      else
        ::urbiserver->debug("UNKNOWN TYPE!\n");

  if (expression) { ::urbiserver->debug("%s  Expr:",tabb); expression->print(); ::urbiserver->debug("\n");}; 
  if (command) { ::urbiserver->debug("%s  Command (%d:%d):\n",tabb,
                            (int)command,
                            (int)command->status); command->print(l+3);};

  ::urbiserver->debug("%sEND LOOPN ------\n",tabb);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_FOR::UCommand_FOR( UCommandType type,
                            UCommand* instr1,
                            UExpression* test,
                            UCommand* instr2,
                            UCommand* command) :
  UCommand(type)
{	
  ADDOBJ(UCommand_FOR); 
  this->instr1      = instr1;
  this->test        = test;
  this->instr2      = instr2;
  this->command     = command;
  this->first       = true;
}

//! UCommand subclass destructor.
UCommand_FOR::~UCommand_FOR()
{
  FREEOBJ(UCommand_FOR);
  if (command)    delete command;
  if (instr1)     delete instr1;
  if (instr2)     delete instr2;
  if (test)       delete test;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_FOR::execute(UConnection *connection)
{   
  UCommand *tmp_instr2;

  if ((first) && (tag)) {
    first = false;
    if ((instr1) && (instr1->tag)) instr1->tag->update(tag->str());
    if ((instr2) && (instr2->tag)) instr2->tag->update(tag->str());
  }

  if (command == 0) return ( status = UCOMPLETED );

  if (instr1) {

    UCommand *first_instruction = instr1;

    instr1 = 0;
    morph = (UCommand*) 
      new UCommand_TREE(USEMICOLON,
                        first_instruction, 
                        this);
    persistant = true;
    return( status = UMORPH );
  }
    
  persistant = false;

  if (!test) return (status = UCOMPLETED);
  UTestResult testres = booleval(test->eval(this, connection));

  if (testres == UTESTFAIL) 
    return ( status = UCOMPLETED );
  
  if (testres == UTRUE) {
    
    UNodeType     nodeType;

    if (type == CMD_FOR)      nodeType = USEMICOLON;
    if (type == CMD_FOR_PIPE) nodeType = UPIPE;
    if (type == CMD_FOR_AND)  nodeType = UAND;
    tmp_instr2 = 0;

    if ((nodeType == UPIPE) ||
        (nodeType == UAND)) {

      if (instr2) 
        morph = (UCommand*) 
          new UCommand_TREE(
              nodeType,
              command->copy(),
              new UCommand_TREE(
                    UPIPE,                                                                            
                    tmp_instr2 = instr2->copy(),
	            this
                    )              
            );      
      else
        morph = (UCommand*) 
          new UCommand_TREE(
              nodeType,                                                         
              command->copy(),
              this
            );
    }
    else {

      if (instr2)
        morph = (UCommand*) 
          new UCommand_TREE(
              nodeType,             
              new UCommand_TREE(
                    UAND,                                            
                    new UCommand_TREE(
                          UPIPE,                                                          
                          command->copy(),
                          tmp_instr2 = instr2->copy()
                        ),
                    new UCommand_NOOP()                    
                  ),
              this
            );      
      else
        morph = (UCommand*) 
          new UCommand_TREE(
              nodeType,                                           
              new UCommand_TREE(
                    UAND,                                                          
                    command->copy(),
                    new UCommand_NOOP()
                  ),
              this
            );
    }
    if (tmp_instr2) tmp_instr2->morphed = true;
    persistant = true;
    return( status = UMORPH );
  }
  else
    return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_FOR::copy() 
{  
  UCommand*   copy_instr1;
  UCommand*   copy_instr2;
  UCommand*   copy_command;
  UExpression*      copy_test;
  
  if (test)    copy_test = test->copy(); else copy_test = 0;
  if (instr1)  copy_instr1 = instr1->copy(); else copy_instr1 =0;
  if (instr2)  copy_instr2 = instr2->copy(); else copy_instr2 =0;
  if (command) copy_command = command->copy(); else copy_command =0;

  UCommand_FOR *ret = new UCommand_FOR(type,
                                       copy_instr1,
                                       copy_test,
                                       copy_instr2,
                                       copy_command);
  ret->first = first;
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_FOR::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("FOR:");
  if (type == CMD_FOR) ::urbiserver->debug("\n");
  else
    if (type == CMD_FOR_AND) ::urbiserver->debug("(AND)\n");    
    else
      if (type == CMD_FOR_PIPE) ::urbiserver->debug("(PIPE)\n");    
      else
        ::urbiserver->debug("UNKNOWN TYPE!\n");

  if (test)     { ::urbiserver->debug("%s  Test:",tabb); test->print(); ::urbiserver->debug("\n");}; 
  if (instr1)   { ::urbiserver->debug("%s  Instr1:\n",tabb); instr1->print(l+3);};
  if (instr2)   { ::urbiserver->debug("%s  Instr2:\n",tabb); instr2->print(l+3);};
  if (command)  { ::urbiserver->debug("%s  Command:\n",tabb); command->print(l+3);};

  ::urbiserver->debug("%sEND FOR ------\n",tabb);
}


// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_FOREACH::UCommand_FOREACH( UCommandType type,
                                    UVariableName* variablename,
                                    UExpression* expression,
                                    UCommand* command) :
  UCommand(type)
{	
  ADDOBJ(UCommand_FOREACH); 
  this->variablename = variablename;
  this->expression   = expression;
  this->command      = command;

  position = 0;
  firsttime = true;
}

//! UCommand subclass destructor.
UCommand_FOREACH::~UCommand_FOREACH()
{
  FREEOBJ(UCommand_FOREACH);
  if (command)      delete command;
  if (variablename) delete variablename;
  if (expression)   delete expression;
}

//! UCommand subclass execution function
UCommandStatus 
UCommand_FOREACH::execute(UConnection *connection)
{ 
  if (firsttime) {
    
    firsttime = false;
    position = expression->eval(this, connection);
    if (position == 0) return ( status = UCOMPLETED ); 
    if (position->dataType == DATA_LIST)      
      position = position->list;      
  }

  if (position == 0) 
    return (status = UCOMPLETED);

  UNodeType     nodeType;
  
  if (type == CMD_FOREACH)      nodeType = USEMICOLON;
  if (type == CMD_FOREACH_PIPE) nodeType = UPIPE;
  if (type == CMD_FOREACH_AND)  nodeType = UAND;

  UExpression* currentvalue = new UExpression(EXPR_VALUE,0.0);
  if (!currentvalue)  return( status = UCOMPLETED );  
  currentvalue->dataType = position->dataType;
  if (position->dataType == DATA_NUM)    currentvalue->val = position->val;
  if (position->dataType == DATA_STRING) currentvalue->str = new UString(position->str);
  if (position->dataType == DATA_BINARY) {
    // add support here
    
  }
  if ((position->dataType != DATA_NUM) &&
      (position->dataType != DATA_STRING)) {
    
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
               "!!! This type is not supported yet\n");
      connection->send(tmpbuffer,tag->str());
      delete currentvalue;
      return( status = UCOMPLETED );  
  }
                                             
  morph =  (UCommand*) 
    new UCommand_TREE(
         nodeType,             
         new UCommand_TREE(
              UPIPE,
              new UCommand_ASSIGN_VALUE(
                  variablename->copy(),
                  currentvalue,
                  (UNamedParameters*)0),
              command->copy()),
         this
         );
  ((UCommand_TREE*)((UCommand_TREE*)morph)->command1)->command1->tag->update(tag->str());

  position = position->list;
  persistant = true;
  return (status = UMORPH);   
}

//! UCommand subclass hard copy function
UCommand*
UCommand_FOREACH::copy() 
{  
  UCommand*       copy_command;
  UExpression*    copy_expression;
  UVariableName*  copy_variablename;
  
  if (variablename) copy_variablename = variablename->copy(); else copy_variablename = 0;
  if (expression) copy_expression = expression->copy(); else copy_expression = 0;
  if (command) copy_command = command->copy(); else copy_command =0;

  UCommand_FOREACH *ret = new UCommand_FOREACH(type,
                                               copy_variablename,
                                               copy_expression,                                     
                                               copy_command);  
  copybase(ret);
  position = 0;  
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_FOREACH::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("FOREACH:");
  if (type == CMD_FOREACH) ::urbiserver->debug("\n");
  else
    if (type == CMD_FOREACH_AND) ::urbiserver->debug("(AND)\n");    
    else
      if (type == CMD_FOREACH_PIPE) ::urbiserver->debug("(PIPE)\n");    
      else
        ::urbiserver->debug("UNKNOWN TYPE!\n");

  if (variablename) { ::urbiserver->debug("%s  VariableName:",tabb); variablename->print(); ::urbiserver->debug("\n");};    
  if (expression) { ::urbiserver->debug("%s  List:",tabb); expression->print(); ::urbiserver->debug("\n");}; 
  if (command)  { ::urbiserver->debug("%s  Command:\n",tabb); command->print(l+3);};

  ::urbiserver->debug("%sEND FOREACH ------\n",tabb);
}


// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
    If zerotime is true, the noop command will be terminated as soon as it is
    executed. It is a truely empty command, used to structure command trees
    like in the { commands... }  case.
*/
UCommand_NOOP::UCommand_NOOP(bool zerotime) :
  UCommand(CMD_NOOP)
{	
  ADDOBJ(UCommand_NOOP);
  if (zerotime) status = URUNNING;
}

//! UCommand subclass destructor.
UCommand_NOOP::~UCommand_NOOP()
{
  FREEOBJ(UCommand_NOOP);
}

//! UCommand subclass execution function
UCommandStatus UCommand_NOOP::execute(UConnection *connection)
{
  if (status == UONQUEUE) {
    if (!connection->receiving)
      status = URUNNING;
    else 
      status = UONQUEUE;
  }
  else
    status = UCOMPLETED;
  return(status);
}

//! UCommand subclass hard copy function
UCommand*
UCommand_NOOP::copy() 
{  
  UCommand_NOOP *ret = new UCommand_NOOP(status == URUNNING);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_NOOP::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("NOOP, level =%d\n",(int)status);
}

// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
    This class is used to delay the processing of a loaded file.
*/
UCommand_LOAD::UCommand_LOAD(UCommand_TREE* mainnode) :
  UCommand(CMD_LOAD)
{	
  ADDOBJ(UCommand_LOAD);

  loadQueue = new UCommandQueue (4096,1048576,false);  
  this->mainnode = mainnode;
}

//! UCommand subclass destructor.
UCommand_LOAD::~UCommand_LOAD()
{
  FREEOBJ(UCommand_LOAD);

  if (loadQueue) delete loadQueue;
}

//! UCommand subclass execution function
UCommandStatus UCommand_LOAD::execute(UConnection *connection)
{   
  if (connection->receiving) return (status = URUNNING);

  int length;
  ubyte* str_command = loadQueue->popCommand(length);    
  
  if ((str_command == 0) && (length==-1)) 
    return (status = UCOMPLETED);      
  
  if (length !=0) {
    
    ::urbiserver->parser.commandTree = 0;
    errorMessage[0] = 0;

    ::urbiserver->systemcommands = false;
    int result = ::urbiserver->parser.process(str_command, length, connection);
    ::urbiserver->systemcommands = true;
    
    if (errorMessage[0] != 0) { // a parsing error occured 
      
      if (::urbiserver->parser.commandTree) {
        delete ::urbiserver->parser.commandTree;
        ::urbiserver->parser.commandTree = 0;
      }
      
      connection->send(errorMessage,"error");                   
    }
    else {

      ::urbiserver->parser.commandTree->tag->update("__system__");
      ::urbiserver->parser.commandTree->command2 = this;
      up = ::urbiserver->parser.commandTree;
      position = &(::urbiserver->parser.commandTree->command2);
      morph = ::urbiserver->parser.commandTree;      
      ::urbiserver->parser.commandTree = 0;

      morph->tag->update("__system__");
      persistant = true;
      return( status = UMORPH );
    }
  }
  else {
    //mainnode->node = UAND;
    return (status = UCOMPLETED);
  }  
}

//! UCommand subclass hard copy function
UCommand*
UCommand_LOAD::copy() 
{  
  UCommand_LOAD *ret = new UCommand_LOAD(mainnode);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command 
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UCommand_LOAD::print(int l)
{
  char tabb[100];  

  strcpy(tabb,"");
  for (int i=0;i<l;i++)
    strcat(tabb," ");

  if (tag) { ::urbiserver->debug("%s Tag:[%s] ",tabb,tag->str());}
  else ::urbiserver->debug("%s",tabb);

  ::urbiserver->debug("LOAD\n");
}

