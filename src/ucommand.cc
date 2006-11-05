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

#include <cmath>
#include <cstdlib>
#include <cstdio>

#include <sstream>

#include "ucommand.h"
#include "uasynccommand.h"
#include "uconnection.h"
#include "userver.h"
#include "ucallid.h"
#include "utypes.h"
#include "uobject/uobject.hh"
#include "ueventhandler.h"
#include "ueventcompound.h"
#include "ueventinstance.h"
#include "uatcandidate.h"

#if (__GNUC__ == 2)
static const string left = "";
#endif

namespace
{
  UNodeType nodeType_foreach (UCommandType type)
  {
    switch (type)
      {
      case CMD_FOREACH:      return USEMICOLON;
      case CMD_FOREACH_PIPE: return UPIPE;
      case CMD_FOREACH_AND:  return UAND;
      default:
	abort ();
      }
  }

  UNodeType nodeType_loopn (UCommandType type)
  {
    switch (type)
      {
      case CMD_LOOPN:      return USEMICOLON;
      case CMD_LOOPN_PIPE: return UPIPE;
      case CMD_LOOPN_AND:  return UAND;
      default:
	abort ();
      }
  }

  UNodeType nodeType_for (UCommandType type)
  {
    switch (type)
      {
      case CMD_FOR:      return USEMICOLON;
      case CMD_FOR_PIPE: return UPIPE;
      case CMD_FOR_AND:  return UAND;
      default:
	abort ();
      }
  }

}


char tmpbuffer[UCommand::MAXSIZE_TMPMESSAGE];  ///< temporary global string
MEMORY_MANAGER_INIT(UCommand);

// **************************************************************************
//! UCommand constructor.
/*! The parameter 'type' is required here to describe the type of the command.

    \param type is the command type
*/
UCommand::UCommand(UCommandType _type)
{

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
  flag_nbTrue2      = 0;
  flag_nbTrue4      = 0;
  morphed           = false;
  tagInfo           = 0;
  /*XXX todo: L1:remove this, assert to ensure a setTag is called before use
  L2: pass a tag or a command ptr to ctor
  */
  tag="";
  if (::urbiserver->systemcommands)
    setTag("__system__"); //untouchable
  else
    setTag("notag"); //untouchable
}

//! UCommand destructor.
UCommand::~UCommand()
{
  unsetTag();
  delete flags;
}

UCommandStatus
UCommand::execute(UConnection*)
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

  command->setTag(this);

  if (flags)
    {
      command->flags = flags->copy();
      if (!command->flags)
	return UMEMORYFAIL;
    }
  return USUCCESS;
}




//! Print command
void
UCommand::print(int)
{
}

//! Command auto morphing according to group hierarchy
UCommand*
UCommand::scanGroups(UVariableName** (UCommand::*refName)(),
                     bool with_nostruct)
{
  UVariableName **varname = ((*this).*refName)();
  if (!varname) return 0; // we are in a non broadcastable command
  UString       *devicename = (*varname)->getDevice();
  UString       *method     = (*varname)->getMethod();

  HMgrouptab::iterator hmg;
  if ((!(*varname)->rooted) && (devicename))
    {
      UGroup *oo = 0;
      if (((*varname)->nostruct) && (with_nostruct))
	hmg = ::urbiserver->grouptab.find(method->str());
      else
	hmg = ::urbiserver->grouptab.find(devicename->str());

      if (hmg != ::urbiserver->grouptab.end())
	oo = (*hmg).second;

      if ((oo) && (oo->members.size() > 0))
	{
	  UCommand *gplist = 0;
	  UCommand *gplist_prev = 0;
	  UCommand *clone;
	  UNamedParameters *varindex;

	  for (std::list<UString*>::iterator retr = oo->members.begin();
	       retr != oo->members.end();
	       retr++)
	    {
	      clone = copy();
	      delete (*((clone->*refName)()));

	      if ((*varname)->index)
		varindex = (*varname)->index->copy();
	      else
		varindex = 0;

	      if (((*varname)->nostruct) && (with_nostruct))
		*((clone->*refName)()) = new UVariableName(devicename->copy(),
							   (*retr)->copy(),
							   false,
							   varindex);
	      else
		*((clone->*refName)()) = new UVariableName((*retr)->copy(),
							   method->copy(),
							   false,
							   varindex);

	      (*(clone->*refName)())->isnormalized = (*varname)->isnormalized;
	      (*(clone->*refName)())->deriv = (*varname)->deriv;
	      (*(clone->*refName)())->varerror = (*varname)->varerror;
	      (*(clone->*refName)())->nostruct = (*varname)->nostruct;
	      (*(clone->*refName)())->id_type = (*varname)->id_type;
	      (*(clone->*refName)())->local_scope = (*varname)->local_scope;

	      gplist = (UCommand*) new UCommand_TREE(UAND,
						     clone,
						     gplist_prev);
	      gplist_prev = gplist;
	    }

	  morph = (UCommand*) gplist;

	  (*varname)->rooted = true;
	  (*varname)->fromGroup = true;
	  persistant = false;
	  return morph;
	}
    }

  return 0;
}


TagInfo*
TagInfo::insert(HMtagtab &tab)
{
  HMtagtab::iterator i
    = tab.insert(HMtagtab::value_type(name, *this)).first;
  TagInfo * result = &i->second;

  //remove last part of tag
  size_t pos = name.find_last_of('.');
  if (pos == std::string::npos) //we reached base tag
    return result;

  std::string subtag = name.substr(0, name.length()-pos-1);
  HMtagtab::iterator it = tab.find(subtag);
  TagInfo *parent;
  if (it == tab.end())
  {
    TagInfo t;
    t.blocked = t.frozen = false;
    t.name = subtag;
    parent = t.insert(urbiserver->tagtab);
  }
  else
    parent = &it->second;

  parent->subTags.push_back(result);
  result->parent = parent;
  result->parentPtr = --parent->subTags.end();

  return result;
}


void
UCommand::setTag(const std::string & tag)
{
  if (tag==this->tag)
    return;
  if (tag != "")
    unsetTag();
  this->tag = tag;

  TagInfo* ti;
  HMtagtab::iterator it = urbiserver->tagtab.find(tag);

  if (it == urbiserver->tagtab.end())
  {
    TagInfo t;
    t.blocked = t.frozen = false;
    t.name = tag;

    ti = t.insert(urbiserver->tagtab);

  }
  else
    ti = &it->second;

  //add ourself to the taginfo, add it to ourself
  ti->commands.push_back(this);
  tagInfoPtr = --ti->commands.end();
  tagInfo = ti; //we know he won't die before us

}

void
UCommand::setTag(UCommand * cmd)
{
  tag = cmd->tag;
  tagInfo = cmd->tagInfo;
  if (tagInfo)
  {
    tagInfo->commands.push_back(this);
    tagInfoPtr = --tagInfo->commands.end();
  }
}


void
UCommand::unsetTag()
{
  if (!tagInfo)
    return; //nothing to do
  tagInfo->commands.erase(tagInfoPtr);
  TagInfo * ti = tagInfo;
  while (ti && ti->commands.empty() && ti->subTags.empty()
      && !ti->frozen && !ti->blocked)
  {
    //remove from parent list
    if (ti->parent)
      ti->parent->subTags.erase(ti->parentPtr);
    //remove from hash table
    TagInfo * next = ti->parent;
    urbiserver->tagtab.erase(urbiserver->tagtab.find(ti->name));
    //try again on our parent
    ti = next;
  }
}

bool
UCommand::isFrozen()
{
  TagInfo * t = tagInfo;

  while (t)
  {
    if( t->frozen)
      return true;
    t = t->parent;
  }
  return false;
}

bool
UCommand::isBlocked()
{
  TagInfo * t = tagInfo;
  while (t)
  {
    if( t->blocked)
      return true;
    t = t->parent;
  }
  return false;
}

MEMORY_MANAGER_INIT(UCommand_TREE);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
    The background parameter lets the tree execute in background.
    This is useful for the LOAD command which should be run in bg and
    still cannot be persistant (like a AT or WHENEVER).
*/
UCommand_TREE::UCommand_TREE( UNodeType node,
			      UCommand* command1,
			      UCommand* command2)
  : UCommand( CMD_TREE )
{
  ADDOBJ(UCommand_TREE);
  this->command1    = command1;
  this->command2    = command2;
  this->node        = node;

  if (command1)
    {
      command1->up = this;
      command1->position = &this->command1;
    }
  if (command2)
    {
      command2->up = this;
      command2->position = &this->command2;
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

  delete command1;
  delete command2;
  // this frees the local variable for the function call, including
  // the function parameters
  delete callid;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_TREE::execute(UConnection*)
{
  return URUNNING;
}

namespace
{
  // FIXME: Should take a const arg, but does not work currently.
  template <typename T>
  inline
  T*
  ucopy (T* t)
  {
    return t ? t->copy () : 0;
  }
}

//! UCommand subclass hard copy function
UCommand*
UCommand_TREE::copy()
{
  UCommand_TREE *ret = new UCommand_TREE(node,
					 ucopy (command1),
					 ucopy (command2));
  copybase(ret);
  return (UCommand*)ret;
}

//! Deletes sub commands marked for deletion after a stop command
void
UCommand_TREE::deleteMarked()
{
  int go_to = 1;
  UCommand_TREE *tree = this;

  while (tree != up)
  {
    if (tree->command1 && go_to == 1)
      if (tree->command1->toDelete)
	{
	  delete tree->command1;
	  tree->command1 = 0;
	}
      else if (tree->command1->type == CMD_TREE)
	{
	  tree = (UCommand_TREE*) tree->command1;
	  go_to = 1;
	  continue;
	}

    if (tree->command2 && go_to >= 1)
      if (tree->command2->toDelete)
	{
	  delete tree->command2;
	  tree->command2 = 0;
	}
      else if (tree->command2->type == CMD_TREE)
	{
	  tree = (UCommand_TREE*) tree->command2;
	  go_to = 1;
	  continue;
	}

    go_to = 2;
    if (tree->up)
      if (*tree->position == tree->up->command2)
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%sTag:[%s] toDelete=%d ",
                      tabb,
                      getTag().c_str(),
                      toDelete);

  switch (node)
    {
    case UAND:       ::urbiserver->debug("Tree AND "); break;
    case UPIPE:      ::urbiserver->debug("Tree PIPE "); break;
    case USEMICOLON: ::urbiserver->debug("Tree SEMICOLON "); break;
    case UCOMMA:     ::urbiserver->debug("Tree COMMA "); break;
    default:         ::urbiserver->debug("UNKNOWN TREE!\n");
    }

  ::urbiserver->debug("(%ld:%ld) :\n", (long)this, (long)status);
  if (command1)
    {
      ::urbiserver->debug("%s  Com1 (%ld:%d) up=%ld:\n",
			  tabb,
			  (long)command1, command1->status, (long)command1->up);
      command1->print(l+3);
    }
  if (command2)
    {
      ::urbiserver->debug("%s  Com2 (%ld:%d) up=%ld:\n",
			  tabb,
			  (long)command2, command2->status, (long)command2->up);
      command2->print(l+3);
    }
  ::urbiserver->debug("%sEND TREE ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_ASSIGN_VALUE);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_ASSIGN_VALUE::UCommand_ASSIGN_VALUE(UVariableName *variablename,
					     UExpression* expression,
					     UNamedParameters *parameters,
					     bool defkey)
  : UCommand(CMD_ASSIGN_VALUE)
{
  ADDOBJ(UCommand_ASSIGN_VALUE);
  this->variablename= variablename;
  this->expression  = expression;
  this->parameters  = parameters;
  finished          = false;
  this->method      = 0;
  this->devicename  = 0;
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
  delete expression;
  delete variablename;
  delete parameters;
  delete tmp_phase;
  delete tmp_time;

  if (assigned)
    {
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
  UValue *modifier;
  //UValue *value;
  ufloat currentTime;
  UNamedParameters *modif;
  //UVariable *vari;

  // General initializations
  if (!variable)
  {
    variable = variablename->getVariable(this, connection);
    if (!variablename->getFullname())
      return ( status = UCOMPLETED );
    method = variablename->getMethod();
    devicename = variablename->getDevice();
  }
  currentTime = connection->server->lastTime();

  // Wait in queue if needed
  if (variable)
    if (variable->blendType == UQUEUE && variable->nbAverage > 0)
      return status;

  // Broadcasting
  if (scanGroups(&UCommand::refVarName, true))
    return ( status = UMORPH );

  // Function call
  // morph into the function code
  if (expression->type == EXPR_FUNCTION)
  {
    UString* functionname =
      expression->variablename->buildFullname(this, connection);
    if (!functionname)
      return ( status = UCOMPLETED );

    if (scanGroups(&UCommand::refVarName2, true))
      return ( status = UMORPH );

    UFunction *fun;
    HMfunctiontab::iterator hmf;

    ////// EXTERNAL /////

    HMbindertab::iterator it =
      ::urbiserver->functionbindertab.find(functionname->str());
    if ((it != ::urbiserver->functionbindertab.end()) &&
        (
         ( (expression->parameters)
           && (it->second->nbparam == expression->parameters->size()))
         ||
         ((!expression->parameters)
          && (it->second->nbparam==0))) &&
        (!it->second->monitors.empty()))
    {
      int UU = unic();
      char tmpprefix[1024];
      snprintf(tmpprefix, 1024, "[0,\"%s__%d\",\"__UFnctret.EXTERNAL_%d\"",
               functionname->str(), it->second->nbparam, UU);

      for (std::list<UMonitor*>::iterator it2 = it->second->monitors.begin();
           it2 != it->second->monitors.end();
           it2++)
      {
        (*it2)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
        (*it2)->c->send((const ubyte*)tmpprefix, strlen(tmpprefix));
        for (UNamedParameters *pvalue = expression->parameters;
             pvalue != 0;
             pvalue = pvalue->next)
        {
          (*it2)->c->send((const ubyte*)",", 1);
          UValue* valparam = pvalue->expression->eval(this, connection);
          valparam->echo((*it2)->c);
        }
        (*it2)->c->send((const ubyte*)"]\n", 2);
      }

      persistant = false;
      sprintf(tmpbuffer,
              "{waituntil(isdef(__UFnctret.EXTERNAL_%d))|"
              "%s=__UFnctret.EXTERNAL_%d|delete __UFnctret.EXTERNAL_%d}",
              UU, variablename->getFullname()->str(), UU, UU);

      morph = (UCommand*)
        new UCommand_EXPR
        (
         new UExpression
         (
          EXPR_FUNCTION,
          new UVariableName
          (
           new UString("global"),
           new UString("exec"),
           false,
           (UNamedParameters *)0),
          new UNamedParameters
          (
           new UExpression
           (
            EXPR_VALUE,
            new UString(tmpbuffer)
           )
          )
         )
        );
      return ((status = UMORPH));
    }


    ////// INTERNAL /////

    ////// user-defined /////

    hmf = ::urbiserver->functiontab.find(functionname->str());
    bool found = (hmf != ::urbiserver->functiontab.end());
    if (!found)
    {
      //trying inheritance
      const char* devname = expression->variablename->getDevice()->str();
      bool ambiguous;
      fun = 0;
      HMobjtab::iterator itobj;
      if ((itobj = ::urbiserver->objtab.find(devname)) !=
          ::urbiserver->objtab.end())
      {
        fun = itobj->second->searchFunction
          (expression->variablename->getMethod()->str(),
           ambiguous);
        if (ambiguous)
        {
          snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                   "!!! Ambiguous multiple inheritance on function %s\n",
                   functionname->str());
          connection->send(tmpbuffer, getTag().c_str());
          return (status = UCOMPLETED);
        }
      }
    }
    else
      fun = hmf->second;


    if (fun)
    {
      if ( ( expression->parameters
             && fun->nbparam()
             && expression->parameters->size() != fun->nbparam())
           || ( expression->parameters && !fun->nbparam())
           || ( !expression->parameters && fun->nbparam()) )
      {
        snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                 "!!! invalid number of arguments for %s"
                 " (should be %d params)\n",
                 functionname->str(), fun->nbparam());
        connection->send(tmpbuffer, getTag().c_str());

        return ((status = UCOMPLETED));
      }

      persistant = false;
      UVariableName* resultContainer =
        new UVariableName(
                          new UString("__UFnct"),
                          new UString("__result__"),
                          true,
                          (UNamedParameters*)0);

      morph = (UCommand*)
        new UCommand_TREE
        (
         UPIPE,
         fun->cmdcopy(),
         new UCommand_ASSIGN_VALUE
         (
          variablename->copy(),
          new UExpression(EXPR_VARIABLE,
                          resultContainer),
          (UNamedParameters*)0
         )
        );


      if (morph)
      {
        sprintf(tmpbuffer, "__UFnct%d", unic());

        ((UCommand_TREE*)morph)->callid =
          new UCallid(tmpbuffer,
                      expression->variablename->device->str(),
                      (UCommand_TREE*)morph);

        resultContainer->nameUpdate(((UCommand_TREE*)morph)->callid->str(),
                                    "__result__");
        if (!((UCommand_TREE*)morph)->callid)
          return (status = UCOMPLETED);
        ((UCommand_TREE*)morph)->connection = connection;

        UNamedParameters *pvalue = expression->parameters;
        UNamedParameters *pname  = fun->parameters;
        for (;
             pvalue != 0;
             pvalue = pvalue->next, pname = pname->next)
        {
          UValue* valparam = pvalue->expression->eval(this, connection);
          if (!valparam)
          {
            connection->send("!!! EXPR evaluation failed\n", getTag().c_str());
            return (status = UCOMPLETED);
          }

          ((UCommand_TREE*)morph)->callid->store
            (new UVariable(((UCommand_TREE*)morph)->callid->str(),
                           pname->name->str(),
                           valparam));
        }
      }

      return status = UMORPH;
    } // fi: function exists


    ////// module-defined /////

    urbi::UTable::iterator hmfi =
      urbi::functionmap.find(functionname->str());
    if (hmfi != urbi::functionmap.end())
    {
      bool found_function = false;

      for (std::list<urbi::UGenericCallback*>::iterator cbi =
           hmfi->second.begin();
           ((cbi != hmfi->second.end()) && (!found_function));
           cbi++)
      {
        if ( ( expression->parameters
               && expression->parameters->size() == (*cbi)->nbparam)
             || ( !expression->parameters && !(*cbi)->nbparam) )
        {
          // here you could spawn a thread... if only Aprios
          // knew how to!
          urbi::UList tmparray;
          for (UNamedParameters *pvalue = expression->parameters;
               pvalue != 0;
               pvalue = pvalue->next)
          {
            UValue* valparam = pvalue->expression->eval(this, connection);
            if (!valparam)
            {
              connection->send("!!! EXPR evaluation failed\n",
                               getTag().c_str());
              return (status = UCOMPLETED);
            }
            // urbi::UValue do not see ::UValue, so it must
            // be valparam who does the job.
            urbi::UValue *tmpvalue = valparam->urbiValue();
            tmparray.array.push_back(tmpvalue);
          }

          delete expression;
          expression = new UExpression
            (EXPR_VALUE,
             new UValue( (*cbi)->__evalcall(tmparray)));
          found_function = true;
        }
      }
    }
  } // fi: expr == function


  ////////////////////////////////////////
  // Initialization phase (first pass)
  ////////////////////////////////////////

  if (status == UONQUEUE)
  {
    // object aliasing here
    if (variablename->nostruct
        && expression->type == EXPR_VARIABLE
        && expression->variablename
        && expression->variablename->nostruct)
    {
      UString* objname = expression->variablename->id;

      HMobjtab::iterator objit = ::urbiserver->objtab.find(objname->str());
      if (objit != ::urbiserver->objtab.end())
      {
        // the use of 'id' is a hack that works.
        HMaliastab::iterator hmi =
          ::urbiserver->objaliastab.find(variablename->id->str());
        if (hmi != ::urbiserver->objaliastab.end())
          (*hmi).second->update(objname);
        else
        {
          UString* objalias = new UString(variablename->method);
          ::urbiserver->objaliastab[objalias->str()] = new UString(objname);
        }
        return status = UCOMPLETED;
      }
    }

    // Objects cannot be assigned
    if (variable
        && !variablename->fromGroup
        && variable->value->dataType == DATA_OBJ)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Warning: %s type mismatch: no object assignment\n",
               variablename->getFullname()->str());
      connection->send(tmpbuffer, getTag().c_str());
      return (status = UCOMPLETED);
    }

    // Strict variable definition checking
    if (!variable && connection->server->defcheck && !defkey)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Unknown identifier: %s\n",
               variablename->getFullname()->str());
      connection->send(tmpbuffer, getTag().c_str());
    }

    // Check the +error flag
    UNamedParameters *param = flags;
    errorFlag = false;
    while (param)
    {
      if (param->name &&
          param->name->equal("flag") &&
          param->expression &&
          param->expression->val == 2) // 2 = +error
        errorFlag = true;

      param = param->next;
    }

    // UCANCEL mode
    if (variable && variable->blendType == UCANCEL)
    {
      variable->nbAverage = 0;
      variable->cancel = this;
    }

    // eval the right side of the assignment and check for errors
    target = expression->eval(this, connection);
    //if ((target == 0) || (target->dataType == DATA_VOID))
    if (target == 0)
      return (status = UCOMPLETED);

    // Check type compatibility if the left side variable already exists
    if (variable &&
        variable->value->dataType != DATA_VOID &&
        target->dataType != variable->value->dataType)
    {
      if (::urbiserver->defcheck)
      {
        snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                 "!!! Warning: %s type mismatch\n",
                 variablename->getFullname()->str());
        connection->send(tmpbuffer, getTag().c_str());
        delete target;
        return status = UCOMPLETED;
      }

      delete variable;
      variable = 0;
    }

    // VOID init ///////////////////
    //////////////////////////////////

    if (target->dataType == DATA_VOID) // VOID
    {
      if (variable) // the variable already exists
        variable->set(target);
      else
      {
        variable = new UVariable(variablename->getFullname()->str(),
                                 target->copy());
        if (!variable) return ( status = UCOMPLETED );
        connection->localVariableCheck(variable);
        variable->updated();
      }

      delete target;
      return ((status = UCOMPLETED));
    }


    // STRING init ///////////////////
    //////////////////////////////////
    if (target->dataType == DATA_STRING)  // STRING
    {
      // Handle String Composition
      if (parameters != 0)
      {
        char *result = (char*)malloc(sizeof(char)
                                     * (65000+target->str->len()));
        char *possub;

        if (result)
        {
          strcpy (result, target->str->str());
          modif = parameters;
          while (modif)
          {
            modifier = modif->expression->eval(this, connection);
            if (!modifier)
            {
              snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                       "!!! String composition failed\n");
              connection->send(tmpbuffer, getTag().c_str());
              delete target;
              return ((status = UCOMPLETED));
            }

            if (modifier->dataType == DATA_NUM)
            {
              modifier->dataType = DATA_STRING;
              std::ostringstream ostr;
              ostr << modifier->val;
              modifier->str = new UString(ostr.str().c_str());
            }

            snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                     "$%s", modif->name->str());

            if (strstr(modifier->str->str(), tmpbuffer) == 0)
              while ((possub = strstr(result, tmpbuffer)))
              {
                memmove(possub + modifier->str->len(),
                        possub + strlen(tmpbuffer),
                        strlen(result) -
                        (int)(possub - result) -
                        strlen(tmpbuffer)+1);

                strncpy (possub,
                         modifier->str->str(),
                         modifier->str->len());
              }

            delete modifier;
            modif = modif->next;
          }
          target->str->update(result);
          free(result);
        }
      } // end of string composition: target is up to date

      // Assignment
      if (variable) // the variable already exists
        variable->set(target);
      else
      {
        variable = new UVariable(variablename->getFullname()->str(),
                                 target->copy());
        if (!variable)
          return ( status = UCOMPLETED );
        connection->localVariableCheck(variable);
        variable->updated();
      }

      delete target;
      return ((status = UCOMPLETED));
    }

    // BINARY init ///////////////////
    //////////////////////////////////
    if (target->dataType == DATA_BINARY)  // BINARY
    {
      // Assignment
      if (variable) // the variable already exists
        variable->set(target);
      else
      {
        variable = new UVariable(variablename->getFullname()->str(),
                                 target->copy());
        if (!variable)
          return ((status = UCOMPLETED));
        connection->localVariableCheck(variable);
        variable->updated();
      }

      delete (target);
      return ((status = UCOMPLETED));
    }

    // LIST init ///////////////////
    //////////////////////////////////

    if (target->dataType == DATA_LIST) // LIST
    {
      // Assignment
      if (variable) // the variable already exists
        variable->set(target);
      else
      {
        variable = new UVariable(variablename->getFullname()->str(),
                                 target->copy());
        if (!variable)
          return ( status = UCOMPLETED );
        connection->localVariableCheck(variable);
        variable->updated();
      }

      delete target;
      return (status = UCOMPLETED);
    }

    // NUM init ///////////////////
    //////////////////////////////////
    if (target->dataType == DATA_NUM) // NUM
    {
      bool controlled = false; // is a virtual "time:0" needed?
      targetval = target->val;

      // Handling normalized correction
      if (variable && variablename->isnormalized)
      {
        if (variable->rangemin == -UINFINITY
            || variable->rangemax ==  UINFINITY)
        {
          if (!variablename->fromGroup)
          {
            snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                     "!!! Impossible to normalize:"
                     " no range defined for variable %s\n",
                     variablename->getFullname()->str());
            connection->send(tmpbuffer, getTag().c_str());
          }
          delete target;
          return ((status = UCOMPLETED));
        }

        if (targetval < 0) targetval = 0;
        if (targetval > 1) targetval = 1;

        targetval = variable->rangemin + targetval *
          (variable->rangemax - variable->rangemin);
      }

      // Store init time
      starttime = currentTime;

      // Handling FLAGS
      if (parameters)
      {
        // Check if sinusoidal (=> no start value needed = no integrity check)
        modif = parameters;
        bool sinusoidal = false;
        while (modif)
        {
          if (modif->name->equal("sin")
              || modif->name->equal("cos"))
            sinusoidal = true;
          modif = modif->next;
        }

        // Checking integrity (variable exists), if not sinusoidal
        if ((variable == 0) && (!sinusoidal))
        {
          snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                   "!!! Modificator error: %s unknown (no start value)\n",
                   variablename->getFullname()->str());
          if (!variablename->fromGroup)
            connection->send(tmpbuffer, getTag().c_str());
          delete target;
          return ((status = UCOMPLETED));
        }

        speed    = 0;

        // Initialize modifiers

        bool found;
        modif = parameters;

        while (modif)
        {
          if ((!modif->expression) ||
              (!modif->name))
          {
            snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                     "!!! Invalid modifier\n");
            connection->send(tmpbuffer, getTag().c_str());

            delete target;
            return ((status = UCOMPLETED));
          }

          found = false;

          if (modif->name->equal("sin"))
          {
            modif_sin = modif->expression;
            found = true;
            controlled = true;
          }

          if (modif->name->equal("cos"))
          {
            modif_sin = modif->expression;
            tmp_phase = new UExpression(EXPR_VALUE, PI/ufloat(2));
            modif_phase = tmp_phase;
            found = true;
            controlled = true;
          }

          if (modif->name->equal("ampli"))
          {
            modif_ampli = modif->expression;
            found = true;
          }

          if (modif->name->equal("smooth"))
          {
            modif_smooth = modif->expression;
            found = true;
            controlled = true;
          }

          if (modif->name->equal("time"))
          {
            modif_time = modif->expression;
            found = true;
            controlled = true;
          }

          if (modif->name->equal("speed"))
          {
            modif_speed = modif->expression;
            found = true;
            controlled = true;
          }

          if (modif->name->equal("accel"))
          {
            modif_accel = modif->expression;
            found = true;
            controlled = true;
          }

          if (modif->name->equal("adaptive"))
          {
            modif_adaptive = modif->expression;
            found = true;
          }

          if (modif->name->equal("phase"))
          {
            modif_phase = modif->expression;
            found = true;
          }

          if (modif->name->equal("getphase"))
          {
            if (modif->expression->type != EXPR_VARIABLE)
            {
              snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                       "!!! a variable is expected for"
                       " the 'getphase' modifier\n");
              connection->send(tmpbuffer, getTag().c_str());
              return ((status = UCOMPLETED));
            }
            modif_getphase = modif->expression->variablename;
            found = true;
          }


          if (modif->name->equal("timelimit"))
          {
            modifier = modif->expression->eval(this, connection);
            if ( (!modifier) ||
                 (modifier->dataType != DATA_NUM) )
            {
              snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                       "!!! Invalid modifier value\n");
              connection->send(tmpbuffer, getTag().c_str());

              delete modifier;
              delete target;
              return ((status = UCOMPLETED));
            }
            endtime = currentTime + modifier->val;
            delete modifier;
            found = true;
          }

          if (!found)
          {
            snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                     "!!! Unkown modifier name\n");
            connection->send(tmpbuffer, getTag().c_str());

            delete target;
            return (status = UCOMPLETED);
          }

          modif = modif->next;
        }
      } // end FLAGS handling

      // create var if it does not already exist
      if (!variable)
      {
        variable = new UVariable(variablename->getFullname()->str(),
                                 target->copy());
        if (!variable)
          return ( status = UCOMPLETED );
        connection->localVariableCheck(variable);
      }

      // correct the type of VOID variables (comming from a def)
      if (variable->value->dataType == DATA_VOID)
        variable->value->dataType = DATA_NUM;

      // virtual "time:0" if no modifier specified (controlled == false)
      if (!controlled)
      {
        // no controlling modifier => time:0
        tmp_time = new UExpression(EXPR_VALUE, ufloat(0));
        modif_time = tmp_time;
      }

      // clean the temporary target UValue
      delete target;

      // UDISCARD mode
      if (variable->blendType == UDISCARD &&
          variable->nbAssigns > 0)
        return (status = UCOMPLETED);

      // init valarray for a "val" assignment
      ufloat *targetvalue;
      if (!controlled)
        targetvalue = &(variable->value->val); // prevents a read access
      else
        targetvalue =  &(variable->get()->val);

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

  if (status == URUNNING)
  {
    if (finished)
      if (variable->reloop)
        finished = false;
      else
        return (status = UCOMPLETED);

    // Cancel if needed
    if (variable->blendType == UCANCEL && variable->cancel != this)
      return (status = UCOMPLETED);

    // Discard if needed
    if (variable->blendType == UDISCARD && variable->nbAverage > 0)
      return (status = UCOMPLETED);

    // In normal mode, there is always only one value to consider
    if (variable->blendType == UNORMAL)
      variable->nbAverage = 0;

    // In add mode, the current value is always added
    if (variable->blendType == UADD && variable->nbAverage > 1)
      variable->nbAverage = 1;

    ///////////////////////////////
    // Process the active modifiers
    if (processModifiers(connection, currentTime) == UFAIL)
      return (status = UCOMPLETED);
    ///////////////////////////////

    // absorb average and set reinit list to set
    // nbAverage back to 0 after work()
    if (variable->blendType != UADD)
      *valtmp = *valtmp / (ufloat)(variable->nbAverage+1);
    variable->nbAverage++;

    if (variable->activity == 0)
      connection->server->reinitList.push_front(variable);
    variable->activity = 1;

    // Variable updating or signal for update (modified)
    // UMIX and UADD are treated separatly
    // in the processing of the reinit list
    // because we don't want to have several calls to notifyWrite (when the
    // variable is with a notifyWrite==true flag) for each intermediary step
    // of the UADD and UMIX aggregation, but only at the end.
    // Hence the report to reinit list processing.

    if (variable->blendType != UMIX && variable->blendType != UADD)
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
  return ((status = UCOMPLETED));
}

// Processing the modifiers in a URUNNING assignment
UErrorValue
UCommand_ASSIGN_VALUE::processModifiers(UConnection* connection,
					ufloat currentTime)
{
  ufloat deltaTime = connection->server->getFrequency();
  ufloat currentVal = variable->get()->val;
  ufloat phase, amplitude;

  // Adaptive mode? (only for "speed" and "time")
  bool adaptive = false;
  if (modif_adaptive)
    if ((tmpeval = modif_adaptive->eval(this, connection)))
      {
	adaptive = (tmpeval->val != 0);
	delete tmpeval;
      }

  // timeout
  if (endtime != -1 && currentTime >= endtime)
    {
      finished = true;
      *valtmp = variable->nbAverage * *valtmp + currentVal;
      return USUCCESS;
    }

  // speedmin conversion for convenience
  speedmin = variable->speedmin / 1000.;

  // time
  if (modif_time)
    {
      if (adaptive)
	if (ABSF(currentVal - targetval) <= variable->delta)
	  {
	    finished = true;
	    *valtmp = variable->nbAverage * *valtmp +
	      targetval;
	  }

      if ((tmpeval = modif_time->eval(this, connection)))
	{
	  targettime = ABSF(tmpeval->val);
          //enforce speedmax here
          if (variable->speedmax != UINFINITY)
            if (targettime <  ABSF ((targetval-startval)/ variable->speedmax))
            {
              if (errorFlag)
                connection->send("!!! Warning: request exceeds speedmax."
                                 " Enforcing limitation\n",
		                 getTag().c_str());
              targettime = ABSF ((targetval - startval)/ variable->speedmax);
            }
	  delete tmpeval;
	}

      // check for speedmin
      if ( (targettime > (currentTime - starttime)) &&
	   (ABSF((targetval - currentVal) /
		 (targettime - (currentTime - starttime))) < speedmin))
	{
	  targettime = currentTime - starttime +
	    ABSF(targetval - currentVal)/ speedmin;

	  if (errorFlag && first)
	    connection->send("!!! low speed: increased to speedmin\n",
			     getTag().c_str());
	}

      if (currentTime - starttime + deltaTime >= targettime)
	{
	  if (!adaptive)
	    finished = true;
	  *valtmp = variable->nbAverage * *valtmp +
	    targetval;
	}
      else if (adaptive)
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

      return USUCCESS;
    }

  // smooth
  if (modif_smooth)
    {
      if ((tmpeval = modif_smooth->eval(this, connection)))
	{
	  targettime = ABSF(tmpeval->val);
	  delete tmpeval;
	}

      // test for speedmin (with linear mvt approximation)
      if ( (targettime > (currentTime - starttime)) &&
	   (ABSF((targetval - currentVal) /
		 (targettime - (currentTime - starttime))) < speedmin))
	{
	  targettime = currentTime - starttime +
	    ABSF(targetval - currentVal)/speedmin;

	  if (errorFlag && first)
	    connection->send("!!! low speed: increased to speedmin\n",
			     getTag().c_str());
	}

      if (currentTime - starttime + deltaTime >= targettime)
	{
	  finished = true;
	  *valtmp = variable->nbAverage * *valtmp +
	    targetval;
	}
      else
	*valtmp = variable->nbAverage * *valtmp +
	  startval +
	  ( (targetval - startval) * 0.5 *
	    (ufloat(1)+sin(-(PI/ufloat(2))+ PI*(currentTime - starttime + deltaTime) /
			   targettime
			   ))
	    );
      return USUCCESS;
    }

  //speed
  if (modif_speed)
    {
      if (adaptive)
	if (ABSF(currentVal - targetval) <= variable->delta)
	  {
	    finished = true;
	    *valtmp = variable->nbAverage * *valtmp +
	      targetval;
	  }

      if ((tmpeval = modif_speed->eval(this, connection)))
	{
	  speed = ABSF(tmpeval->val);
	  delete tmpeval;
	}

      if (speed == 0)
	speed = 0.001;

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
		 (targettime - (currentTime - starttime))) < speedmin))
	{
	  targettime = currentTime - starttime +
	    ABSF(targetval - currentVal)/ speedmin;

	  if (errorFlag && first)
	    connection->send("!!! low speed: increased to speedmin\n",
                             getTag().c_str());
	}

      if (currentTime - starttime + deltaTime >= targettime)
	{
	  if (!adaptive)
	    finished = true;
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

      return USUCCESS;
    }

  //accel
  if (modif_accel)
    {
      if ((tmpeval = modif_accel->eval(this, connection)))
	{
	  accel = ABSF(tmpeval->val/1000.);
	  delete tmpeval;
	}

      if (targetval < startval) accel = -accel;

      if (accel == 0) accel = 0.001;

      if (variablename->isnormalized)
	accel = accel * (variable->rangemax - variable->rangemin);

      targettime = sqrt ( 2 * ABSF(targetval - startval)
                          / (ABSF(accel)/1000.));

      if (currentTime - starttime + deltaTime >= targettime)
	{
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
  if (modif_sin)
    {
      targettime = 0;
      if ((tmpeval = modif_sin->eval(this, connection)))
	{
	  targettime = ABSF(tmpeval->val);
	  delete tmpeval;
	}
      if (targettime == 0) targettime = 0.1;

      phase = 0;
      if (modif_phase &&
	  (tmpeval = modif_phase->eval(this, connection)))
      {
	phase = tmpeval->val;
	delete tmpeval;
      }

      amplitude = 0;
      if (modif_ampli &&
	  (tmpeval = modif_ampli->eval(this, connection)))
      {
	amplitude = tmpeval->val;
	delete tmpeval;
      }
      if (variablename->isnormalized)
	amplitude = amplitude * (variable->rangemax - variable->rangemin);

      if (expression &&
	  (tmpeval = expression->eval(this, connection)))
      {
	targetval = tmpeval->val;
	if (variablename->isnormalized)
	  {
	    if (targetval < 0) targetval = 0;
	    if (targetval > 1) targetval = 1;

	    targetval = variable->rangemin + targetval *
	      (variable->rangemax - variable->rangemin);
	  }
	delete tmpeval;
      }

      ufloat intermediary;
      intermediary = targetval + amplitude * sin(phase +
						 (PI*ufloat(2))*( (currentTime - starttime + deltaTime) /
								  targettime ));
      if (modif_getphase)
	{
	  UVariable *phasevari = modif_getphase->getVariable(this, connection);
	  if (!phasevari)
	    {
	      if (!modif_getphase->getFullname())
		{
		  connection->send("!!! invalid phase variable name\n", getTag().c_str());

		  return UFAIL;
		}
	      phasevari = new UVariable(modif_getphase->getFullname()->str(),
                                        ufloat(0));
	      connection->localVariableCheck(phasevari);
	    }

	  UValue *phaseval = phasevari->value;

	  phaseval->val = (phase +
			   (PI*ufloat(2))*( (currentTime - starttime + deltaTime) /
					    targettime ));
	  int n = (int)(phaseval->val / (PI*ufloat(2)));
	  if (n<0) n--;
	  phaseval->val = phaseval->val - n  * (PI*ufloat(2));
	}

      *valtmp = variable->nbAverage * *valtmp + intermediary;

      return USUCCESS;
    }
  return USUCCESS;
}


//! UCommand subclass hard copy function
UCommand*
UCommand_ASSIGN_VALUE::copy()
{
  UCommand_ASSIGN_VALUE *ret =
    new UCommand_ASSIGN_VALUE(ucopy (variablename),
			      ucopy (expression),
			      ucopy (parameters));

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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] toDelete=%d ",
                      tabb,
                      getTag().c_str(), toDelete);

  ::urbiserver->debug("ASSIGN VALUE:\n");

  if (variablename)
  {
    ::urbiserver->debug("%s  Variable:", tabb);
    variablename->print(); ::urbiserver->debug("\n");
  };
  if (expression)
  {
    ::urbiserver->debug("%s  Expr:", tabb);
    expression->print(); ::urbiserver->debug("\n");
  };
  if (parameters)
  {
    ::urbiserver->debug("%s  Param:{", tabb);
    parameters->print(); ::urbiserver->debug("}\n");
  };
  ::urbiserver->debug("%sEND ASSIGN VALUE ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_ASSIGN_BINARY);
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
  delete variablename;
  LIBERATE(refBinary);
}

//! UCommand subclass execution function
UCommandStatus
UCommand_ASSIGN_BINARY::execute(UConnection *connection)
{
  // General initializations
  if (!variable)
  {
    variable = variablename->getVariable(this, connection);
    if (!variablename->getFullname())
      return ( status = UCOMPLETED );
    method = variablename->getMethod();
    devicename = variablename->getDevice();
  }

  // Broadcasting
  if (scanGroups(&UCommand::refVarName, true)) return ( status = UMORPH );

  // Type checking
  UValue *value;
  if ((variable)
		&& (variable->value->dataType != DATA_BINARY)
		&& (variable->value->dataType != DATA_VOID))
  {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! %s type mismatch\n", variablename->getFullname()->str());
      connection->send(tmpbuffer, getTag().c_str());

      return (status = UCOMPLETED);
    }

  // Create variable if it doesn't exist
  if (!variable)
  {
    value = new UValue();
    value->dataType = DATA_BINARY;
    variable = new UVariable(variablename->getFullname()->str(), value);
    if (!variable) return (status = UCOMPLETED);
    variable->blendType = UQUEUE;

    connection->localVariableCheck(variable);
  }
  else {
    if (variable->value->dataType == DATA_BINARY)
      LIBERATE(variable->value->refBinary);
  }
  variable->value->dataType = DATA_BINARY;
  variable->value->refBinary = refBinary->copy();

  variable->updated();
  return (status = UCOMPLETED);
}

//! UCommand subclass hard copy function
UCommand*
UCommand_ASSIGN_BINARY::copy()
{
  UCommand_ASSIGN_BINARY *ret =
    new UCommand_ASSIGN_BINARY(ucopy (variablename),
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("ASSIGN BINARY:\n");

  if (variablename)
  {
    ::urbiserver->debug("%s  Variable:", tabb);
    variablename->print(); ::urbiserver->debug("\n");
  };
  if (refBinary)
  {
    ::urbiserver->debug("%s  Binary:", tabb);
    refBinary->ref()->print(); ::urbiserver->debug("\n");};
  ::urbiserver->debug("%sEND ASSIGN BINARY ------\n",
                      tabb);
}

MEMORY_MANAGER_INIT(UCommand_ASSIGN_PROPERTY);
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
  delete variablename;
  delete expression;
  delete oper;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_ASSIGN_PROPERTY::execute(UConnection *connection)
{
  UVariable* variable = variablename->getVariable(this, connection);
  if (!variablename->getFullname())
    return ( status = UCOMPLETED );
  variablename->getMethod();
  variablename->getDevice();

  // Broadcasting
  if (scanGroups(&UCommand::refVarName, true)) return ( status = UMORPH );

  // variable existence checking
  if (!variable)
  {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! Variable %s does not exist\n",
               variablename->getFullname()->str());
      if (!variablename->fromGroup)
	connection->send(tmpbuffer, getTag().c_str());
      return ( status = UCOMPLETED );
  }

  // Property handling


  // blend
  if (strcmp(oper->str(), "blend")==0)
  {
    UValue *blendmode = expression->eval(this, connection);
    if (blendmode == 0)
      return (status = UCOMPLETED);

    if (blendmode->dataType != DATA_STRING)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! Invalid blend mode.\n");
      connection->send(tmpbuffer, getTag().c_str());
      return (status = UCOMPLETED);
    }

    if ((variable->value->dataType != DATA_NUM) &&
	(variable->value->dataType != DATA_BINARY))
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! %s type is invalid for mixing\n",
		variablename->getFullname()->str());
      connection->send(tmpbuffer, getTag().c_str());
      return (status = UCOMPLETED);
    }

    if (strcmp(blendmode->str->str(), "normal")==0)
      variable->blendType = UNORMAL;
    else
      if (strcmp(blendmode->str->str(), "mix")==0)
        variable->blendType = UMIX;
      else
        if (strcmp(blendmode->str->str(), "add")==0)
          variable->blendType = UADD;
        else
          if (strcmp(blendmode->str->str(), "discard")==0)
            variable->blendType = UDISCARD;
          else
            if (strcmp(blendmode->str->str(), "queue")==0)
              variable->blendType = UQUEUE;
            else
              if (strcmp(blendmode->str->str(), "cancel")==0)
                variable->blendType = UCANCEL;
              else {
                snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                         "!!! Unknown blend mode: %s\n",
                         blendmode->str->str());
                connection->send(tmpbuffer, getTag().c_str());
                return (status = UCOMPLETED);
              }

    return (status = UCOMPLETED);
  }

  // rangemax
  if (strcmp(oper->str(), "rangemax")==0)
  {
    UValue *nb = expression->eval(this, connection);
    if (nb == 0)
      return (status = UCOMPLETED);

    if (nb->dataType != DATA_NUM)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! Invalid range type. NUM expected.\n");
      connection->send(tmpbuffer, getTag().c_str());
      return (status = UCOMPLETED);
    }

    variable->rangemax = nb->val;
    return (status = UCOMPLETED);
  }

  // delta
  if (strcmp(oper->str(), "delta")==0)
  {
    UValue *nb = expression->eval(this, connection);
    if (nb == 0)
      return (status = UCOMPLETED);

    if (nb->dataType != DATA_NUM)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! Invalid delta type. NUM expected.\n");
      connection->send(tmpbuffer, getTag().c_str());
      return (status = UCOMPLETED);
    }

    variable->delta = nb->val;
    return (status = UCOMPLETED);
  }


  // unit
  if (strcmp(oper->str(), "unit")==0)
  {
    UValue *unitval = expression->eval(this, connection);
    if (unitval == 0)
      return ( status = UCOMPLETED );

    if (unitval->dataType != DATA_STRING)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! Invalid unit type (must be a string).\n");
      connection->send(tmpbuffer, getTag().c_str());
      return ( status = UCOMPLETED );
    }

    if ((variable->value->dataType != DATA_NUM) &&
	(variable->value->dataType != DATA_BINARY))
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! %s type is invalid for unit attribution\n",
		variablename->getFullname()->str());
      connection->send(tmpbuffer, getTag().c_str());
      return ( status = UCOMPLETED );
    }

    if (variable->unit) variable->unit->update(unitval->str->str());
    else
      variable->unit = new UString(unitval->str->str());

    return ( status = UCOMPLETED );
  }

  // rangemin
  if (strcmp(oper->str(), "rangemin")==0)
  {
    UValue *nb = expression->eval(this, connection);
    if (nb == 0)
      return ( status = UCOMPLETED );

    if (nb->dataType != DATA_NUM)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! Invalid range type. NUM expected.\n");
      connection->send(tmpbuffer, getTag().c_str());
      return ( status = UCOMPLETED );
    }

    variable->rangemin = nb->val;
    return ( status = UCOMPLETED );
  }

  // speedmax
  if (strcmp(oper->str(), "speedmax")==0)
  {
    UValue *nb = expression->eval(this, connection);
    if (nb == 0)
      return ( status = UCOMPLETED );

    if (nb->dataType != DATA_NUM)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! Invalid speed type. NUM expected.\n");
      connection->send(tmpbuffer, getTag().c_str());
      return ( status = UCOMPLETED );
    }

    variable->speedmax = nb->val;
    return ( status = UCOMPLETED );
  }

  // speedmin
  if (strcmp(oper->str(), "speedmin")==0)
  {
    UValue *nb = expression->eval(this, connection);
    if (nb == 0)
      return ( status = UCOMPLETED );

    if (nb->dataType != DATA_NUM)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! Invalid speed type. NUM expected.\n");
      connection->send(tmpbuffer, getTag().c_str());
      return ( status = UCOMPLETED );
    }

    variable->speedmin = nb->val;
    return ( status = UCOMPLETED );
  }

  snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	   "!!! Unknown property: %s\n", oper->str());
  connection->send(tmpbuffer, getTag().c_str());
  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_ASSIGN_PROPERTY::copy()
{
  UCommand_ASSIGN_PROPERTY *ret =
    new UCommand_ASSIGN_PROPERTY(ucopy (variablename),
				 ucopy (oper),
				 ucopy (expression));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("ASSIGN PROPERTY [%s]:\n", oper->str());

  if (variablename)
  {
    ::urbiserver->debug("%s  Variable:", tabb);
    variablename->print(); ::urbiserver->debug("\n");
  };
  if (expression)
  {
    ::urbiserver->debug("%s  Expr:", tabb);
    expression->print(); ::urbiserver->debug("\n");
  };

  ::urbiserver->debug("%sEND ASSIGN PROPERTY ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_AUTOASSIGN);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_AUTOASSIGN::UCommand_AUTOASSIGN( UVariableName* variablename,
					  UExpression* expression,
					  int assigntype) :
  UCommand(CMD_ASSIGN_VALUE)
{
  ADDOBJ(UCommand_AUTOASSIGN);
  this->variablename = variablename;
  this->expression   = expression;
  this->assigntype   = assigntype;
}

//! UCommand subclass destructor.
UCommand_AUTOASSIGN::~UCommand_AUTOASSIGN()
{
  FREEOBJ(UCommand_AUTOASSIGN);
  delete variablename;
  delete expression;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_AUTOASSIGN::execute(UConnection*)
{
  if (!variablename || !expression)
    return (status = UCOMPLETED);

  UExpression* extended_expression = 0;

  switch (assigntype)
    {
    case 0: /* += */
      extended_expression =
	new UExpression(EXPR_PLUS,
			new UExpression(EXPR_VARIABLE, variablename->copy()),
			expression->copy());
      break;
    case 1: /* -= */
      extended_expression =
	new UExpression(EXPR_MINUS,
			new UExpression(EXPR_VARIABLE, variablename->copy()),
			expression->copy());
      break;
    }

  if (!extended_expression)
    return ((status = UCOMPLETED));

  morph =  (UCommand*)
    new UCommand_ASSIGN_VALUE(variablename->copy(),
	extended_expression,
	(UNamedParameters*)0,
	false);

  persistant = false;
  return ((status = UMORPH));
}

//! UCommand subclass hard copy function
UCommand*
UCommand_AUTOASSIGN::copy()
{
  UCommand_AUTOASSIGN *ret =
    new UCommand_AUTOASSIGN(ucopy (variablename),
			    ucopy (expression),
			    assigntype);
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command
/*! This function is for debugging purpose only.
    It is not safe, efficient or crash proof. A better version will come later.
*/
void
UCommand_AUTOASSIGN::print(int l)
{
  char tabb[100];

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());


  ::urbiserver->debug("AUTOASSIGN (%d):", assigntype);

  if (variablename)
  {
    ::urbiserver->debug("%s  VariableName:", tabb);
    variablename->print(); ::urbiserver->debug("\n");
  };
  if (expression)
  {
    ::urbiserver->debug("%s  expression:", tabb);
    expression->print(); ::urbiserver->debug("\n");
  };

  ::urbiserver->debug("%sEND AUTOASSIGN ------\n", tabb);
}


MEMORY_MANAGER_INIT(UCommand_EXPR);
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
  delete expression;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_EXPR::execute(UConnection *connection)
{
  HMfunctiontab::iterator hmf;

  if (expression->type == EXPR_FUNCTION)
  {
    // Execution & morphing
    UString* funname =
      expression->variablename->buildFullname(this, connection);
    if (!funname) return ( status = UCOMPLETED );

    // Broadcasting
    if (scanGroups(&UCommand::refVarName, false)) return ( status = UMORPH );

    UFunction *fun;

    ////// EXTERNAL /////
    HMbindertab::iterator it =
      ::urbiserver->functionbindertab.find(funname->str());
    if ((it != ::urbiserver->functionbindertab.end()) &&
		(
			( (expression->parameters)
                          && (it->second->nbparam
                              == expression->parameters->size()))
			||
			((!expression->parameters) && (it->second->nbparam==0))) &&
	(!it->second->monitors.empty()))  {

      int UU = unic();
      char tmpprefix[1024];
      snprintf(tmpprefix, 1024, "[0,\"%s__%d\",\"__UFnctret.EXTERNAL_%d\"",
	  funname->str(), it->second->nbparam, UU);

      for (std::list<UMonitor*>::iterator it2 = it->second->monitors.begin();
	   it2 != it->second->monitors.end();
	   it2++)
      {
	(*it2)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
	(*it2)->c->send((const ubyte*)tmpprefix, strlen(tmpprefix));
	for (UNamedParameters *pvalue = expression->parameters;
	    pvalue != 0;
	    pvalue = pvalue->next)
        {
	  (*it2)->c->send((const ubyte*)",", 1);
	  UValue* valparam = pvalue->expression->eval(this, connection);
	  valparam->echo((*it2)->c);
	}
	(*it2)->c->send((const ubyte*)"]\n", 2);
      }

      persistant = false;
      sprintf(tmpbuffer,
              "{waituntil(isdef(__UFnctret.EXTERNAL_%d))|"
              "%s:__UFnctret.EXTERNAL_%d|delete __UFnctret.EXTERNAL_%d}",
      UU, getTag().c_str(), UU, UU);

      morph = (UCommand*)
      new UCommand_EXPR(
	  new UExpression(
	    EXPR_FUNCTION,
	    new UVariableName(new UString("global"),
                              new UString("exec"),
                              false,
                              (UNamedParameters *)0),
	    new UNamedParameters(
		new UExpression(
		  EXPR_VALUE,
		  new UString(tmpbuffer)
		  )
		)
	    )
	  );

      return ( status = UMORPH );
    }

    ////// INTERNAL /////

    ////// user-defined /////

    hmf = ::urbiserver->functiontab.find(funname->str());
    bool found = (hmf != ::urbiserver->functiontab.end());
    if (!found)
    {
      //trying inheritance
      const char* devname = expression->variablename->getDevice()->str();
      bool ambiguous;
      fun = 0;
      HMobjtab::iterator itobj;
      if ((itobj = ::urbiserver->objtab.find(devname)) !=
          ::urbiserver->objtab.end())
      {
        fun = itobj->second->
          searchFunction(expression->variablename->getMethod()->str(),
                         ambiguous);
        if (ambiguous)
        {
          snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                   "!!! Ambiguous multiple inheritance on function %s\n",
                   funname->str());
          connection->send(tmpbuffer, getTag().c_str());
          return ( status = UCOMPLETED );
        }
      }
    }
    else
      fun = hmf->second;

    if (fun)
    {
      if ( ( (expression->parameters) &&
	     (fun->nbparam()) &&
	     (expression->parameters->size() != fun->nbparam())) ||
	   ( (expression->parameters) && (!fun->nbparam())) ||
	   ( (!expression->parameters) && (fun->nbparam())) )
      {
	snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		 "!!! invalid number of arguments for %s (should be %d params)\n",
		 funname->str(), fun->nbparam());
	connection->send(tmpbuffer, getTag().c_str());

	return ( status = UCOMPLETED );
      }

      persistant = false;
      UVariableName* resultContainer = new UVariableName(
	  new UString("__UFnct"),
	  new UString("__result__"),
	  true,
	  (UNamedParameters*)0);

      UCommand_EXPR* cexp = new UCommand_EXPR(new UExpression(EXPR_VARIABLE,
	    resultContainer));

      cexp->setTag(this);
      morph = (UCommand*)
        new UCommand_TREE(UPIPE,
                          fun->cmdcopy(getTag()),
                          cexp);

      if (morph)
      {
        morph->morphed = true;
        morph->setTag(this);

        if (flags)
          morph->flags = flags->copy();

        sprintf(tmpbuffer, "__UFnct%d", unic());
        UString* fundevice = expression->variablename->getDevice();
        if (!fundevice)
        {
          connection->send("!!! Function name evaluation failed\n",
                           getTag().c_str());
          return (status = UCOMPLETED);
        }

        ((UCommand_TREE*)morph)->callid = new UCallid(tmpbuffer,
                                                      fundevice->str(),
						      (UCommand_TREE*)morph);
	resultContainer->nameUpdate(((UCommand_TREE*)morph)->callid->str(),
								"__result__");
	if (!((UCommand_TREE*)morph)->callid) return (status = UCOMPLETED);
	((UCommand_TREE*)morph)->connection = connection;

	UNamedParameters *pvalue = expression->parameters;
	UNamedParameters *pname  = fun->parameters;
	for (;
	     pvalue != 0;
	     pvalue = pvalue->next, pname = pname->next)
        {
	  UValue* valparam = pvalue->expression->eval(this, connection);
	  if (!valparam)
          {
	    connection->send("!!! EXPR evaluation failed\n", getTag().c_str());
	    return (status = UCOMPLETED);
	  }
	  snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		   "%s.%s", ((UCommand_TREE*)morph)->callid->str(), pname->name->str());

	  ((UCommand_TREE*)morph)->callid->store(
		   new UVariable(((UCommand_TREE*)morph)->callid->str(),
				 pname->name->str(),
				 valparam)
		   );
	}
      }

      return ( status = UMORPH );
    }
    else
      if ((connection->receiving) &&
	  (expression->variablename->id->equal("exec")))
	return ( status = URUNNING);

    ////// module-defined /////

    urbi::UTable::iterator hmfi = urbi::functionmap.find(funname->str());
    if (hmfi != urbi::functionmap.end())
    {
      for (std::list<urbi::UGenericCallback*>::iterator cbi =
           hmfi->second.begin();
           cbi != hmfi->second.end();
           cbi++)
      {
        if ( ( (expression->parameters) &&
               (expression->parameters->size() == (*cbi)->nbparam)) ||
             ( (!expression->parameters) && (!(*cbi)->nbparam)) )
        {
          // here you could spawn a thread... if only Aperios knew how to!
          urbi::UList tmparray;
          for (UNamedParameters *pvalue = expression->parameters;
               pvalue != 0;
               pvalue = pvalue->next)
          {
            UValue* valparam = pvalue->expression->eval(this, connection);
            if (!valparam)
            {
              connection->send("!!! EXPR evaluation failed\n",
                               getTag().c_str());
              return (status = UCOMPLETED);
            }
            // urbi::UValue do not see ::UValue,
            // so it must be valparam who does the job.
            urbi::UValue *tmpvalue = valparam->urbiValue();

            tmparray.array.push_back(tmpvalue);
          }

          UValue ret = (*cbi)->__evalcall(tmparray);

          if (ret.dataType != DATA_VOID)
          {
            connection->sendPrefix(getTag().c_str());
            ret.echo(connection);
          }
          if ((ret.dataType!=DATA_BINARY) && (ret.dataType!=DATA_VOID))
            connection->endline();
          return ( status = UCOMPLETED );
        }
      }

      connection->send("!!! Invalid function call\n", getTag().c_str());
      return ( status = UCOMPLETED );
    }
  }

  // Normal expression (no function)
  UValue* ret = expression->eval(this, connection);
  if (ret == 0)
  {
    connection->send("!!! EXPR evaluation failed\n", getTag().c_str());
    return (status = UCOMPLETED);
  }

  // Expression morphing (currently used for load only)
  if (morph)
  {
    delete ret;
    return ( status = UMORPH);
  }

  if (ret->dataType != DATA_VOID)
  {
    connection->sendPrefix(getTag().c_str());
    ret->echo(connection);
  }
  if ((ret->dataType!=DATA_BINARY) && (ret->dataType != DATA_VOID))
    connection->endline();
  delete ret;
  return (status = UCOMPLETED);
}

//! UCommand subclass hard copy function
UCommand*
UCommand_EXPR::copy()
{
  UCommand_EXPR *ret = new UCommand_EXPR(ucopy (expression));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("EXPR:\n");

  if (expression)
  {
    ::urbiserver->debug("%s  Expr:", tabb);
    expression->print(); ::urbiserver->debug("\n");
  };

  ::urbiserver->debug("%sEND EXPR ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_RETURN);
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
  delete expression;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_RETURN::execute(UConnection *connection)
{
  if (!connection->stack.empty())
  {
    connection->returnMode = true;
    if (expression)
    {
      UValue *value = expression->eval(this, connection);
      if (!value)
      {
        connection->send("!!! EXPR evaluation failed\n", getTag().c_str());
        return (status = UCOMPLETED);
      }

      new UVariable(connection->stack.front()->str(),
                    "__result__",
                    value);
    }
    else
      new UVariable(connection->stack.front()->str(),
                    "__result__", new UValue());
  }
  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_RETURN::copy()
{
  UCommand_RETURN *ret = new UCommand_RETURN(ucopy (expression));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("RETURN:\n");

  if (expression)
  {
    ::urbiserver->debug("%s  Expr:", tabb);
    expression->print(); ::urbiserver->debug("\n");
  };

  ::urbiserver->debug("%sEND RETURN ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_ECHO);
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
  delete expression;
  delete parameters;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_ECHO::execute(UConnection *connection)
{
  UValue* ret = expression->eval(this, connection);

  if (ret==0)
  {
    connection->send("!!! EXPR evaluation failed\n", getTag().c_str());

    return (status = UCOMPLETED);
  }

  UNamedParameters *param = parameters;
  while (param)
  {
    if (param->name->equal("connection"))
    {
      UValue *e1 = param->expression->eval(this, connection);
      if ((e1) && (e1->dataType == DATA_STRING))
	connectionTag = new UString(e1->str);

      delete e1;
    }
    param = param->next;
  }

  if (!connectionTag)
  {
    connection->send("*** ", getTag().c_str());
    ret->echo(connection, true);
    connection->endline();
  }
  else {

    bool ok = false;

    // Scan currently opened connections to locate the connection with the
    // appropriate tag (connectionTag)
    for (std::list<UConnection*>::iterator retr =
         connection->server->connectionList.begin();
	 retr != connection->server->connectionList.end();
	 retr++)
      if  ( ((*retr)->isActive()) &&
	    ( ((*retr)->connectionTag->equal(connectionTag)) ||
	      (connectionTag->equal("all")) ||
	      ( (!(*retr)->connectionTag->equal(connection->connectionTag)) &&
		(connectionTag->equal("other")) ) )
	    )
      {
	ok = true;
	(*retr)->send("*** ", getTag().c_str());
	ret->echo((*retr), true);
	(*retr)->endline();
      }

    if (!ok)
      {
	snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		 "!!! %s: no such connection\n", connectionTag->str());
	connection->send(tmpbuffer, getTag().c_str());
      }
  }

  delete ret;
  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_ECHO::copy()
{
  UCommand_ECHO *ret =
    new UCommand_ECHO(ucopy (expression),
		      ucopy (parameters),
		      ucopy (connectionTag));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("ECHO:\n");

  if (expression)
  {
    ::urbiserver->debug("%s  Expr:", tabb);
    expression->print(); ::urbiserver->debug("\n");
  };
  if (parameters)
  {
    ::urbiserver->debug("%s  Param:{", tabb);
    parameters->print(); ::urbiserver->debug("}\n");
  };

  ::urbiserver->debug("%sEND ECHO ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_NEW);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_NEW::UCommand_NEW(UVariableName* varname,
			   UString* obj,
			   UNamedParameters *parameters,
			   bool noinit) :
  UCommand(CMD_NEW)
{
  ADDOBJ(UCommand_NEW);
  this->varname     = varname;
  this->obj         = obj;
  this->parameters  = parameters;
  this->noinit      = noinit;
	this->id          = 0;
  remoteNew         = false;
}

//! UCommand subclass destructor.
UCommand_NEW::~UCommand_NEW()
{
  FREEOBJ(UCommand_NEW);
  delete varname;
  delete obj;
	delete id;
  // parameters are handled by the morphed init
}

//! UCommand subclass execution function
UCommandStatus
UCommand_NEW::execute(UConnection *connection)
{
  // Wait for remote new
  HMobjWaiting::iterator ow;

  if (remoteNew)
    {
      ow = ::urbiserver->objWaittab.find(id->str());
      if (ow!=::urbiserver->objWaittab.end()) return (status = URUNNING);
    }

  morph = 0;
  char tmpprefixnew[1024];

  if (!id)
  { // init id
    if (!varname->nostruct)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Object names cannot be nested in Kernel 1\n");
      connection->send(tmpbuffer, getTag().c_str());

      return ((status = UCOMPLETED));
    }
    UString* name = varname->buildFullname(this, connection, false);
    if (!name)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
               "!!! Invalid object name\n");
      connection->send(tmpbuffer, getTag().c_str());

      return ((status = UCOMPLETED));
    }
    id = new UString(name->str());
  }

  if (!id)
    return ((status = UCOMPLETED));
  if (!obj)
    return ((status = UCOMPLETED));

  if (id->equal(obj))
  {
    snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	 "!!! Object %s cannot new itself\n", obj->str());
    connection->send(tmpbuffer, getTag().c_str());

    return ((status = UCOMPLETED));
  }

  HMobjtab::iterator objit = ::urbiserver->objtab.find(obj->str());
  if (objit == ::urbiserver->objtab.end())
    {
      char* objname = (char*)obj->str();
      while ( ::urbiserver->objaliastab.find(objname) !=
	      ::urbiserver->objaliastab.end())
	objname = (char*)::urbiserver->objaliastab[objname]->str();

      objit = ::urbiserver->objtab.find(objname);
      if (objit == ::urbiserver->objtab.end())  {

	snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		 "!!! Unkown object %s\n", obj->str());
	connection->send(tmpbuffer, getTag().c_str());
	return ( status = UCOMPLETED );
      }
    }


  UObj* newobj;
  bool creation = false;

  // EXTERNAL
  if ((objit->second->binder)&& (!remoteNew))
    {
      snprintf(tmpprefixnew, 1024, "[4,\"%s\",\"%s\"]\n",
	       id->str(),
	       objit->second->device->str());

      int nb=0;
      for (std::list<UMonitor*>::iterator it2 =
	     objit->second->binder->monitors.begin();
	   it2 != objit->second->binder->monitors.end();
	   it2++)
	{
	  (*it2)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
	  (*it2)->c->send((const ubyte*)tmpprefixnew, strlen(tmpprefixnew));
	  nb++;
	}
      ow = ::urbiserver->objWaittab.find(id->str());
      if (ow!=::urbiserver->objWaittab.end())
	(*ow).second->nb += nb;
      else {
	UWaitCounter *wc = new UWaitCounter(id, nb);
	ASSERT(wc!=0) ::urbiserver->objWaittab[wc->id->str()] = wc;
      }
      // initiate remote new waiting
      remoteNew = true;
      return (status = URUNNING);
    }

  if (objit->second->internalBinder)
    objit->second->internalBinder->copy(std::string(id->str()));

  HMobjtab::iterator idit = ::urbiserver->objtab.find(id->str());
  if (idit == ::urbiserver->objtab.end())
  {
    newobj = new UObj(id);
    creation = true;
  }
  else
    newobj = idit->second;

  if (std::find(newobj->up.begin(), newobj->up.end(), objit->second) !=
      newobj->up.end())
  {
    snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	     "!!! %s has already inherited from %s\n", id->str(), obj->str());
    if (creation) delete newobj;
    connection->send(tmpbuffer, getTag().c_str());
    return ( status = UCOMPLETED );
  }

  newobj->up.push_back(objit->second);
  objit->second->down.push_back(newobj);

  // Here the policy of "a = new b" which does not call init could be
  // enforced with 'noinit'
  // Currently, after discussions, all call to new tries to start init.
  // if ((!parameters) && (noinit)) return (status = UCOMPLETED);

  // init constructor call
  //
  // For the moment, multiple inheritance with multiple constructors
  // will not be accepted. However, in principle there is no ambiguity since
  // we have a clear reference to the inherited object in this case. It will
  // be fixed later.

  int UU = unic();
  persistant = false;
  std::ostringstream oss;
  std::string tmpval("__UInitret.tmpval_");

  oss << "{ ";

  bool component = false;
  UFunction* initfun = 0;
  // wait for init created if external component
  if ((objit->second->binder) || (objit->second->internalBinder))
  {
    oss << "waituntil(isdef(" << id->str() << ".init)) | ";
    component = true;
  }
  else
  {
    // detects if init exists for the object or somewhere in the hierarchy
    bool ambiguous;
    initfun = newobj->searchFunction("init", ambiguous);
  }

  if (parameters || (initfun != 0) || component)
  {
    oss << tmpval << UU << "=" << id->str() << ".init(";

    for (UNamedParameters *pvalue = parameters;
         pvalue != 0;
         pvalue = pvalue->next)
    {
      UValue* valparam = pvalue->expression->eval(this, connection);
      if (!valparam)
      {
        connection->send("!!! EXPR evaluation failed\n", getTag().c_str());
        snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
                 "{delete %s}", id->str());

        morph = (UCommand*)
          new UCommand_EXPR
          (new UExpression
           (EXPR_FUNCTION,
            new UVariableName
            (new UString("global"),
             new UString("exec"),
             false,
             (UNamedParameters *)0),
            new UNamedParameters
            (new UExpression
             (EXPR_VALUE,
              new UString(tmpbuffer)
             )
            )
           )
          );
        return (status = UMORPH);
      }

      oss << valparam->echo();
      if (pvalue->next) oss << ",";
    }

    oss << ") | if (!isdef("
      << tmpval << UU << ") || ((" << tmpval << UU << "!=0) && (!isvoid("
      << tmpval << UU << ")))) { "
      << "echo \"Error: Constructor failed, objet deleted\";"
      << " delete "
      << id->str() << "} | if (isdef("
      << tmpval << UU << ")) delete " << tmpval << UU
      << "}";
  }
  else
    oss << "noop }";

  morph = (UCommand*)
    new UCommand_EXPR
    (
     new UExpression
     (
      EXPR_FUNCTION,
      new UVariableName
      (
       new UString("global"),
       new UString("exec"),
       false,
       (UNamedParameters *)0),
      new UNamedParameters
      (
       new UExpression
       (
        EXPR_VALUE,
        new UString(oss.str().c_str())
       )
      )
     )
    );

  return ( status = UMORPH );
}


//! UCommand subclass hard copy function
UCommand*
UCommand_NEW::copy()
{
  UCommand_NEW *ret = new UCommand_NEW(ucopy (varname),
				       ucopy (obj),
				       ucopy (parameters));

  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command
/*! This function is for debugging purpose only.
    It is not safe, efficient or crash proof. A better version will come later.
*/
void
UCommand_NEW::print(int l)
{
  char tabb[100];

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("NEW:\n");

  if (id)
  {
    ::urbiserver->debug("%s  Id:[%s]\n", tabb, id->str());
  }
  if (obj)
  {
    ::urbiserver->debug("%s  Obj:[%s]\n", tabb, obj->str());
  }
  if (parameters)
  {
    ::urbiserver->debug("%s  Param:{", tabb);
    parameters->print(); ::urbiserver->debug("}\n");
  };

  ::urbiserver->debug("%sEND NEW ------\n", tabb);
}


MEMORY_MANAGER_INIT(UCommand_ALIAS);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_ALIAS::UCommand_ALIAS(UVariableName* aliasname,
			       UVariableName* id,
			       bool eraseit) :
  UCommand(CMD_ALIAS)
{
  ADDOBJ(UCommand_ALIAS);
  this->id           = id;
  this->aliasname    = aliasname;
  this->eraseit      = eraseit;
}

//! UCommand subclass destructor.
UCommand_ALIAS::~UCommand_ALIAS()
{
  FREEOBJ(UCommand_ALIAS);
  delete id;
  delete aliasname;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_ALIAS::execute(UConnection *connection)
{
  //alias setting
  if (aliasname && id)
    {
      UString *id0 = aliasname->buildFullname(this, connection, false);
      UString *id1 = id->buildFullname(this, connection, false);
      if (id0 && id1)
	if (!connection->server->addAlias(id0->str(), id1->str()))
	  {
	    connection->send("!!! Circular alias detected, abort command.\n",
                             getTag().c_str());
	    return (status = UCOMPLETED);
	  }

      return ( status = UCOMPLETED );
    }

  // full alias query
  if (!aliasname && !id)
    {
      for ( HMaliastab::iterator retr =
	      connection->server->aliastab.begin();
	    retr != connection->server->aliastab.end();
	    retr++)
	{
	  snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		   "*** %25s -> %s\n",
		   (*retr).first, (*retr).second->str());
	  connection->send(tmpbuffer, getTag().c_str());
	}

      return ( status = UCOMPLETED );
    }

  // specific alias query
  if (aliasname && !id && (!eraseit))
    {
      UString *id0 = aliasname->buildFullname(this, connection, false);
      HMaliastab::iterator retr =
        connection->server->aliastab.find(id0->str());
      if (retr != connection->server->aliastab.end())
	{
	  snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		   "*** %25s -> %s\n",
		   (*retr).first, (*retr).second->str());
	  connection->send(tmpbuffer, getTag().c_str());
	}
      return ((status = UCOMPLETED));
    }

  // unalias query
  if (aliasname && !id && (eraseit))
    {
      UString *id0 = aliasname->buildFullname(this, connection, false);
      HMaliastab::iterator retr =
        connection->server->aliastab.find(id0->str());
      if (retr != connection->server->aliastab.end())
	connection->server->aliastab.erase(retr);

      return ((status = UCOMPLETED));
    }
  return ((status = UCOMPLETED));
}


//! UCommand subclass hard copy function
UCommand*
UCommand_ALIAS::copy()
{
  UCommand_ALIAS *ret = new UCommand_ALIAS(ucopy (aliasname),
					   ucopy (id),
					   eraseit);
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("ALIAS (%d) :\n", (int)eraseit);

  if (aliasname)
  {
    ::urbiserver->debug("  %s  Aliasname:", tabb);
    aliasname->print(); ::urbiserver->debug("\n");
  };
  if (id)
  {
    ::urbiserver->debug("  %s  id:", tabb);
    id->print(); ::urbiserver->debug("\n");
  };

  ::urbiserver->debug("%sEND ALIAS ------\n", tabb);
}


MEMORY_MANAGER_INIT(UCommand_GROUP);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_GROUP::UCommand_GROUP(UString* id,
			       UNamedParameters* parameters,
			       int grouptype) :
  UCommand(CMD_GROUP)
{
  ADDOBJ(UCommand_GROUP);
  this->id           = id;
  this->parameters   = parameters;
  this->grouptype    = grouptype;
}

//! UCommand subclass destructor.
UCommand_GROUP::~UCommand_GROUP()
{
  FREEOBJ(UCommand_GROUP);
  delete id;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_GROUP::execute(UConnection *connection)
{
  HMgrouptab::iterator hma;
  UGroup *g;

  if (parameters)
  {
    hma = ::urbiserver->grouptab.find(id->str());
    if (hma != ::urbiserver->grouptab.end())
      g = hma->second;
    else {
      g = new UGroup(id);
      ::urbiserver->grouptab[g->name->str()] = g;
    }
    if (grouptype==0)  g->members.clear();

    UNamedParameters* param = parameters;
    while (param)
    {
      if (grouptype == 2)
      {//del
	for (std::list<UString*>::iterator it = g->members.begin();
	    it != g->members.end();
	    )
	  if ((*it)->equal(param->name))
	    it =g->members.erase(it);
	  else
	    it++;
      }
      else {

	char* objname = (char*)param->name->str();
	while ( ::urbiserver->objaliastab.find(objname) !=
	    ::urbiserver->objaliastab.end())
	  objname = (char*)::urbiserver->objaliastab[objname]->str();

	g->members.push_back(new UString(objname));
      }

      param = param->next;
    }

    return (status = UCOMPLETED);
  }

  // full query
  if (!id)
  {
    for ( HMgrouptab::iterator retr =
	connection->server->grouptab.begin();
	retr != connection->server->grouptab.end();
	retr++)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	  "*** %s = {",
	  (*retr).first);
      for (std::list<UString*>::iterator it = (*retr).second->members.begin();
	   it !=  (*retr).second->members.end();
	   )
      {
	strncat(tmpbuffer, (*it)->str(), UCommand::MAXSIZE_TMPMESSAGE);
	it++;
	if (it != (*retr).second->members.end())
	  strncat(tmpbuffer, ",", UCommand::MAXSIZE_TMPMESSAGE);
      }
      strncat(tmpbuffer, "}\n", UCommand::MAXSIZE_TMPMESSAGE);
      connection->send(tmpbuffer, getTag().c_str());
    }
    return (status = UCOMPLETED);
  }

  // specific query
  HMgrouptab::iterator retr = connection->server->grouptab.find(id->str());
  if (retr !=  connection->server->grouptab.end())
  {
    UNamedParameters *ret = 0;

    std::list<UString*>::iterator it = (*retr).second->members.begin();
    if (it !=  (*retr).second->members.end())
    {
      ret = new UNamedParameters(new UExpression(EXPR_VALUE, (*it)->copy()),
	   (UNamedParameters*)0);
      it++;
    }

    while (it !=  (*retr).second->members.end())
    {
      ret = new UNamedParameters(new UExpression(EXPR_VALUE, (*it)->copy()),
	  ret);
      it++;
    }

    morph = new UCommand_EXPR(new UExpression(EXPR_LIST, ret));

    persistant = false;
    return (status = UMORPH);
  }

  return (status = UCOMPLETED);
}


//! UCommand subclass hard copy function
UCommand*
UCommand_GROUP::copy()
{
  UCommand_GROUP *ret = new UCommand_GROUP(ucopy (id),
					   ucopy (parameters),
					   grouptype);
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("GROUP :\n");
  if (id)  { ::urbiserver->debug("%s  Id:[%s]\n", tabb, id->str());}

  ::urbiserver->debug("%sEND GROUP ------\n", tabb);
}




MEMORY_MANAGER_INIT(UCommand_OPERATOR_ID);
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
  delete oper;
  delete id;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_OPERATOR_ID::execute(UConnection *connection)
{
  if (strcmp(oper->str(), "stop")==0)
  {
    if (status == URUNNING)
      return ( status = UCOMPLETED);
    connection->server->mark(id);
    connection->server->somethingToDelete = true;
    return ( status = URUNNING );
  }

  if (strcmp(oper->str(), "killall")==0)
  {
    bool ok = false;

    // Scan currently opened connections to locate the connection with the
    // appropriate tag (connectionTag)
    for (std::list<UConnection*>::iterator retr =
         connection->server->connectionList.begin();
	 retr != connection->server->connectionList.end();
	 retr++)
      if  ( ((*retr)->isActive()) &&
	    ((*retr)->connectionTag->equal(id)))
      {
	ok = true;
	(*retr)->killall = true;
      }

    if (!ok)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! %s: no such connection\n", id->str());
      connection->send(tmpbuffer, getTag().c_str());

      return (status = UCOMPLETED);
    }
    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "disconnect")==0)
  {
    bool ok = false;

    // Scan currently opened connections to locate the connection with the
    // appropriate tag (connectionTag)
    for (std::list<UConnection*>::iterator retr =
         connection->server->connectionList.begin();
	 retr != connection->server->connectionList.end();
	 retr++)
      if  ( ((*retr)->isActive()) &&
	    ((*retr)->connectionTag->equal(id)))
      {
	ok = true;
	(*retr)->disactivate();
	(*retr)->closeConnection();
      }

    if (!ok)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! %s: no such connection\n", id->str());
      connection->send(tmpbuffer, getTag().c_str());

      return (status = UCOMPLETED);
    }
    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "block")==0)
  {
    if (status == URUNNING)
      return ( status = UCOMPLETED);

    if (strcmp(id->str(), UNKNOWN_TAG)==0)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! cannot block 'notag'\n");
      connection->send(tmpbuffer, getTag().c_str());
    }
    else
      connection->server->block(id->str());

    return ( status = URUNNING );
  }

  if (strcmp(oper->str(), "unblock")==0)
  {
   connection->server->unblock(id->str());

   return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "freeze")==0)
  {
    if (status == URUNNING)
      return ( status = UCOMPLETED);

    if (strcmp(id->str(), UNKNOWN_TAG)==0)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! cannot freeze 'notag'\n");
      connection->send(tmpbuffer, getTag().c_str());
    }
    else
      connection->server->freeze(id->str());

    return ( status = URUNNING );
  }

  if (strcmp(oper->str(), "unfreeze")==0)
  {
    connection->server->unfreeze(id->str());
    return ( status = UCOMPLETED );
  }

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_OPERATOR_ID::copy()
{
  UCommand_OPERATOR_ID *ret = new UCommand_OPERATOR_ID(ucopy (oper),
						       ucopy (id));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());
  ::urbiserver->debug("OPERATOR_ID %s:\n", oper->str());

  if (id)  { ::urbiserver->debug("%s  Id:[%s]\n", tabb, id->str());}

  ::urbiserver->debug("%sEND OPERATOR_ID ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_DEVICE_CMD);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_DEVICE_CMD::UCommand_DEVICE_CMD( UVariableName* device,
					  ufloat *cmd) :
  UCommand(CMD_GENERIC)
{
  ADDOBJ(UCommand_DEVICE_CMD);
  this->variablename  = device;
  this->cmd           = *cmd;
}

//! UCommand subclass destructor.
UCommand_DEVICE_CMD::~UCommand_DEVICE_CMD()
{
  FREEOBJ(UCommand_DEVICE_CMD);
  delete variablename;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_DEVICE_CMD::execute(UConnection *connection)
{
  if (!variablename) return ( status = UCOMPLETED );
  variablename->buildFullname(this, connection);

  if (variablename->nostruct)
  {
    UVariableName* recreatevar =
      new UVariableName(variablename->getMethod()->copy(),
                        new UString("load"),
                        variablename->rooted,
                        (UNamedParameters*)0);
    delete variablename;
    variablename = recreatevar;
    variablename->buildFullname(this, connection);
  }

  // broadcasting
  if (scanGroups(&UCommand::refVarName, true)) return ( status = UMORPH );

  // Main execution
  if (cmd == -1)
    morph = new UCommand_ASSIGN_VALUE(
	variablename->copy(),
	new UExpression(EXPR_MINUS,
	  new UExpression(EXPR_VALUE, ufloat(1)),
	  new UExpression(EXPR_VARIABLE, variablename->copy())),
	(UNamedParameters*)0,
	false);
  else
    morph = new UCommand_ASSIGN_VALUE(
	variablename->copy(),
	new UExpression(EXPR_VALUE, ufloat(cmd)),
	(UNamedParameters*)0,
	false);

  persistant = false;
  return ( status = UMORPH );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_DEVICE_CMD::copy()
{
  UCommand_DEVICE_CMD *ret =
    new UCommand_DEVICE_CMD(ucopy (variablename),
			    new ufloat(cmd));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("DEVICE_CMD %s:\n", variablename->device->str());

  if (cmd)  { ::urbiserver->debug("%s  Cmd:[%f]\n", tabb, cmd);}

  ::urbiserver->debug("%sEND DEVICE_CMD ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_OPERATOR_VAR);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_OPERATOR_VAR::UCommand_OPERATOR_VAR(UString* oper,
					     UVariableName* variablename)
  : UCommand(CMD_GENERIC)
{
  ADDOBJ(UCommand_OPERATOR_VAR);
  this->oper         = oper;
  this->variablename = variablename;
}

//! UCommand subclass destructor.
UCommand_OPERATOR_VAR::~UCommand_OPERATOR_VAR()
{
  FREEOBJ(UCommand_OPERATOR_VAR);
  delete oper;
  delete variablename;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_OPERATOR_VAR::execute(UConnection *connection)
{
  UString *fullname = variablename->buildFullname(this, connection);
  if (!fullname) return ( status = UCOMPLETED );

  if ( (strcmp(oper->str(), "undef")==0) ||
       (strcmp(oper->str(), "delete")==0) )
  {
    if (status != URUNNING)
    {
      variable = 0;
      fun = variablename->getFunction(this, connection);
      if (!fun)
      {
	variable = variablename->getVariable(this, connection);

	if ((!variable) && (variablename->nostruct))
        {
	  UString* objname = variablename->getMethod();
	  if (::urbiserver->variabletab.find(objname->str()) !=
	      ::urbiserver->variabletab.end())
	    variable = ::urbiserver->variabletab[objname->str()];
	}
      }

      if ((!fun) && (!variable))
      {
	snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		 "!!! identifier %s does not exist\n", fullname->str());
	connection->send(tmpbuffer, getTag().c_str());
	return ( status = UCOMPLETED );
      }
    }

    if (variable)
    {// undef variable
      if (variable->toDelete)
      {
	delete variable;
	return ( status = UCOMPLETED );
      }

      // test if variable is an object with subclasses (and reject if yes)
      if ((variable->value) &&
	  (variable->value->dataType == DATA_OBJ) &&
	  (variable->value->str))
      {
	HMobjtab::iterator idit =
          ::urbiserver->objtab.find(variable->value->str->str());
	if ( (idit != ::urbiserver->objtab.end()) &&
	     (!idit->second->down.empty()) )
        {
	  snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	      "!!! This object has subclasses. Delete subclasses first.\n");
	  connection->send(tmpbuffer, getTag().c_str());
	  return ( status = UCOMPLETED );
	}
      }

      // variable is not an object or it does not have subclasses
      if ((variable->nbAssigns == 0) && (variable->uservar))
      {
	variable->toDelete = true;
	return ( status = URUNNING );
      }
      else
      {
	snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		 "!!! variable %s already in use or is a system var. Cannot delete.\n",
		 fullname->str());
	connection->send(tmpbuffer, getTag().c_str());
	return ( status = UCOMPLETED );
      }
    }

    if (fun)
    { //undef function
      connection->server->functiontab.erase(
	     connection->server->functiontab.find(fullname->str()));
      connection->server->functiondeftab.erase(
	     connection->server->functiondeftab.find(fullname->str()));

      delete fun;
      return ( status = UCOMPLETED );
    }

    return ( status = UCOMPLETED );
  }

 //FIXME
 /*
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

      connection->send(tmpbuffer,getTag().c_str());
      return ( status = UCOMPLETED );
    }

    if ((dev) && (!variable))
      variable = dev->device_val;

    if (dev) {
      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
	       "*** device description: %s\n",
	       dev->detail->str());
      connection->send(tmpbuffer,getTag().c_str());

      snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
	       "*** device name: %s\n",
	       dev->device->str());
      connection->send(tmpbuffer,getTag().c_str());
    }
std::ostringstream tstr;

    if (variable) {
      switch (variable->value->dataType) {

case DATA_NUM: tstr << "*** current value: "
	       << variable->value->val << std::endl;
		     break;

case DATA_STRING: tstr << "*** current value: \""
		  << variable->value->str->str() <<"\"\n";
		     break;

case DATA_BINARY: tstr << "*** current value: binary\n";
		     break;
      }
connection->send(tstr.str().c_str(),getTag().c_str());
tstr.str("");
    }

    if (dev) {
tstr << "*** current device load: " << dev->device_load->value->val<<'\n';
connection->send(tstr.str().c_str(),getTag().c_str());
tstr.str("");
    }

    if (variable) {
      if (variable->rangemin != -UINFINITY)
tstr << "*** rangemin: " << variable->rangemin << '\n';
      else
tstr << "*** rangemin: -INF\n";
connection->send(tstr.str().c_str(),getTag().c_str());
tstr.str("");

      if (variable->rangemax != UINFINITY)
tstr << "*** rangemax: " << variable->rangemax << '\n';
      else
tstr << "*** rangemax: INF\n";
connection->send(tstr.str().c_str(),getTag().c_str());
tstr.str("");

      if (variable->speedmin != -UINFINITY)
tstr << "*** speedmin: " << variable->rangemin << '\n';
      else
tstr << "*** speedmin: -INF\n";
connection->send(tstr.str().c_str(),getTag().c_str());
tstr.str("");

      if (variable->speedmax != UINFINITY)
tstr << "*** speedmax: " << variable->rangemax << '\n';
      else
tstr << "*** speedmax: INF\n";
connection->send(tstr.str().c_str(),getTag().c_str());
tstr.str("");


      if (variable->unit)
	snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
		 "*** unit: %s\n",
		 variable->unit->str());
      else
	snprintf(tmpbuffer,UCommand::MAXSIZE_TMPMESSAGE,
		 "*** unit: unspecified\n");
	connection->send(tmpbuffer,getTag().c_str());

    }

    return (status = UCOMPLETED);
  }*/

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_OPERATOR_VAR::copy()
{
  UCommand_OPERATOR_VAR *ret =
    new UCommand_OPERATOR_VAR(ucopy (oper),
			      ucopy (variablename));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());
  ::urbiserver->debug("OPERATOR_VAR %s:\n", oper->str());
  if (variablename)
  {
    ::urbiserver->debug("  %s  Variablename:", tabb);
    variablename->print(); ::urbiserver->debug("\n");
  };

  ::urbiserver->debug("%sEND OPERATOR_VAR ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_BINDER);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_BINDER::UCommand_BINDER(UVariableName* objname,
				 UString* binder,
				 int type,
				 UVariableName* variablename,
				 int nbparam) :
  UCommand(CMD_GENERIC)
{
  ADDOBJ(UCommand_BINDER);
  this->objname      = objname;
  this->binder       = binder;
  this->variablename = variablename;
  this->type	     = type;
  this->nbparam	     = nbparam;
}

//! UCommand subclass destructor.
UCommand_BINDER::~UCommand_BINDER()
{
  FREEOBJ(UCommand_BINDER);
  delete binder;
  delete variablename;
  delete objname;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_BINDER::execute(UConnection *connection)
{
  UObj* uobj;
  UString *fullname = variablename->buildFullname(this, connection);
  if (!fullname) return ( status = UCOMPLETED );

  UString *fullobjname = 0;
  if (objname)
  {
    fullobjname = objname->id;
    if (!fullobjname) return ( status = UCOMPLETED );
  }

  if (type != 3) // not object binder
    ::urbiserver->debug("BINDING: %s type(%d) %s[%d] from %s\n",
	binder->str(), type, fullname->str(), nbparam, fullobjname->str());
  else
    ::urbiserver->debug("BINDING: %s type(%d) %s\n",
	binder->str(), type, variablename->id->str());

  UBindMode mode = UEXTERNAL;

  UString *key = new UString(fullname);

  switch (type)
  {
    case UBIND_VAR:
      {
        HMvariabletab::iterator it =
          ::urbiserver->variabletab.find(key->str());
        if (it == ::urbiserver->variabletab.end())
        {
          UVariable *variable = new UVariable(key->str(), new UValue());
          variable->binder = new UBinder(fullobjname, fullname,
                                         mode,
                                         (UBindType)type, nbparam, connection);
        }
        else
        {
          if (it->second->binder)
            it->second->binder->addMonitor(fullobjname, connection);
          else
            it->second->binder = new UBinder(fullobjname, fullname,
                                             mode,
                                             (UBindType)type,
                                             nbparam,
                                             connection);
        }
      }
      break;

    case UBIND_FUNCTION:
      if ( ::urbiserver->functionbindertab.find(key->str())
           == ::urbiserver->functionbindertab.end())
	::urbiserver->functionbindertab[key->str()] =
          new UBinder(fullobjname, fullname,
	    mode, (UBindType)type, nbparam, connection);
      else
	::urbiserver->functionbindertab[key->str()]->
          addMonitor(fullobjname, connection);
      break;

    case UBIND_EVENT:
      if ( ::urbiserver->eventbindertab.find(key->str())
           == ::urbiserver->eventbindertab.end())
	::urbiserver->eventbindertab[key->str()] = new UBinder(fullobjname, fullname,
	    mode, (UBindType)type, nbparam, connection);
      else
	::urbiserver->eventbindertab[key->str()]->addMonitor(fullobjname, connection);
      break;

    case UBIND_OBJECT:
      if (::urbiserver->objtab.find(variablename->id->str()) !=
	  ::urbiserver->objtab.end())
	uobj = ::urbiserver->objtab[variablename->id->str()];
      else
	uobj = new UObj(variablename->id);
      if (uobj->binder)
	uobj->binder->addMonitor(variablename->id, connection);
      else
	uobj->binder = new UBinder(uobj->device,
                                   uobj->device,
                                   mode,
                                   (UBindType)type,
                                   0,
                                   connection);
      break;
  }

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_BINDER::copy()
{
  UCommand_BINDER *ret = new UCommand_BINDER(ucopy (objname),
					     ucopy (binder),
                                             type,
					     ucopy (variablename),
					     nbparam);

  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command
/*! This function is for debugging purpose only.
    It is not safe, efficient or crash proof. A better version will come later.
*/
void
UCommand_BINDER::print(int l)
{
  char tabb[100];

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("BINDER %s type:%d nbparam:%d:\n",
                      binder->str(), type, nbparam);
  if (objname)
  {
    ::urbiserver->debug("  %s  objname:", tabb);
    objname->print(); ::urbiserver->debug("\n");
  };
  if (variablename)
  {
    ::urbiserver->debug("  %s  Variablename:", tabb);
    variablename->print(); ::urbiserver->debug("\n");
  };

  ::urbiserver->debug("%sEND BINDER ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_OPERATOR);
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
  delete oper;
}

//#define ENABLE_BENCH
#ifdef ENABLE_BENCH
#include "testspeed.h"
#endif
//! UCommand subclass execution function
UCommandStatus UCommand_OPERATOR::execute(UConnection *connection)
{
std::ostringstream tstr;
  if (strcmp(oper->str(), "ping")==0)
  {
#ifdef ENABLE_BENCH
   dotest(connection->server);
#endif
   tstr <<  "*** pong time="<<std::left <<connection->server->getTime()<<'\n';

    connection->send(tstr.str().c_str(), getTag().c_str());
    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "commands")==0)
  {
    if (connection->activeCommand)
      connection->activeCommand->print(0);
    ::urbiserver->debug("*** LOCAL TREE ***\n");
    if (connection->server->parser.commandTree)
      connection->server->parser.commandTree->print(0);
    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "strict")==0)
  {
    connection->server->defcheck = true;
    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "unstrict")==0)
  {
    connection->server->defcheck = false;
    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "motoron")==0)
  {
    if (connection->receiving) return (status = URUNNING);

    snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
     "!!! This command is no longer valid. Please use \"motor on\" instead\n");
    connection->send(tmpbuffer, getTag().c_str());

    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "motoroff")==0)
  {
    snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
    "!!! This command is no longer valid. Please use \"motor off\" instead\n");
    connection->send(tmpbuffer, getTag().c_str());

    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "stopall")==0)
  {
    snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
     "*** All commands cleared\n");
    connection->send(tmpbuffer, getTag().c_str());
    connection->server->stopall = true;
    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "undefall")==0)
  {
    snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
     "*** All variables and functions cleared\n");
    connection->send(tmpbuffer, getTag().c_str());

    for ( HMvariabletab::iterator retr =
	    connection->server->variabletab.begin();
	  retr != connection->server->variabletab.end();)
      delete (*retr).second;

    connection->server->variabletab.clear();
    connection->server->functiontab.clear();
    connection->server->emittab.clear();

    return ( status = UCOMPLETED );
  }
  if (strcmp(oper->str(), "reset")==0)
  {
    snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
             "*** Reset in progress\n");
    connection->send(tmpbuffer, getTag().c_str());
    ::urbiserver->reseting = true;

    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "devices")==0)
  {
    snprintf(tmpbuffer,
             UCommand::MAXSIZE_TMPMESSAGE,
             "*** devices is deprecated. Use 'group objects' instead.\n");
    connection->send(tmpbuffer, getTag().c_str());

    return ( status = UCOMPLETED );
  }

  UString* fullname;

  if (strcmp(oper->str(), "vars")==0)
  {
    for ( HMvariabletab::iterator retr =
          connection->server->variabletab.begin();
          retr != connection->server->variabletab.end();
          retr++)
    {
      fullname = (*retr).second->varname;
      if (fullname)
      {
        std::ostringstream tstr;
        switch ((*retr).second->value->dataType)
        {
          case DATA_NUM:
            tstr << "*** "
              << fullname->str() << " = "<< (*retr).second->value->val
              <<'\n';
            break;

          case DATA_STRING:
            tstr << "*** "
              << fullname->str() << " = " << (*retr).second->value->str->str()
              <<'\n';
            break;

          case DATA_BINARY:
            if ((*retr).second->value->refBinary)
              tstr <<"*** "<<fullname->str()<<" = BIN "<<
                (*retr).second->value->refBinary->ref()->bufferSize<<'\n';
            else
              tstr << "*** "<<fullname->str()<<" = BIN 0 null\n";
            break;

          default:
            tstr << "*** "<<fullname->str()<<" = UNKNOWN TYPE\n";
        } // end switch

        connection->send(tstr.str().c_str(), getTag().c_str());
      }
    }

    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "events")==0)
  {
    for ( HMemittab::iterator retr =
          connection->server->emittab.begin();
          retr != connection->server->emittab.end();
          retr++)
    {
      std::ostringstream tstr;
      tstr << "*** " << (*retr).second->unforgedName->str() << "["
        <<  (*retr).second->nbarg () << "]\n";

      connection->send(tstr.str().c_str(), getTag().c_str());
    }

    return ( status = UCOMPLETED );
  }


  if (strcmp(oper->str(), "uservars")==0)
  {
    for ( HMvariabletab::iterator retr =
	    connection->server->variabletab.begin();
	  retr != connection->server->variabletab.end();
	  retr++)
    {
      fullname = (*retr).second->varname;
      if ((*retr).second->uservar)
      {
	std::ostringstream tstr;
	switch ((*retr).second->value->dataType)
        {
	  case DATA_NUM:
	    tstr << "*** " << fullname->str() << " = "<< (*retr).second->value->val
	      <<'\n';
	    break;

	  case DATA_STRING:
	    tstr << "*** "
              << fullname->str() << " = " << (*retr).second->value->str->str()
	      <<'\n';
	    break;

	  case DATA_BINARY:
	    if ((*retr).second->value->refBinary)
	      tstr <<"*** "<<fullname->str()<<" = BIN "<<
		(*retr).second->value->refBinary->ref()->bufferSize<<'\n';
	    else
	      tstr << "*** "<<fullname->str()<<" = BIN 0 null\n";
	    break;

	   case DATA_OBJ:
	      tstr << "*** "
                << fullname->str()
                << " = OBJ " << (*retr).second->value->str->str()
	      <<'\n';
	    break;

	  default:
	    tstr << "*** "<<fullname->str()<<" = UNKNOWN TYPE\n";
	} // end switch

	connection->send(tstr.str().c_str(), getTag().c_str());
      }
    }

    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "debugon")==0)
  {
    connection->server->debugOutput = true;
    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "connections")==0)
  {

    for (std::list<UConnection*>::iterator retr =
         ::urbiserver->connectionList.begin();
	 retr != ::urbiserver->connectionList.end();
	 retr++)
      if ((*retr)->isActive())  {
	snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		 "*** %s (%d.%d.%d.%d)\n",
		 (*retr)->connectionTag->str(),
		 (int) (((*retr)->clientIP>>24) % 256),
		 (int) (((*retr)->clientIP>>16) % 256),
		 (int) (((*retr)->clientIP>>8) % 256),
		 (int) ( (*retr)->clientIP     % 256)
		 );

	connection->send(tmpbuffer, getTag().c_str());
      }


    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "debugoff")==0)
  {
    connection->server->debugOutput = false;
    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "quit")==0)
  {
    connection->closeConnection();
    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "reboot")==0)
  {
    connection->server->reboot();
    return ( status = UCOMPLETED );
  }

  if (strcmp(oper->str(), "shutdown")==0)
  {
    connection->server->shutdown();
    return ( status = UCOMPLETED );
  }

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_OPERATOR::copy()
{
  UCommand_OPERATOR *ret = new UCommand_OPERATOR(ucopy (oper));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("OPERATOR %s:\n", oper->str());
  ::urbiserver->debug("%sEND OPERATOR ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_WAIT);
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
  delete expression;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_WAIT::execute(UConnection *connection)
{

  if (status == UONQUEUE)
  {
    UValue *nb = expression->eval(this, connection);
    if (nb == 0)
      return ( status = UCOMPLETED );

    if (nb->dataType != DATA_NUM)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! Invalid type. NUM expected.\n");
      connection->send(tmpbuffer, getTag().c_str());
      return ( status = UCOMPLETED );
    }
    if (nb->val == 0) return (status = UCOMPLETED);

    endtime = connection->server->lastTime() + nb->val;

    delete nb;
    status = URUNNING;
  }
  else
    if (connection->server->lastTime() >= endtime)
      status = UCOMPLETED;

  return (status);
}

//! UCommand subclass hard copy function
UCommand*
UCommand_WAIT::copy()
{
  UCommand_WAIT *ret = new UCommand_WAIT(ucopy (expression));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("WAIT:\n");

  if (expression)
  {
    ::urbiserver->debug("%s  Expr:", tabb);
    expression->print(); ::urbiserver->debug("\n");
  };

  ::urbiserver->debug("%sEND WAIT ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_EMIT);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_EMIT::UCommand_EMIT(UVariableName *eventname,
			     UNamedParameters *parameters,
			     UExpression *duration) :
  UCommand(CMD_EMIT)
{
  ADDOBJ(UCommand_EMIT);
  this->eventname    = eventname;
  this->parameters   = parameters;
  this->duration     = duration;
  eventnamestr       = 0;
  firsttime = true;
  event = 0;
  eh = 0;
}

//! UCommand subclass destructor.
UCommand_EMIT::~UCommand_EMIT()
{
  FREEOBJ(UCommand_EMIT);

  delete eventname;
  delete parameters;
  delete duration;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_EMIT::execute(UConnection *connection)
{
  if (connection->receiving)
    return ( status = UONQUEUE);

  ufloat thetime = connection->server->lastTime();

  if (firsttime)
    {
      if (duration)
	{
	  UValue *dur = duration->eval(this, connection);
	  if (!dur)
	    {
	      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		       "!!! invalid event duration for event %s\n",
		       eventnamestr);
	      connection->send(tmpbuffer, getTag().c_str());
	      return ((status = UCOMPLETED));
	    }
	  targetTime = thetime + dur->val;
	  delete dur;
	}
      else
	{
	  targetTime = thetime;

	  UNamedParameters *pevent = parameters;
	  while (pevent)
	    {
	      UValue *e1 = pevent->expression->eval(this, connection);
	      if (e1)
		{
		  delete pevent->expression;
		  pevent->expression = new UExpression(EXPR_VALUE, e1->copy());
		  delete e1;
		}

	      pevent = pevent->next;
	    }
	}

      UString* ens = eventname->buildFullname(this, connection);

      // register event
      int nbargs = 0;
      if (parameters) nbargs = parameters->size ();

      eh = kernel::findEventHandler (ens, nbargs);
      if (!eh)
	{
	  if (::urbiserver->defcheck)
	    {
	      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		       "!!! undefined event %s with %d param(s)\n",
		       ens->str (),
		       nbargs);
	      connection->send(tmpbuffer, getTag().c_str());
	      return ((status = UCOMPLETED));
	    }
	  eh = new UEventHandler (ens, nbargs);
	}
      ASSERT (eh);
      event = eh->addEvent (parameters, this, connection);
      eventnamestr = ens->str ();

      ////// EXTERNAL /////

      HMbindertab::iterator it =
        ::urbiserver->eventbindertab.find(eventnamestr);

      if ((it != ::urbiserver->eventbindertab.end()) &&
          (
           ( (parameters) && (it->second->nbparam == parameters->size()))
           ||
           ((!parameters) && (it->second->nbparam==0))) &&
          (!it->second->monitors.empty()))
      {
        char tmpprefix[1024];
        snprintf(tmpprefix, 1024, "[2,\"%s__%d\"",
                 eventnamestr, it->second->nbparam);

        for (std::list<UMonitor*>::iterator it2 = it->second->monitors.begin();
             it2 != it->second->monitors.end();
             it2++)
        {
          (*it2)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
          (*it2)->c->send((const ubyte*)tmpprefix, strlen(tmpprefix));
          for (UNamedParameters *pvalue = parameters;
               pvalue != 0;
               pvalue = pvalue->next)
          {
            (*it2)->c->send((const ubyte*)",", 1);
            UValue* valparam = pvalue->expression->eval(this, connection);
            valparam->echo((*it2)->c);
          }
          (*it2)->c->send((const ubyte*)"]\n", 2);
        }
      }


      ////// INTERNAL /////

      urbi::UTable::iterator hmfi = urbi::eventmap.find(eventnamestr);
      if (hmfi != urbi::eventmap.end())
      {
        for (std::list<urbi::UGenericCallback*>::iterator cbi =
             hmfi->second.begin();
             cbi != hmfi->second.end();
             cbi++)
        {
          if ( ( (parameters) &&
                 (parameters->size() == (*cbi)->nbparam)) ||
               ( (!parameters) && (!(*cbi)->nbparam)) )
          {
            urbi::UList tmparray;
            for (UNamedParameters *pvalue = parameters;
                 pvalue != 0;
                 pvalue = pvalue->next)
            {
              UValue* valparam = pvalue->expression->eval(this, connection);
              if (!valparam)
              {
                connection->send("!!! EXPR evaluation failed\n",
                                 getTag().c_str());
                return (status = UCOMPLETED);
              }
              // urbi::UValue do not see ::UValue, so it must be
              // valparam who does the job.
              urbi::UValue *tmpvalue = valparam->urbiValue();
              tmparray.array.push_back(tmpvalue);
            }

            (*cbi)->__evalcall(tmparray);
          }
        }
      }
    }

  if ((thetime > targetTime) && (!firsttime))
  {
    // unregister event
    ASSERT (event && eh) // free the event
      eh->removeEvent (event);

    ////// EXTERNAL /////

    HMbindertab::iterator it =
      ::urbiserver->eventbindertab.find(eventnamestr);
    if ((it != ::urbiserver->eventbindertab.end()) &&
        (parameters) &&
        (it->second->nbparam == parameters->size()) &&
        (!it->second->monitors.empty()))
    {
      char tmpprefix[1024];
      snprintf(tmpprefix, 1024, "[3,\"%s__%d\"]\n",
               eventnamestr, it->second->nbparam);

      for (std::list<UMonitor*>::iterator it2 = it->second->monitors.begin();
           it2 != it->second->monitors.end();
           it2++)
      {
        (*it2)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
        (*it2)->c->send((const ubyte*)tmpprefix, strlen(tmpprefix));
      }
    }

    ////// INTERNAL /////

    urbi::UTable::iterator hmfi = urbi::eventendmap.find(eventnamestr);
    if (hmfi != urbi::eventendmap.end())
    {
      for (std::list<urbi::UGenericCallback*>::iterator cbi =
           hmfi->second.begin();
           cbi != hmfi->second.end();
           cbi++)
      {
        urbi::UList tmparray;
        (*cbi)->__evalcall(tmparray);
      }
    }

    return ((status = UCOMPLETED));
  }

  firsttime = false;
  return (status = UBACKGROUND);
}

//! UCommand subclass hard copy function
UCommand*
UCommand_EMIT::copy()
{
  UCommand_EMIT *ret =
    new UCommand_EMIT(ucopy (eventname), ucopy (parameters));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("EMIT:\n");

  if (eventname)
  {
    ::urbiserver->debug("%s  Event:", tabb);
    eventname->print(); ::urbiserver->debug("\n");
  };

  ::urbiserver->debug("%sEND EMIT ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_WAIT_TEST);
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
  delete test;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_WAIT_TEST::execute(UConnection *connection)
{
  if (!test) return (status = UCOMPLETED);
  UTestResult testres = booleval(test->eval(this, connection));

  if (testres == UTESTFAIL)
    return ( status = URUNNING );

  if (testres == UTRUE)
  {
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
  UCommand_WAIT_TEST *ret = new UCommand_WAIT_TEST(ucopy (test));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("WAIT_TEST:\n");

  if (test)
  {
    ::urbiserver->debug("%s  Test:", tabb);
    test->print(); ::urbiserver->debug("\n");
  };

  ::urbiserver->debug("%sEND WAIT_TEST ------\n", tabb);
}


MEMORY_MANAGER_INIT(UCommand_INCDECREMENT);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_INCDECREMENT::UCommand_INCDECREMENT(UCommandType type,
                                             UVariableName *variablename) :
  UCommand(type)
{
  ADDOBJ(UCommand_INCDECREMENT);
  this->variablename  = variablename;
}

//! UCommand subclass destructor.
UCommand_INCDECREMENT::~UCommand_INCDECREMENT()
{
  FREEOBJ(UCommand_INCDECREMENT);
  delete variablename;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_INCDECREMENT::execute(UConnection *connection)
{
  variablename->getVariable(this, connection);
  if (!variablename->getFullname())
    return ((status = UCOMPLETED));
  variablename->getMethod();
  variablename->getDevice();

  // Broadcasting
  if (scanGroups(&UCommand::refVarName, true))
    return ( status = UMORPH );

  // Main execution
  if (type == CMD_INCREMENT)
  {
    morph = (UCommand*)
      new UCommand_ASSIGN_VALUE(
	      variablename->copy(),
	      new UExpression(
			 EXPR_PLUS,
			 new UExpression(EXPR_VARIABLE, variablename->copy()),
			 new UExpression(EXPR_VALUE, ufloat(1))), 0);

    persistant = false;
    return ( status = UMORPH );
  }

  if (type == CMD_DECREMENT)
  {
    morph = (UCommand*)
      new UCommand_ASSIGN_VALUE(
	      variablename->copy(),
	      new UExpression(
			 EXPR_MINUS,
			 new UExpression(EXPR_VARIABLE, variablename->copy()),
			 new UExpression(EXPR_VALUE, ufloat(1))), 0);

    persistant = false;
    return ( status = UMORPH );
  }

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_INCDECREMENT::copy()
{
  UCommand_INCDECREMENT *ret =
    new UCommand_INCDECREMENT(type, ucopy (variablename));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("INCDECREMENT:");
  if (type == CMD_INCREMENT) ::urbiserver->debug("INC\n");
  else
    if (type == CMD_DECREMENT) ::urbiserver->debug("DEC\n");
    else
      ::urbiserver->debug("UNKNOWN TYPE\n");

  if (variablename)
  {
    ::urbiserver->debug("%s  Variable:", tabb);
    variablename->print(); ::urbiserver->debug("\n");
  };
  ::urbiserver->debug("%sEND INCDECREMENT ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_DEF);
// **************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_DEF::UCommand_DEF(UDefType deftype,
			   UVariableName *variablename,
			   UNamedParameters *parameters,
			   UCommand* command) :
  UCommand(CMD_DEF)
{
  ADDOBJ(UCommand_DEF);
  this->deftype      = deftype;
  this->variablename = variablename;
  this->parameters   = parameters;
  this->command      = command;
  this->device       = 0;
  this->variablelist = 0;
}

//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_DEF::UCommand_DEF(UDefType deftype,
			   UString *device,
			   UNamedParameters *parameters) :
  UCommand(CMD_DEF)
{
  ADDOBJ(UCommand_DEF);
  this->deftype      = deftype;
  this->variablename = 0;
  this->parameters   = parameters;
  this->command      = 0;
  this->device       = device;
  this->variablelist = 0;
}

//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_DEF::UCommand_DEF(UDefType deftype,
			   UVariableList *variablelist) :
  UCommand(CMD_DEF)
{
  ADDOBJ(UCommand_DEF);
  this->deftype      = deftype;
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
  delete variablename;
  delete variablelist;
  delete device;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_DEF::execute(UConnection *connection)
{
  // Def list query
  if ( (!variablename) &&
       (!command) &&
       (!parameters) &&
       (!variablelist))
  {
    for ( HMfunctiontab::iterator retr =
	    connection->server->functiontab.begin();
	  retr != connection->server->functiontab.end();
	  retr++)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "*** %s : %d param(s)\n",
	       (*retr).second->name()->str(),
	       (*retr).second->nbparam());

      connection->send(tmpbuffer, getTag().c_str());
    }
    return (status = UCOMPLETED);
  }

  // Function definition
  if (deftype == UDEF_FUNCTION && variablename && command)
    {
      UString* funname = variablename->buildFullname(this, connection);
      if (!funname) return (status = UCOMPLETED);

      if ((variablename->nostruct) &&
	  (::urbiserver->grouptab.find(variablename->getMethod()->str()) !=
	   ::urbiserver->grouptab.end()))
      {
	snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		 "!!! function name conflicts with group %s \n",
                 variablename->getMethod()->str());
	connection->send(tmpbuffer, getTag().c_str());

	return ( status = UCOMPLETED );
      }

      if (connection->server->functiontab.find(funname->str()) !=
	  connection->server->functiontab.end())
      {
	snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
		 "!!! function %s already exists\n", funname->str());
	connection->send(tmpbuffer, getTag().c_str());

	return ( status = UCOMPLETED );
      }

      UFunction *fun = new UFunction(new UString(funname),
				     parameters,
				     command);
      if (fun)
	connection->server->functiondeftab[fun->name()->str()] = fun;

      if ((fun) && (command))
	connection->server->functiontab[fun->name()->str()] = fun;

      return ( status = UCOMPLETED );
    }

  // Event definition
  if ((deftype == UDEF_EVENT) && (variablename))
  {
    UString* eventname = variablename->buildFullname(this, connection);
    if (!eventname) return (status = UCOMPLETED);
    int eventnbarg = 0;
    if (parameters) eventnbarg = parameters->size();

    UEventHandler* eh = kernel::findEventHandler(eventname, eventnbarg);

    if (!eh)
      eh = new UEventHandler(eventname, eventnbarg);

    return ( status = UCOMPLETED );
  }


  // Single Variable definition
  if ((variablename)
      && (!command)
      && (!parameters)
      && (deftype != UDEF_FUNCTION))
  {
    UVariable* variable = variablename->getVariable(this, connection);
    if (!variablename->getFullname()) return ( status = UCOMPLETED );
    if (variable) // the variable is already defined
      return ( status = UCOMPLETED );

    // Variable definition

    variable = new UVariable(variablename->getFullname()->str(), new UValue());
    connection->localVariableCheck(variable);

    return ( status = UCOMPLETED );
  }

  // Device variable set definition
  if ((device) && (!command) && (parameters))
  {
    UNamedParameters * param = parameters;
    UCommand_DEF *cdef = new UCommand_DEF (UDEF_VAR,
                                           new UVariableName(device->copy(),
                                                             param->name,
                                                             true,
                                                             0),
					   (UNamedParameters*) 0,
					   (UCommand*) 0);
    cdef->setTag(this);
    morph = cdef;
    param = param->next;

    while (param)
    {
      if (param->name)
      {
	cdef = new UCommand_DEF (UDEF_VAR,
                                 new UVariableName(device->copy(),
                                                   param->name,
                                                   true,
                                                   0),
				 (UNamedParameters*) 0,
				 (UCommand*) 0);
	cdef->setTag(this);
	morph = (UCommand*) new UCommand_TREE(UAND, cdef, morph);
      }
      param = param->next;
    }
    persistant = false;
    return ( status = UMORPH );
  }

  // Multi Variable definition
  if (variablelist)
  {
    UVariableList *list = variablelist;
    list->variablename->local_scope = true;
    UCommand_DEF *cdef = new UCommand_DEF (UDEF_VAR,
                                           list->variablename->copy(),
					   (UNamedParameters*) 0,
					   (UCommand*) 0);
    cdef->setTag(this);
    morph = cdef;
    list = list->next;

    while (list)
    {
      if (list->variablename)
      {
	list->variablename->local_scope = true;
	cdef = new UCommand_DEF (UDEF_VAR,
                                 list->variablename->copy(),
				 (UNamedParameters*) 0,
				 (UCommand*) 0);
	cdef->setTag(this);
	morph = (UCommand*) new UCommand_TREE(UAND, cdef, morph);
      }
      list = list->next;
    }

    persistant = false;
    return ( status = UMORPH );
  }

  return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_DEF::copy()
{
  UCommand_DEF *ret = new UCommand_DEF(deftype, ucopy (variablename),
				       ucopy (parameters),
				       ucopy (command));
  ret->variablelist = ucopy (variablelist);
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("DEF:\n");


  if (variablename)
  {
    ::urbiserver->debug("%s  Variablename:", tabb);
    variablename->print(); ::urbiserver->debug("\n");
  };
  if (variablelist)
  {
    ::urbiserver->debug("%s  Variablelist: {", tabb);
    variablelist->print(); ::urbiserver->debug("}\n");
  };

  if (parameters)
  {
    ::urbiserver->debug("%s  Param:{", tabb);
    parameters->print(); ::urbiserver->debug("}\n");
  };
  if (command)
  {
    ::urbiserver->debug("%s  Command:\n", tabb);
    command->print(l+3);
  };
  ::urbiserver->debug("%sEND DEF ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_CLASS);
// **************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_CLASS::UCommand_CLASS(UString *object,
			       UNamedParameters *parameters) :
  UCommand(CMD_CLASS)
{
  ADDOBJ(UCommand_CLASS);
  this->object       = object;
  this->parameters   = parameters;
}

//! UCommand subclass destructor.
UCommand_CLASS::~UCommand_CLASS()
{
  FREEOBJ(UCommand_CLASS);
  delete object;
  delete parameters;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_CLASS::execute(UConnection*)
{
  // remote new processing
  HMobjWaiting::iterator ow;
  ow = ::urbiserver->objWaittab.find(object->str());
  if (ow != ::urbiserver->objWaittab.end())
  {
    (*ow).second->nb--;
    if ((*ow).second->nb == 0)
      ::urbiserver->objWaittab.erase(ow);
    else
      return (status = URUNNING);
  }


  // add some object storage here based on 'object'
  new UObj(object);
  if (!parameters) return ( status = UCOMPLETED );

  // morph into a series of & for each element of the class
  morph = 0;

  UNamedParameters * param = parameters;
  UCommand_DEF *cdef=0;
  while (param)
    {
    if (param->expression)
      {
	switch (param->expression->type)
	  {
	  case EXPR_VALUE:
	    cdef = new UCommand_DEF(UDEF_VAR,
				    new UVariableName(
						      new UString(object),
						      new UString(param->expression->str),
						      true,
						      (UNamedParameters*)0),
				    (UNamedParameters*) 0,
				    (UCommand*) 0);
	    break;
	  case EXPR_FUNCTION:
	    cdef = new UCommand_DEF(UDEF_FUNCTION,
				    new UVariableName(
						      new UString(object),
						      new UString(param->expression->variablename->id),
						      true,
						      (UNamedParameters*)0),
				    param->expression->parameters,
				    (UCommand*) 0);
	    break;
	  case EXPR_EVENT:
	    cdef = new UCommand_DEF(UDEF_EVENT,
				    new UVariableName(
						      new UString(object),
						      new UString(param->expression->variablename->id),
						      true,
						      (UNamedParameters*)0),
				    param->expression->parameters,
				    (UCommand*) 0);
	    break;

	  case EXPR_VARIABLE:
	  case EXPR_LIST:
	  case EXPR_GROUP:
	  case EXPR_ADDR_VARIABLE:
	  case EXPR_PLUS:
	  case EXPR_MINUS:
	  case EXPR_MULT:
	  case EXPR_DIV:
	  case EXPR_MOD:
	  case EXPR_EXP:
	  case EXPR_NEG:
	  case EXPR_COPY:
	  case EXPR_PROPERTY:
	  case EXPR_TEST_EQ:
	  case EXPR_TEST_REQ:
	  case EXPR_TEST_PEQ:
	  case EXPR_TEST_DEQ:
	  case EXPR_TEST_NE:
	  case EXPR_TEST_GT:
	  case EXPR_TEST_GE:
	  case EXPR_TEST_LT:
	  case EXPR_TEST_LE:
	  case EXPR_TEST_BANG:
	  case EXPR_TEST_AND:
	  case EXPR_TEST_OR:
	    break;
	  }
	if (cdef)
        {
	  cdef->setTag(this);
	  if (param == parameters)
	    morph = cdef;
	  else
	    morph = (UCommand*) new UCommand_TREE(UAND, cdef, morph);
	}
      }

    param = param->next;
  }

  if (morph)
  {
    persistant = false;
    return ( status = UMORPH );
  }
  else
    return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_CLASS::copy()
{
  UCommand_CLASS *ret = new UCommand_CLASS(ucopy (object),
					   ucopy (parameters));
  copybase(ret);
  return ((UCommand*)ret);
}

//! Print the command
/*! This function is for debugging purpose only.
    It is not safe, efficient or crash proof. A better version will come later.
*/
void
UCommand_CLASS::print(int l)
{
  char tabb[100];

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("CLASS:\n");

  if (object)
  {
    ::urbiserver->debug("%s  Object name: %s\n", tabb,
                        object->str());
  };
  if (parameters)
  {
    ::urbiserver->debug("%s  Param:{", tabb);
    parameters->print(); ::urbiserver->debug("}\n");
  };
  ::urbiserver->debug("%sEND CLASS ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_IF);
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
  delete command1;
  delete command2;
  delete test;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_IF::execute(UConnection *connection)
{
  if (!test) return (status = UCOMPLETED);

  UTestResult testres = booleval(test->eval(this, connection));

  if (testres == UTESTFAIL)
    return ( status = UCOMPLETED );

  if (testres == UTRUE)
  {
    morph = command1;
    command1 = 0; // avoid delete of command when this is deleted
    persistant = false;
    return ( status = UMORPH );
  }
  else
    if (command2)
    {
      morph = command2;
      command2 = 0; // avoid delete of command when this is deleted
      persistant = false;
      return ( status = UMORPH );
    }
    else
      return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_IF::copy()
{
  UCommand_IF *ret = new UCommand_IF(ucopy (test),
				     ucopy (command1),
				     ucopy (command2));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("IF:\n");

  if (test)
  {
    ::urbiserver->debug("%s  Test:", tabb);
    test->print(); ::urbiserver->debug("\n");
  };
  if (command1)
  {
    ::urbiserver->debug("%s  Command1:\n", tabb);
    command1->print(l+3);
  };
  if (command2)
  {
    ::urbiserver->debug("%s  Command2:\n", tabb);
    command2->print(l+3);
  };
  ::urbiserver->debug("%sEND IF ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_EVERY);
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
  delete command;
  delete duration;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_EVERY::execute(UConnection *connection)
{
  ufloat thetime = connection->server->lastTime();

  if (command == 0) return ( status = UCOMPLETED );

  UValue *interval = duration->eval(this, connection);
  if (!interval) return ( status = UCOMPLETED);

  if ((starttime + interval->val <= thetime) ||
      (firsttime))
  {
    persistant = true;
    morph = (UCommand*)
      new UCommand_TREE( UAND,
			 command->copy(),
			 this
			 );
    starttime = thetime;
    firsttime = false;
    delete interval;
    return ( status = UMORPH );
  }

  delete interval;
  return ( status = UBACKGROUND );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_EVERY::copy()
{
  UCommand_EVERY *ret = new UCommand_EVERY(ucopy (duration),
					   ucopy (command));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("EVERY:");

  if (duration)
  {
    ::urbiserver->debug("%s  Duration:", tabb);
    duration->print(); ::urbiserver->debug("\n");
  };
  if (command)
  {
    ::urbiserver->debug("%s  Command:\n", tabb);
    command->print(l+3);
  };
  ::urbiserver->debug("%sEND EVERY ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_TIMEOUT);
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

  snprintf(tmpbuffer,
           UCommand::MAXSIZE_TMPMESSAGE, "__TAG_timeout_%d", (int)unic());
  this->tagRef      = new UString(tmpbuffer);
}

//! UCommand subclass destructor.
UCommand_TIMEOUT::~UCommand_TIMEOUT()
{
  FREEOBJ(UCommand_TIMEOUT);
  delete command;
  delete duration;
  delete tagRef;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_TIMEOUT::execute(UConnection*)
{
  if (command == 0)
    return ( status = UCOMPLETED );

  persistant = false;
  morph = (UCommand*)
    new UCommand_TREE( UAND,
		       new UCommand_TREE( UPIPE,
					  new UCommand_WAIT(duration->copy()),
					  new UCommand_OPERATOR_ID(new UString("stop"),
								   tagRef->copy())),
		       command->copy()
		       );

  morph->setTag(tagRef->str());
  return ( status = UMORPH );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_TIMEOUT::copy()
{
  UCommand_TIMEOUT *ret = new UCommand_TIMEOUT(ucopy (duration),
					       ucopy (command));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("TIMEOUT:");

  if (duration)
  {
    ::urbiserver->debug("%s  Duration:", tabb);
    duration->print(); ::urbiserver->debug("\n");
  };
  if (command)
  {
    ::urbiserver->debug("%s  Command:\n", tabb);
    command->print(l+3);
  };
  ::urbiserver->debug("%sEND TIMEOUT ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_STOPIF);
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

  snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE, "__TAG_stopif_%d", unic());
  this->tagRef      = new UString(tmpbuffer);
}

//! UCommand subclass destructor.
UCommand_STOPIF::~UCommand_STOPIF()
{
  FREEOBJ(UCommand_STOPIF);
  delete command;
  delete condition;
  delete tagRef;
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
  morph->setTag(tagRef->str());
  return ( status = UMORPH );


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

  morph->setTag(tagRef->str());
  return ( status = UMORPH );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_STOPIF::copy()
{
  UCommand_STOPIF *ret = new UCommand_STOPIF(ucopy (condition),
					     ucopy (command));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("STOPIF:");

  if (condition)
  {
    ::urbiserver->debug("%s  Condition:", tabb);
    condition->print(); ::urbiserver->debug("\n");
  };
  if (command)
  {
    ::urbiserver->debug("%s  Command:\n", tabb);
    command->print(l+3);
  };
  ::urbiserver->debug("%sEND STOPIF ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_FREEZEIF);
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

  snprintf(tmpbuffer,
           UCommand::MAXSIZE_TMPMESSAGE, "__TAG_stopif_%d", (int)unic());
  this->tagRef      = new UString(tmpbuffer);
}

//! UCommand subclass destructor.
UCommand_FREEZEIF::~UCommand_FREEZEIF()
{
  FREEOBJ(UCommand_FREEZEIF);
  delete command;
  delete condition;
  delete tagRef;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_FREEZEIF::execute(UConnection*)
{
  if (command == 0) return ( status = UCOMPLETED );

  persistant = false;
  UCommand* cmd = new UCommand_TREE( UPIPE,
				     command->copy(),
				     new UCommand_NOOP()
				     );
  cmd->setTag(tagRef->str());
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

  return ( status = UMORPH );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_FREEZEIF::copy()
{
  UCommand_FREEZEIF *ret = new UCommand_FREEZEIF(ucopy (condition),
						 ucopy (command));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("FREEZEIF:");

  if (condition)
  {
    ::urbiserver->debug("%s  Condition:", tabb);
    condition->print(); ::urbiserver->debug("\n");
  };
  if (command)
  {
    ::urbiserver->debug("%s  Command:\n", tabb);
    command->print(l+3);
  };
  ::urbiserver->debug("%sEND FREEZEIF ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_AT);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_AT::UCommand_AT( UCommandType type,
			  UExpression *test,
			  UCommand* command1,
			  UCommand* command2) :
  UCommand(type),
  UASyncCommand()
{
  ADDOBJ(UCommand_AT);
  this->test        = test;
  this->command1    = command1;
  this->command2    = command2;

  firsttime = true;
  reloop_ = false;
}

//! UCommand subclass destructor.
UCommand_AT::~UCommand_AT()
{
  FREEOBJ(UCommand_AT);
  delete command1;
  delete command2;
  delete test;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_AT::execute(UConnection *connection)
{
  UTestResult testres;
  UEventCompound* ec;
  UCommand* morph_onleave = 0;
  bool domorph = false;
  ufloat currentTime = connection->server->lastTime();

  if (!test) return (status = UCOMPLETED);

  if  (firsttime)
  {
    firsttime = false;
    if (test->asyncScan ( (UASyncCommand*)this, connection) == UFAIL)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
               "!!! invalid name resolution in test. "
               "Did you define all events and variables?\n");
      connection->send(tmpbuffer, getTag().c_str());
      return ((status = UCOMPLETED));
    }
  }

  if (reeval ())
  {
    ec = 0;
    UValue *testeval = test->eval(this, connection, ec);
    if (!ec) ec = new UEventCompound (testeval);
    reset_reeval ();

    testres = booleval(testeval, true);
    if (testres == UTESTFAIL)
      return ((status = UCOMPLETED));

    // softtest evaluation
    ufloat duration = 0;
    if (test->softtest_time)
      duration = test->softtest_time->val;

    std::list<UMultiEventInstance*> mixlist;
    ASSERT (ec)
    {
      ec->normalForm ();
      mixlist = ec->mixing ();
    }

    for (std::list<UMultiEventInstance*>::iterator imei = mixlist.begin ();
         imei != mixlist.end ();
         ++imei)
    {
      bool ok = false;
      for (std::list<UAtCandidate*>::iterator ic = candidates.begin ();
           ic != candidates.end () && !ok;
           ++ic)
      {
        if ((*ic)->equal (*imei))
        {
          (*ic)->visited ();
          ok = true;
          delete *imei;
        }
      }
      if (!ok)
        candidates.push_back (new UAtCandidate (currentTime + duration,
                                                *imei));
    }

    //cleanup of candidates that do not appear anymore in the mixlist
    for (std::list<UAtCandidate*>::iterator ic = candidates.begin ();
         ic != candidates.end ();
        )
      if (!(*ic)->isVisited ())
      {
        delete *ic;
        ic = candidates.erase (ic);
        if (command2)
        {
          if (!morph_onleave)
            morph_onleave = command2->copy ();
          else
            morph_onleave = (UCommand*) new UCommand_TREE
              (UAND, morph_onleave, command2->copy ());
          domorph = true;
          morph = this;
        }
      }
      else
      {
        (*ic)->unVisited ();
        ++ic;
      }
    delete ec;
    reloop_ = true;
  } //end reeval



  if (reloop_)
  {
    reloop_ = false;
    morph = this;
    // scan triggering candidates
    for  (std::list<UAtCandidate*>::iterator ic = candidates.begin ();
          ic != candidates.end ();
          ++ic)
    {
      UCommand* assigncmd;
      if  (!(*ic)->hasTriggered())
      {
        if ((*ic)->trigger (currentTime, assigncmd))
        {
          if (assigncmd)
          {
            morph = (UCommand*)
              new UCommand_TREE
              ( UAND,
                new UCommand_TREE
                ( UPIPE,
                  assigncmd,
                  command1->copy()
                ),
                morph
              );
          }
          else
            morph = (UCommand*)
              new UCommand_TREE (UAND, command1->copy (), morph);
        }
        else
          reloop_ = true; // we should try again later
      }
    }

    // morph if necessary
    if (morph != this) domorph = true;
  }

  // morphing, if required
  if (domorph)
  {
    if (morph_onleave)
    {
      // at this point, morph is at least equal to "this"
      morph = (UCommand*) new UCommand_TREE
        (UAND, morph, morph_onleave);
    }
    morph->background = true;
    persistant = true;
    return ((status = UMORPH));
  }

  return ((status = UBACKGROUND));
}

//! UCommand subclass hard copy function
UCommand*
UCommand_AT::copy()
{
  UCommand_AT *ret = new UCommand_AT(type,
				     ucopy (test),
				     ucopy (command1),
				     ucopy (command2));
  copybase(ret);
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] toDelete=%d ",
                      tabb, getTag().c_str(), toDelete);

  ::urbiserver->debug("AT:");
  if (type == CMD_AT) ::urbiserver->debug("\n");
  else
    if (type == CMD_AT_AND) ::urbiserver->debug("(AND)\n");
    else
      ::urbiserver->debug("UNKNOWN TYPE!\n");

  if (test)
  {
    ::urbiserver->debug("%s  Test:", tabb);
    test->print(); ::urbiserver->debug("\n");
  };
  if (command1)
  {
    ::urbiserver->debug("%s  Command1:\n", tabb);
    command1->print(l+3);
  };
  if (command2)
  {
    ::urbiserver->debug("%s  Command2:\n", tabb);
    command2->print(l+3);
  };
  ::urbiserver->debug("%sEND AT ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_WHILE);
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
  delete command;
  delete test;
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

  if (testres == UTRUE)
  {
    UNodeType     nodeType = USEMICOLON;

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
    return ( status = UMORPH );
  }
  else
    return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_WHILE::copy()
{
  UCommand_WHILE *ret = new UCommand_WHILE(type,
					   ucopy (test),
					   ucopy (command));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());
  ::urbiserver->debug("WHILE:");
  if (type == CMD_WHILE) ::urbiserver->debug("\n");
  else
    if (type == CMD_WHILE_AND) ::urbiserver->debug("(AND)\n");
    else
      if (type == CMD_WHILE_PIPE) ::urbiserver->debug("(PIPE)\n");
      else
	::urbiserver->debug("UNKNOWN TYPE!\n");

  if (test)
  {
    ::urbiserver->debug("%s  Test:", tabb);
    test->print(); ::urbiserver->debug("\n");
  };
  if (command)
  {
    ::urbiserver->debug("%s  Command:\n", tabb);
    command->print(l+3);
  };
  ::urbiserver->debug("%sEND WHILE ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_WHENEVER);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_WHENEVER::UCommand_WHENEVER(UExpression *test,
				     UCommand* command1,
				     UCommand* command2) :
  UCommand(CMD_WHENEVER),
  UASyncCommand()
{
  ADDOBJ(UCommand_WHENEVER);
  this->test        = test;
  this->command1    = command1;
  this->command2    = command2;

  firsttime = true;
  reloop_ = false;
  active_ = false;
  theloop_ = 0;
}

//! UCommand subclass destructor.
UCommand_WHENEVER::~UCommand_WHENEVER()
{
  FREEOBJ(UCommand_WHENEVER);
  delete command1;
  delete command2;
  delete test;
  if (theloop_)
    ((UCommand_LOOP*)theloop_)->whenever_hook = 0;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_WHENEVER::execute(UConnection *connection)
{
  UTestResult testres;
  UEventCompound* ec;
  bool domorph = false;
  ufloat currentTime = connection->server->lastTime();

  if (!test) return (status = UCOMPLETED);

  // handle the 'else' construct
  if (command2)
  {
    morph =  (UCommand*)
      new UCommand_TREE
      (
       UAND,
       this,
       new UCommand_WHENEVER
        (
         new UExpression (EXPR_TEST_BANG,
                          test->copy (),
                           (UExpression*)0),
         command2,
         (UCommand*)0
        )
      );

    command2 = 0;
    persistant = true;
    return ((status = UMORPH));
  }

  // cache initilialization
  if  (firsttime)
  {
    firsttime = false;
    if (test->asyncScan ( (UASyncCommand*)this, connection) == UFAIL)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
               "!!! invalid name resolution in test. "
               "Did you define all events and variables?\n");
      connection->send(tmpbuffer, getTag().c_str());
      return ((status = UCOMPLETED));
    }
  }

  if (reeval ())
  {
    ec = 0;
    UValue *testeval = test->eval(this, connection, ec);
    if (!ec) ec = new UEventCompound (testeval);
    reset_reeval ();

    testres = booleval(testeval, true);
    if (testres == UTESTFAIL)
      return ((status = UCOMPLETED));

    // softtest evaluation
    ufloat duration = 0;
    if (test->softtest_time)
      duration = test->softtest_time->val;

    std::list<UMultiEventInstance*> mixlist;
    ASSERT (ec)
    {
      ec->normalForm ();
      mixlist = ec->mixing ();
    }

    for (std::list<UMultiEventInstance*>::iterator imei = mixlist.begin ();
         imei != mixlist.end ();
         ++imei)
    {
      bool ok = false;
      for (std::list<UAtCandidate*>::iterator ic = candidates.begin ();
           ic != candidates.end () && !ok;
           ++ic)
      {
        if ((*ic)->equal (*imei))
        {
          (*ic)->visited ();
          ok = true;
          delete *imei;
        }
      }
      if (!ok)
        candidates.push_back (new UAtCandidate (currentTime + duration,
                                                *imei));
    }

    //cleanup of candidates that do not appear anymore in the mixlist
    for (std::list<UAtCandidate*>::iterator ic = candidates.begin ();
         ic != candidates.end ();
        )
      if (!(*ic)->isVisited ())
      {
        delete *ic;
        ic = candidates.erase (ic);
      }
      else
      {
        (*ic)->unVisited ();
        ++ic;
      }
    delete ec;
    reloop_ = true;

    if (candidates.empty () && active_) // whenever stops
    {
      active_ = false;
      connection->server->somethingToDelete = true;
      // theloop_ is 0 if something has deleted it from the outside  (thanks
      // to the 'whenever_hook' attribute), that's why we test here
      if (theloop_) theloop_->toDelete = true;
      theloop_ = 0;
    }
  } //end reeval

  if (reloop_)
  {
    reloop_ = false;
    bool trigger = false;
    UCommand* assign = 0;

    // scan triggering candidates
    for  (std::list<UAtCandidate*>::iterator ic = candidates.begin ();
          ic != candidates.end ();
          ++ic)
    {
      UCommand* assigncmd;
      if  (!(*ic)->hasTriggered())
      {
        if ((*ic)->trigger (currentTime, assigncmd))
        {
          trigger = true;
          if (assigncmd)
          {
            if (!assign) assign = assigncmd;
            else
              assign = (UCommand*)
                new UCommand_TREE
                ( UAND,
                  assigncmd,
                  assign
                );
          }
        }
        else
          reloop_ = true; // we should try again later
      }
    }

    if (trigger && !active_) // we need to start the loop
    {
      active_ = true;
      ASSERT (theloop_ == 0);
      theloop_ = (UCommand*) new UCommand_LOOP (command1->copy ());
      theloop_->setTag ("__system__"); //untouchable
      ((UCommand_LOOP*)theloop_)->whenever_hook = this;
      if (assign)
        assign = (UCommand*)
          new UCommand_TREE (UPIPE,
                             assign,
                             theloop_);
      else
        assign = theloop_;
    }

    if (assign)
    {
      domorph = true;
      morph = (UCommand*)
        new UCommand_TREE (UPIPE,
                           this,
                           assign);
    }
  }

  // morphing, if required
  if (domorph)
  {
    morph->background = true;
    persistant = true;
    return ((status = UMORPH));
  }

  return ((status = UBACKGROUND));
}

//! UCommand subclass hard copy function
UCommand*
UCommand_WHENEVER::copy()
{
  UCommand_WHENEVER *ret = new UCommand_WHENEVER(ucopy (test),
						 ucopy (command1),
						 ucopy (command2));
  copybase(ret);
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());
  ::urbiserver->debug("WHENEVER:\n");

  if (test)
  {
    ::urbiserver->debug("%s  Test:", tabb);
    test->print();
    ::urbiserver->debug("\n");
  };
  if (command1)
  {
    ::urbiserver->debug("%s  Command1:\n", tabb);
    command1->print(l+3);
  };
  if (command2)
  {
    ::urbiserver->debug("%s  Command2:\n", tabb);
    command2->print(l+3);
  };
  ::urbiserver->debug("%sEND WHENEVER ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_LOOP);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
*/
UCommand_LOOP::UCommand_LOOP(UCommand* command) :
  UCommand(CMD_LOOP)
{
  ADDOBJ(UCommand_LOOP);
  this->command     = command;
  whenever_hook = 0;
}

//! UCommand subclass destructor.
UCommand_LOOP::~UCommand_LOOP()
{
  FREEOBJ(UCommand_LOOP);
  delete command;
  if (whenever_hook)
   ((UCommand_WHENEVER*)whenever_hook)->noloop ();
}

//! UCommand subclass execution function
UCommandStatus
UCommand_LOOP::execute(UConnection*)
{
  if (command == 0) return ( status = UCOMPLETED );

  morph = (UCommand*)
    new UCommand_TREE(USEMICOLON,
		      new UCommand_TREE(UAND,
					command->copy(),
					new UCommand_NOOP()),
		      this);
  persistant = true;
  return ( status = UMORPH );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_LOOP::copy()
{
  UCommand_LOOP *ret = new UCommand_LOOP(ucopy (command));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] toDelete=%d",
                      tabb, getTag().c_str(), toDelete);

  ::urbiserver->debug("LOOP:\n");

  if (command)
  {
    ::urbiserver->debug("%s  Command:\n", tabb);
    command->print(l+3);
  };
  ::urbiserver->debug("%sEND LOOP ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_LOOPN);
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
  delete command;
  delete expression;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_LOOPN::execute(UConnection *connection)
{
  if (command == 0) return ( status = UCOMPLETED );

  //if (status == UONQUEUE)
  //  command->status = UONQUEUE;

  if (expression->type != EXPR_VALUE)
  {
    UValue *nb = expression->eval(this, connection);

    if (nb == 0)
      return ( status = UCOMPLETED );

    if (nb->dataType != DATA_NUM)
    {
      snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
	       "!!! number of loops is non numeric\n");
      connection->send(tmpbuffer, getTag().c_str());
      delete nb;
      return ( status = UCOMPLETED );
    }

    expression->type = EXPR_VALUE;
    expression->dataType = DATA_NUM;
    expression->val = nb->val;
    delete nb;
  }

  if (expression->val < 1)
    return ( status = UCOMPLETED);

  expression->val = expression->val - 1;

  UNodeType nodeType = nodeType_loopn (type);

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
  return ( status = UMORPH );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_LOOPN::copy()
{
  UCommand_LOOPN *ret = new UCommand_LOOPN(type,
					   ucopy (expression),
					   ucopy (command));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("LOOPN:");
  if (type == CMD_LOOPN) ::urbiserver->debug("\n");
  else
    if (type == CMD_LOOPN_AND) ::urbiserver->debug("(AND)\n");
    else
      if (type == CMD_LOOPN_PIPE) ::urbiserver->debug("(PIPE)\n");
      else
	::urbiserver->debug("UNKNOWN TYPE!\n");

  if (expression)
  {
    ::urbiserver->debug("%s  Expr:", tabb);
    expression->print(); ::urbiserver->debug("\n");
  };
  if (command)
  {
    ::urbiserver->debug("%s  Command (%ld:%d):\n", tabb,
                        (long)command,
                        (int)command->status); command->print(l+3);
  };

  ::urbiserver->debug("%sEND LOOPN ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_FOR);
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
  delete command;
  delete instr1;
  delete instr2;
  delete test;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_FOR::execute(UConnection *connection)
{
  UCommand *tmp_instr2;

  if (first)
  {
    first = false;
    if (instr1) instr1->setTag(this);
    if (instr2) instr2->setTag(this);
  }

  if (command == 0) return ( status = UCOMPLETED );

  if (instr1)
  {
    UCommand *first_instruction = instr1;

    instr1 = 0;
    morph = (UCommand*)
      new UCommand_TREE(USEMICOLON,
			first_instruction,
			this);
    persistant = true;
    return ( status = UMORPH );
  }

  persistant = false;

  if (!test) return (status = UCOMPLETED);
  UTestResult testres = booleval(test->eval(this, connection));

  if (testres == UTESTFAIL)
    return ( status = UCOMPLETED );

  if (testres == UTRUE)
  {
    UNodeType nodeType = nodeType_for (type);
    tmp_instr2 = 0;

    if (nodeType == UPIPE
        || nodeType == UAND)
    {
      if (instr2)
        morph = (UCommand*)
          new UCommand_TREE(nodeType,
                            command->copy(),
                            new UCommand_TREE(UPIPE,
                                              tmp_instr2 = instr2->copy(),
                                              this
                                             )
                           );
      else
        morph = (UCommand*)
          new UCommand_TREE(nodeType,
                            command->copy(),
                            this
                           );
    }
    else {

      if (instr2)
        morph = (UCommand*)
          new UCommand_TREE(nodeType,
                            new UCommand_TREE(UAND,
                                              new UCommand_TREE
                                              (
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
          new UCommand_TREE(nodeType,
                            new UCommand_TREE(
                                              UAND,
                                              command->copy(),
                                              new UCommand_NOOP()
                                             ),
                            this
                           );
    }
    if (tmp_instr2)
      tmp_instr2->morphed = true;
    persistant = true;
    return ( status = UMORPH );
  }
  else
    return ( status = UCOMPLETED );
}

//! UCommand subclass hard copy function
UCommand*
UCommand_FOR::copy()
{
  UCommand_FOR *ret = new UCommand_FOR(type,
				       ucopy (instr1),
				       ucopy (test),
				       ucopy (instr2),
				       ucopy (command));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("FOR:");
  if (type == CMD_FOR) ::urbiserver->debug("\n");
  else
    if (type == CMD_FOR_AND) ::urbiserver->debug("(AND)\n");
    else
      if (type == CMD_FOR_PIPE) ::urbiserver->debug("(PIPE)\n");
      else
	::urbiserver->debug("UNKNOWN TYPE!\n");

  if (test)
  {
    ::urbiserver->debug("%s  Test:", tabb);
    test->print();
    ::urbiserver->debug("\n");
  };
  if (instr1)
  {
    ::urbiserver->debug("%s  Instr1:\n", tabb);
    instr1->print(l+3);
  };
  if (instr2)
  {
    ::urbiserver->debug("%s  Instr2:\n", tabb);
    instr2->print(l+3);
  };
  if (command)
  {
    ::urbiserver->debug("%s  Command:\n", tabb);
    command->print(l+3);
  };

  ::urbiserver->debug("%sEND FOR ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_FOREACH);
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
  delete command;
  delete variablename;
  delete expression;
}

//! UCommand subclass execution function
UCommandStatus
UCommand_FOREACH::execute(UConnection *connection)
{
  if (firsttime)
  {
    firsttime = false;
    position = expression->eval(this, connection);
    if (position == 0)
      return ((status = UCOMPLETED));
    if (position->dataType == DATA_LIST)
      position = position->liststart;
  }

  if (position == 0)
    return ((status = UCOMPLETED));

  UNodeType nodeType = nodeType_foreach (type);

  UExpression* currentvalue = new UExpression(EXPR_VALUE, ufloat(0));
  if (!currentvalue)
    return ((status = UCOMPLETED));
  currentvalue->dataType = position->dataType;
  if (position->dataType == DATA_NUM)    currentvalue->val = position->val;
  if (position->dataType == DATA_STRING)
    currentvalue->str = new UString(position->str);
  if (position->dataType == DATA_BINARY)
  {
    // add support here
  }
  if ((position->dataType != DATA_NUM) &&
      (position->dataType != DATA_STRING))
  {
    snprintf(tmpbuffer, UCommand::MAXSIZE_TMPMESSAGE,
             "!!! This type is not supported yet\n");
    connection->send(tmpbuffer, getTag().c_str());
    delete currentvalue;
    return ( status = UCOMPLETED );
  }

  morph =  (UCommand*)
    new UCommand_TREE(
                      nodeType,
                      new UCommand_TREE(
                                        UPIPE,
                                        new UCommand_ASSIGN_VALUE
                                        (
                                         variablename->copy(),
                                         currentvalue,
                                         (UNamedParameters*)0),
                                        command->copy()),
                      this
                     );
  ((UCommand_TREE*)((UCommand_TREE*)morph)->command1)->command1->setTag(this);

  position = position->next;
  persistant = true;
  return (status = UMORPH);
}

//! UCommand subclass hard copy function
UCommand*
UCommand_FOREACH::copy()
{
  UCommand_FOREACH *ret = new UCommand_FOREACH(type,
					       ucopy (variablename),
					       ucopy (expression),
					       ucopy (command));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("FOREACH:");
  if (type == CMD_FOREACH)
    ::urbiserver->debug("\n");
  else if (type == CMD_FOREACH_AND)
    ::urbiserver->debug("(AND)\n");
  else if (type == CMD_FOREACH_PIPE)
    ::urbiserver->debug("(PIPE)\n");
  else
    ::urbiserver->debug("UNKNOWN TYPE!\n");

  if (variablename)
  {
    ::urbiserver->debug("%s  VariableName:", tabb);
    variablename->print(); ::urbiserver->debug("\n");
  };
  if (expression)
  {
    ::urbiserver->debug("%s  List:", tabb);
    expression->print(); ::urbiserver->debug("\n");
  };
  if (command)
  {
    ::urbiserver->debug("%s  Command:\n", tabb);
    command->print(l+3);
  };

  ::urbiserver->debug("%sEND FOREACH ------\n", tabb);
}

MEMORY_MANAGER_INIT(UCommand_NOOP);
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
  if (status == UONQUEUE)
  {
    if (!connection->receiving)
      status = URUNNING;
    else
      status = UONQUEUE;
  }
  else
    status = UCOMPLETED;
  return (status);
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

  ::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());
  ::urbiserver->debug("NOOP, level =%d\n", (int)status);
}

MEMORY_MANAGER_INIT(UCommand_LOAD);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
    This class is used to delay the processing of a loaded file.
*/
UCommand_LOAD::UCommand_LOAD(UCommand_TREE* mainnode) :
  UCommand(CMD_LOAD)
{
  ADDOBJ(UCommand_LOAD);

  loadQueue = new UCommandQueue (4096, 1048576, false);
  this->mainnode = mainnode;
}

//! UCommand subclass destructor.
UCommand_LOAD::~UCommand_LOAD()
{
  FREEOBJ(UCommand_LOAD);

  delete loadQueue;
}

//! UCommand subclass execution function
UCommandStatus UCommand_LOAD::execute(UConnection *connection)
{
  if (connection->receiving) return (status = URUNNING);

  int length;
  ubyte* str_command = loadQueue->popCommand(length);

  if ((str_command == 0) && (length==-1))
    return (status = UCOMPLETED);

  if (length !=0)
    {
      ::urbiserver->parser.commandTree = 0;
      errorMessage[0] = 0;

      ::urbiserver->systemcommands = false;
      ::urbiserver->parser.process(str_command, length, connection);
      ::urbiserver->systemcommands = true;

      if (errorMessage[0] != 0)
      { // a parsing error occured
	if (::urbiserver->parser.commandTree)
        {
	  delete ::urbiserver->parser.commandTree;
	  ::urbiserver->parser.commandTree = 0;
	}

	connection->send(errorMessage, "error");
	return (status = UCOMPLETED);
      }
      else {

	::urbiserver->parser.commandTree->setTag("__system__");
	::urbiserver->parser.commandTree->command2 = this;
	up = ::urbiserver->parser.commandTree;
	position = &(::urbiserver->parser.commandTree->command2);
	morph = ::urbiserver->parser.commandTree;
	::urbiserver->parser.commandTree = 0;

	persistant = true;
	return ( status = UMORPH );
      }
    }
  else
    return ((status = UCOMPLETED));
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

  strcpy(tabb, "");
  for (int i=0;i<l;i++)
    strcat(tabb, " ");

::urbiserver->debug("%s Tag:[%s] ", tabb, getTag().c_str());

  ::urbiserver->debug("LOAD\n");
}
