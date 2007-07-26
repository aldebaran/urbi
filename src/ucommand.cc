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
// #define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"
#include "config.h"

#include "libport/cstdio"
#include <cstdarg>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include "libport/cstring"

#include "libport/containers.hh"
#include "libport/ref-pt.hh"

#include "urbi/uobject.hh"
#include "urbi/usystem.hh"

#include "kernel/userver.hh"
#include "kernel/uconnection.hh"
#include "kernel/uvalue.hh"
#include "kernel/uvariable.hh"

#include "uasynccommand.hh"
#include "uatcandidate.hh"
#include "ubinary.hh"
#include "ubinder.hh"
#include "ucallid.hh"
#include "ucommand.hh"
#include "ucopy.hh"
#include "ueventcompound.hh"
#include "ueventhandler.hh"
#include "ueventinstance.hh"
#include "parser/uparser.hh"
#include "ufunction.hh"
#include "ugroup.hh"
#include "unamedparameters.hh"
#include "uvariablename.hh"
#include "uvariablelist.hh"

/// Report an error, with "!!! " prepended, and "\n" appended.
/// \param c     the connection to which the message is sent.
/// \param cmd   the command whose tag will be used.
/// \param fmt   printf-format string.
/// \param args  its arguments.
UErrorValue
vsend_error (UConnection* c, const UCommand* cmd,
	     const char* fmt, va_list args)
{
  std::ostringstream o;
  // FIXME: This is really bad if file names have %.  We need
  // something more robust (such using real C++ here instead of C
  // buffers).
  o << "!!! " << cmd->loc() << ": " << fmt << '\n';
  return c->sendf (cmd->getTag(), o.str().c_str(), args);
}

UErrorValue
send_error (UConnection* c, const UCommand* cmd,
	    const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  UErrorValue res = vsend_error (c, cmd, fmt, args);
  va_end(args);
  return res;
}

const char*
to_string (UCommand::Status s)
{
  switch (s)
  {
#define CASE(K) case UCommand::K: return #K;
    CASE(UONQUEUE);
    CASE(URUNNING);
    CASE(UCOMPLETED);
    CASE(UBACKGROUND);
    CASE(UMORPH);
    CASE(UFALLTHRU);
#undef CASE
  }
  // Pacify warnings.
  pabort ("unexpected case:" << s);
}

namespace
{
  /// Report the value of some attribute.
#define DEBUG_ATTR(Attr)			\
  do {						\
    if (Attr)					\
    {						\
      debug(l, "  " # Attr ":\n");		\
      (Attr)->print();				\
      debug("\n");				\
    }						\
  } while (0)

  /// Attribute that output the ending \n themselves, and support indentation.
#define DEBUG_ATTR_I(Attr)			\
  do {						\
    if (Attr)					\
    {						\
      debug(l, "  " # Attr ":\n");		\
      (Attr)->print(l + 3);			\
      debug("\n");				\
    }						\
  } while (0)
}

MEMORY_MANAGER_INIT(UCommand);



/// Cache the location of notag and system taginfos
TagInfo* UCommand::notagTagInfo = 0;
TagInfo* UCommand::systemTagInfo = 0;


// **************************************************************************
//! UCommand constructor.
/*! The parameter 'type' is required here to describe the type of the command.

 \param type is the command type
 */
UCommand::UCommand(const location& l, Type _type)
  : UAst (l),
    type (_type),
    status (UONQUEUE),
    myconnection(0),
    groupOfCommands(false),
    flags (0),
    morph (0),
    persistant (false),
    toDelete (false),
    background (false),
    frozen (false),
    startTime (-1),
    flagExpr1 (0),
    flagExpr2 (0),
    flagExpr4 (0),
    flagType (0),
    flag_nbTrue2 (0),
    flag_nbTrue4 (0),
    morphed (false),
    tag (""),
    tagInfo (0)
{
  /*XXX todo: L1:remove this, assert to ensure a setTag is called before use
   L2: pass a tag or a command ptr to ctor
   */
  if (::urbiserver->isRunningSystemCommands ())
    setTag(systemTagInfo); //untouchable
  else
    setTag(notagTagInfo); //untouchable
}

//! UCommand destructor.
UCommand::~UCommand()
{
  if (myconnection && flags && flags->notifyEnd && !morph)
    myconnection->send("*** end\n", getTag().c_str());
  unsetTag();
  delete flags;
}


void
UCommand::initializeTagInfos()
{
  // empty name, no parent, not a pb
  TagInfo* dummy = new TagInfo();

  TagInfo t;
  t.name = "__system__";
  systemTagInfo = t.insert(urbiserver->getTagTab ());
  // insert a dummy tag in subtag list, so that the taginfo is never deleted
  systemTagInfo->subTags.push_back(dummy);
  t.name = "notag";
  notagTagInfo =  t.insert(urbiserver->getTagTab ());
  notagTagInfo->subTags.push_back(dummy);
}


UCommand::Status
UCommand::execute(UConnection* c)
{
#ifdef ENABLE_DEBUG_TRACES
  print(0);
#endif
  myconnection = c;
  return status = execute_ (c);
}

//! UCommand base of hard copy function
UCommand*
UCommand::copybase(UCommand* c) const
{
  c->setTag(this);
  if (flags)
    c->flags = flags->copy();
  c->myconnection = myconnection;
  c->groupOfCommands = groupOfCommands;
  return c;
}


//! Print command
void
UCommand::print(unsigned l) const
{
  const char* k = 0;
  switch (type)
  {
#define CASE(K) case K: k = #K; break
    CASE(ALIAS);
    CASE(ASSIGN_BINARY);
    CASE(ASSIGN_PROPERTY);
    CASE(ASSIGN_VALUE);
    CASE(AT);
    CASE(CLASS);
    CASE(DECREMENT);
    CASE(DEF);
    CASE(ECHO);
    CASE(EMIT);
    CASE(EVERY);
    CASE(EXPR);
    CASE(FOREACH);
    CASE(FOR);
    CASE(FREEZEIF);
    CASE(GENERIC);
    CASE(GROUP);
    CASE(IF);
    CASE(INCREMENT);
    CASE(INHERIT);
    CASE(LOAD);
    CASE(LOOP);
    CASE(LOOPN);
    CASE(NEW);
    CASE(NOOP);
    CASE(RETURN);
    CASE(STOPIF);
    CASE(TIMEOUT);
    CASE(TREE);
    CASE(WAIT);
    CASE(WAIT_TEST);
    CASE(WHENEVER);
    CASE(WHILE);
#undef CASE
  }
  debug(l, " Tag:[%s] %s (toDelete=%s status=%s)\n",
	getTag().c_str(), k,
	toDelete ? "true" : "false",
	to_string(status));
  print_ (l);
  debug(l, "END %s ------\n", k);
}

//! Command auto morphing according to group hierarchy
UCommand*
UCommand::scanGroups(UVariableName** (UCommand::*refName)(),
		     bool with_nostruct)
{
  //  PING();
  if (!(this->*refName)())
    // We are in a non broadcastable command.
    return 0;
  UVariableName *varname = *(this->*refName)();
  UString	*devicename = varname->getDevice();
  UString	*method	    = varname->getMethod();

  if (!varname->rooted && devicename)
  {
    UGroup* oo;
    if (varname->nostruct && with_nostruct)
      oo = libport::find0(::urbiserver->getGroupTab (), method->c_str());
    else
      oo = libport::find0(::urbiserver->getGroupTab (), devicename->c_str());

    if (oo && !oo->members.empty())
    {
      UCommand *gplist = 0;
      for (std::list<UString*>::iterator i = oo->members.begin();
	   i != oo->members.end();
	   ++i)
      {
	UCommand *clone = copy();
	UVariableName*& clonename = *((clone->*refName)());
	delete clonename;

	// FIXME: Why don't we use clone here on the UVariableName.
	// There are some attributes that are not copied (e.g.,
	// isstatic).  This is way too error-prone.
	if (varname->nostruct && with_nostruct)
	  clonename = new UVariableName(devicename->copy(), (*i)->copy(),
					false, ucopy (varname->index));
	else
	  clonename = new UVariableName((*i)->copy(), method->copy(),
					false, ucopy (varname->index));

	clonename->isnormalized = varname->isnormalized;
	clonename->deriv = varname->deriv;
	clonename->varerror = varname->varerror;
	clonename->nostruct = varname->nostruct;
	clonename->local_scope = varname->local_scope;
	gplist = new UCommand_TREE(location(), Flavorable::UAND, clone, gplist);
      }

      morph = gplist;

      varname->rooted = true;
      varname->fromGroup = true;
      persistant = false;
      return morph;
    }
  }

  return 0;
}

void
UCommand::setTag(const std::string& tag)
{
  if (tag == this->tag)
    return;
  if (tag != "")
    unsetTag();
  this->tag = tag;

  TagInfo* ti;
  HMtagtab::iterator it = urbiserver->getTagTab ().find(tag);

  if (it == urbiserver->getTagTab ().end())
  {
    TagInfo t;
    t.blocked = t.frozen = false;
    t.name = tag;
    ti = t.insert(urbiserver->getTagTab ());
  }
  else
    ti = &it->second;

  //add ourself to the taginfo, add it to ourself
  ti->commands.push_back(this);
  tagInfoPtr = --ti->commands.end();
  tagInfo = ti; //we know he won't die before us
}


void
UCommand::setTag(TagInfo* ti)
{
  if (tag == ti->name)
    return;
  if (tag != "")
    unsetTag();
  this->tag = ti->name;
  tagInfo = ti;
  if (tagInfo)
  {
    tagInfo->commands.push_back(this);
    tagInfoPtr = --tagInfo->commands.end();
  }
}

void
UCommand::setTag(const UCommand* cmd)
{
  if (tag == cmd->tag)
    return;
  if (tag != "")
    unsetTag();
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
  // FIXME: Most of this code should be moved into TagInfo.
  if (!tagInfo)
    return; //nothing to do
  tagInfo->commands.erase(tagInfoPtr);
  TagInfo* ti = tagInfo;
  while (ti && ti->commands.empty() && ti->subTags.empty()
	 && !ti->frozen && !ti->blocked)
  {
    //remove from parent list
    if (ti->parent)
      ti->parent->subTags.erase(ti->parentPtr);
    //remove from hash table
    TagInfo* next = ti->parent;
    urbiserver->getTagTab ().erase(urbiserver->getTagTab ().find(ti->name));
    //try again on our parent
    ti = next;
  }
}

bool
UCommand::isFrozen()
{
  for (TagInfo* t = tagInfo; t; t = t->parent)
    if (t->frozen)
    {
      // set the command in the frozen state. The 'execute' method is responsible to
      // reset the state to unfrozen once the command continues running.
      if (!frozen && myconnection && flags && flags->notifyFreeze && !morph)
	myconnection->send("*** frozen\n", getTag().c_str());

      frozen = true;

      return true;
    }

  if (frozen && myconnection && flags && flags->notifyFreeze && !morph)
    myconnection->send("*** unfrozen\n", getTag().c_str());

  return false;
}

bool
UCommand::isBlocked()
{
  for (TagInfo* t = tagInfo; t; t = t->parent)
    if (t->blocked)
      return true;
  return false;
}

void
UCommand::strMorph (const std::string& cmd)
{
  morph = new UCommand_EXPR
    (
      loc(),
      new UExpression
      (
	loc(),
	UExpression::FUNCTION,
	new UVariableName (new UString("global"), new UString("exec"),
			   false, 0),
	new UNamedParameters
	(new UExpression (loc(), UExpression::VALUE, new UString(cmd.c_str())))
	)
      );
  status = UMORPH;
}

MEMORY_MANAGER_INIT(UCommand_TREE);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 The background parameter lets the tree execute in background.
 This is useful for the LOAD command which should be run in bg and
 still cannot be persistant (like a AT or WHENEVER).
 */
UCommand_TREE::UCommand_TREE(const location& l,
			     UNodeType flavor,
			     UCommand* command1,
			     UCommand* command2)
  : UCommand(l, TREE),
    Flavorable (flavor),
    command1 (command1),
    command2 (command2),
    callid (0),
    runlevel1 (UWAITING),
    runlevel2 (UWAITING),
    connection (0) // unknown unless there is a context).
{
  ADDOBJ(UCommand_TREE);

  background = false;
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
UCommand::Status
UCommand_TREE::execute_(UConnection*)
{
  return URUNNING;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_TREE::copy() const
{
  return copybase(new UCommand_TREE(loc_, flavor(),
				    ucopy (command1),
				    ucopy (command2)));
}

//! Deletes sub commands marked for deletion after a stop command
void
UCommand_TREE::deleteMarked()
{
  int go_to = 1;
  UCommand_TREE* tree = this;

  while (tree != up)
  {
    if (tree->command1 && go_to == 1)
      if (tree->command1->toDelete)
      {
	delete tree->command1;
	tree->command1 = 0;
      }
      else if (tree->command1->type == TREE)
      {
	tree = dynamic_cast<UCommand_TREE*> (tree->command1);
	assert (tree);
	go_to = 1;
	continue;
      }

    if (tree->command2 && go_to >= 1)
      if (tree->command2->toDelete)
      {
	delete tree->command2;
	tree->command2 = 0;
      }
      else if (tree->command2->type == TREE)
      {
	tree = dynamic_cast<UCommand_TREE*> (tree->command2);
	assert (tree);
	go_to = 1;
	continue;
      }

    go_to = 2;
    if (tree->up && *tree->position == tree->up->command2)
      go_to = 0;

    tree = tree->up;
  }
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_TREE::print_(unsigned l) const
{
  debug(l, "%s (%p:%d) :\n",
	flavor_string(), this, status);
  if (command1)
  {
    debug(l, "  Com1 (%p:%d) up=%p:\n",
	  command1, command1->status, command1->up);
    command1->print(l+3);
  }
  if (command2)
  {
    debug(l, "  Com2 (%p:%d) up=%p:\n",
	  command2, command2->status, command2->up);
    command2->print(l+3);
  }
  debug(l, "END TREE ------\n");
}

MEMORY_MANAGER_INIT(UCommand_ASSIGN_VALUE);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_ASSIGN_VALUE::UCommand_ASSIGN_VALUE(const location& l,
					     UVariableName *variablename,
					     UExpression* expression,
					     UNamedParameters *parameters,
					     bool defkey)
  : UCommand(l, ASSIGN_VALUE),
    variablename(variablename),
    variable (0),
    expression (expression),
    parameters (parameters),
    method (0),
    devicename (0),
    modif_time (0),
    modif_sin (0),
    modif_phase (0),
    modif_smooth (0),
    modif_speed (0),
    modif_accel (0),
    modif_ampli (0),
    modif_adaptive (0),
    modif_getphase (0),
    endtime (-1),
    finished (false),
    profileDone (false),
    assigned (false),
    defkey (defkey),
    controlled (false)
{
  ADDOBJ(UCommand_ASSIGN_VALUE);
}

//! UCommand subclass destructor.
UCommand_ASSIGN_VALUE::~UCommand_ASSIGN_VALUE()
{
  FREEOBJ(UCommand_ASSIGN_VALUE);
  delete expression;
  delete variablename;
  delete parameters;

  if (assigned)
  {
    --variable->nbAssigns;
    if (variable->cancel == this)
      variable->cancel = 0;
  }
}

// Function call. morph into the function code
UCommand::Status
UCommand_ASSIGN_VALUE::execute_function_call(UConnection *connection)
{
  passert (expression, expression->type == UExpression::FUNCTION);
  UString* functionname =
    expression->variablename->buildFullname(this, connection);
  if (!functionname)
    return UCOMPLETED;

  if (scanGroups(&UCommand::refVarName2, true))
    return UMORPH;

  ////// INTERNAL /////

  ////// Native URBI: user-defined /////

  UFunction *fun =
    libport::find0(urbiserver->getFunctionTab (), functionname->c_str());
  if (!fun)
  {
    //trying inheritance
    const char* devname = expression->variablename->getDevice()->c_str();
    if (UObj* o = libport::find0(::urbiserver->getObjTab (), devname))
    {
      bool ambiguous;
      fun = o->searchFunction
	(expression->variablename->getMethod()->c_str(), ambiguous);

      //hack until we get proper nameresolution
      if (fun == kernel::remoteFunction)
	fun = 0;

      if (ambiguous)
      {
	send_error(connection, this,
		   "Ambiguous multiple inheritance on function %s",
		   functionname->c_str());
	return UCOMPLETED;
      }
    }
  }


  if (fun)
  {
    if ((expression->parameters
	 && fun->nbparam()
	 && expression->parameters->size() != fun->nbparam())
	|| (expression->parameters && !fun->nbparam())
	|| (!expression->parameters && fun->nbparam()))
    {
      send_error(connection, this,
		 "Invalid number of arguments for %s (should be %d)",
		 functionname->c_str(), fun->nbparam());
      return UCOMPLETED;
    }

    persistant = false;
    UVariableName* resultContainer =
      new UVariableName(new UString("__UFnct"), new UString("__result__"),
			true, 0);

    morph =
      new UCommand_TREE
      (loc_, Flavorable::UPIPE, fun->cmdcopy(),
       new UCommand_ASSIGN_VALUE (loc_,
				  variablename->copy(),
				  new UExpression(loc_,
						  UExpression::VARIABLE,
						  resultContainer),
				  0));

    if (morph)
    {
      UString* fundevice = expression->variablename->getDevice();

      UCommand_TREE* uc_tree = dynamic_cast<UCommand_TREE*> (morph);
      assert (uc_tree);
      // handle the :: case
      if (expression->variablename->doublecolon
	  && !connection->stack.empty ()
	  && libport::mhas(::urbiserver->getObjTab (),
			   connection->stack.front()->self().c_str()))
	*fundevice = connection->stack.front()->self();

      uc_tree->callid = new UCallid (unique ("__UFnct"),
                                     fundevice->c_str (), uc_tree);

      resultContainer->nameUpdate(uc_tree->callid->str(), "__result__");
      // creates return variable
      uc_tree->callid->setReturnVar (
	new UVariable (uc_tree->callid->str().c_str(), "__result__",
		       new UValue ()));

      if (!uc_tree->callid)
	return UCOMPLETED;
      uc_tree->connection = connection;

      UNamedParameters *pvalue = expression->parameters;
      UNamedParameters *pname	 = fun->parameters;
      for (;
	   pvalue != 0;
	   pvalue = pvalue->next, pname = pname->next)
      {
	UValue* valparam = pvalue->expression->eval(this, connection);
	if (!valparam)
	{
	  send_error(connection, this, "EXPR evaluation failed");
	  return UCOMPLETED;
	}

	uc_tree->callid->store
	  (new UVariable(uc_tree->callid->str().c_str(),
			 pname->name->c_str(),
			 valparam));
      }
    }

    return UMORPH;
  }


  // handle the :: case
  if (expression->variablename->doublecolon
      && !connection->stack.empty ()
      && libport::mhas(::urbiserver->getObjTab (),
		       connection->stack.front()->self().c_str()))
  {
    // rebuild name with parent class
    *expression->variablename->device = connection->stack.front()->self();
    expression->variablename->resetCache (); // this deletes funname pointeur
    functionname = expression->variablename->buildFullname(this, connection);
    if (!functionname)
      return UCOMPLETED;
  }


  ////// module-defined /////
  bool found_function = false;
  urbi::UTable::iterator hmfi = urbi::functionmap->find(functionname->c_str());
  if (hmfi != urbi::functionmap->end())
  {
    for (std::list<urbi::UGenericCallback*>::iterator cbi =
	   hmfi->second.begin();
	 cbi != hmfi->second.end() && !found_function;
	 ++cbi)
    {
      if ((expression->parameters
	   && expression->parameters->size() == (*cbi)->nbparam)
	  || (!expression->parameters && !(*cbi)->nbparam) )
      {
	urbi::UList tmparray;
	for (UNamedParameters* pvalue = expression->parameters;
	     pvalue;
	     pvalue = pvalue->next)
	{
	  UValue* valparam = pvalue->expression->eval(this, connection);
	  if (!valparam)
	  {
	    send_error(connection, this, "EXPR evaluation failed");
	    return UCOMPLETED;
	  }
	  // urbi::UValue do not see ::UValue, so it must
	  // be valparam who does the job.
	  tmparray.array.push_back(valparam->urbiValue());
	  delete valparam;
	}

	delete expression;
	expression =
	  new UExpression (loc(),  UExpression::VALUE,
			   new UValue((*cbi)->__evalcall(tmparray)));
	found_function = true;
      }
    }
  }

  ////// EXTERNAL /////
  if (!found_function)
  {
    UBinder* b = libport::find0(::urbiserver->getFunctionBinderTab (),
				functionname->c_str());
    if (b
	&& (expression->parameters
	    ? b->nbparam == expression->parameters->size()
	    : b->nbparam == 0)
	&& !b->monitors.empty())
    {
      std::string uid = unique ("__UFnctret.EXTERNAL_");
      {
	std::ostringstream o;
	o << "[0,"
	  << "\"" << functionname->c_str()
	  << "__" << b->nbparam << "\","
	  << "\"" << uid << "\"";

	for (std::list<UMonitor*>::iterator j = b->monitors.begin();
	     j != b->monitors.end();
	     ++j)
	{
	  (*j)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
	  (*j)->c->sendc(reinterpret_cast<const ubyte*>(o.str().c_str()),
			 o.str().size());
	  for (UNamedParameters *pvalue = expression->parameters;
	       pvalue != 0;
	       pvalue = pvalue->next)
	  {
	    (*j)->c->sendc(reinterpret_cast<const ubyte*>(","), 1);
	    UValue* valparam = pvalue->expression->eval(this, connection);
	    valparam->echo((*j)->c);
	  }
	  (*j)->c->send(reinterpret_cast<const ubyte*>("]\n"), 2);
	}
      }

      persistant = false;
      {
	std::ostringstream o;
	o << "{"
	  << "  waituntil(isdef(" << uid << "))|"
	  << variablename->getFullname()->c_str()
	  << "=" << uid
	  << "|delete " << uid
	  << "}";
	strMorph (o.str());
      }
      return UMORPH;
    }
  }
  // FIXME: Check that it is indeed possible to reach that point.
  // If it is not, get rid of the enum value UFALLTHRU, it is useless.
  return UFALLTHRU;
}

namespace
{
  // Check if sinusoidal (=> no start value needed = no integrity check)
  bool has_sinusoidal_modifier (const UNamedParameters* mod)
  {
    for (const UNamedParameters* i = mod; i; i = i->next)
      if (*i->name == "sin" || *i->name == "cos")
	return true;
    return false;
  }
}

//! UCommand subclass execution function
UCommand::Status
UCommand_ASSIGN_VALUE::execute_(UConnection *connection)
{
  // General initializations
  if (!variable)
  {
    variable = variablename->getVariable(this, connection);
    if (!variablename->getFullname())
      return UCOMPLETED;
    method = variablename->getMethod();
    devicename = variablename->getDevice();
  }
  ufloat currentTime = connection->server->lastTime();

  // Wait in queue if needed
  if (variable
      && variable->blendType == urbi::UQUEUE && variable->nbAverage > 0)
    return status;

  // Broadcasting
  if (scanGroups(&UCommand::refVarName, true))
    return UMORPH;

  // Function call
  // morph into the function code
  if (expression->type == UExpression::FUNCTION)
  {
    Status s = execute_function_call (connection);
    if (s != UFALLTHRU)
      return s;
  }

  ////////////////////////////////////////
  // Initialization phase (first pass)
  ////////////////////////////////////////

  if (status == UONQUEUE)
  {
    // object aliasing here
    if (variablename->nostruct
	&& expression->type == UExpression::VARIABLE
	&& expression->variablename
	&& expression->variablename->nostruct)
    {
      UString* objname = expression->variablename->id;
      if (libport::mhas(::urbiserver->getObjTab (), objname->c_str()))
      {
	// the use of 'id' is a hack that works.
	HMaliastab::iterator hmi =
	  ::urbiserver->getObjAliasTab ().find(variablename->id->c_str());
	if (hmi != ::urbiserver->getObjAliasTab ().end())
	  *hmi->second = objname;
	else
	{
	  UString* objalias = new UString(*variablename->method);
	  ::urbiserver->getObjAliasTab ()[objalias->c_str()] = new UString(*objname);
	}
	return UCOMPLETED;
      }
    }

    // Objects cannot be assigned
    if (variable
	&& !variablename->fromGroup
	&& variable->value->dataType == DATA_OBJ)
    {
      send_error(connection, this,
		 "Warning: %s type mismatch: no object assignment",
		 variablename->getFullname()->c_str());
      return UCOMPLETED;
    }

    // Strict variable definition checking
    if (!variable && connection->server->isDefChecking () && !defkey)
      send_error(connection, this, "Unknown identifier: %s",
		 variablename->getFullname()->c_str());

    // Check the +error flag
    errorFlag = false;
    for (UNamedParameters *param = flags; param; param = param->next)
      if (param->name
	  && *param->name == "flag"
	  && param->expression
	  && param->expression->val == 2) // 2 = +error
	errorFlag = true;

    // UCANCEL mode
    if (variable && variable->blendType == urbi::UCANCEL)
    {
      variable->nbAverage = 0;
      variable->cancel = this;
    }

    // eval the right side of the assignment and check for errors
    UValue* rhs = expression->eval(this, connection);
    if (!rhs)
      return UCOMPLETED;

    // Check type compatibility if the left side variable already exists
    if (variable &&
	variable->value->dataType != DATA_VOID &&
	rhs->dataType != variable->value->dataType)
    {
      if (::urbiserver->isDefChecking ())
      {
	send_error(connection, this, "Warning: %s type mismatch",
		   variablename->getFullname()->c_str());
	delete rhs;
	return UCOMPLETED;
      }

      delete variable;
      variable = 0;
    }

    switch (rhs->dataType)
    {
      case DATA_UNKNOWN:
      case DATA_FILE:
      case DATA_OBJ:
      case DATA_FUNCTION:
      case DATA_VARIABLE:
	// FIXME: Were implicitly ignored by the previous code, a list of if's.
	break;

      case DATA_STRING:
	// Handle String Composition
	if (parameters != 0)
	  // FIXME: 65000 is not dividable by 42 nor 51.
	  if (char *result =
	      static_cast<char*> (malloc (sizeof (char)
					  * (65000+rhs->str->size()))))
	  {
	    strcpy (result, rhs->str->c_str());

	    for (UNamedParameters* i = parameters; i; i = i->next)
	    {
	      UValue* modifier = i->expression->eval(this, connection);
	      if (!modifier)
	      {
		send_error(connection, this,
			   "String composition failed");
		delete rhs;
		return UCOMPLETED;
	      }

	      if (modifier->dataType == DATA_NUM)
	      {
		modifier->dataType = DATA_STRING;
		std::ostringstream ostr;
		ostr << modifier->val;
		modifier->str = new UString(ostr.str().c_str());
	      }

	      std::string n = std::string("$") + i->name->c_str();
	      if (!strstr(modifier->str->c_str(), n.c_str()))
		while (char* possub = strstr(result, n.c_str()))
		{
		  memmove(possub + modifier->str->size(),
			  possub + n.size(),
			  strlen(result)
			  - static_cast<int>(possub - result)
			  - n.size()+1);
		  strncpy (possub, modifier->str->c_str(), modifier->str->size());
		}

	      delete modifier;
	    }
	    *rhs->str = result;
	    free(result);
	  }

	// FIXME: Factor with the following cases.
	// Assignment
	if (variable) // the variable already exists
	  variable->set(rhs);
	else
	{
	  variable = new UVariable(variablename->getFullname()->c_str(),
				   rhs->copy());
	  if (!variable)
	    return UCOMPLETED;
	  connection->localVariableCheck(variable);
	  variable->updated();
	}

	delete rhs;
	return UCOMPLETED;

      case DATA_BINARY:
      case DATA_LIST:
      case DATA_VOID:
	// Assignment
	if (variable) // the variable already exists
	  variable->set(rhs);
	else
	{
	  variable = new UVariable(variablename->getFullname()->c_str(),
				   rhs->copy());
	  if (!variable)
	    return UCOMPLETED;
	  connection->localVariableCheck(variable);
	  variable->updated();
	}

	delete rhs;
	return UCOMPLETED;

      case DATA_NUM:
	controlled = false; // is a virtual "time:0" needed?
	targetval = rhs->val;

	// Handling normalized correction
	if (variable && variablename->isnormalized)
	{
	  if (variable->rangemin == -UINFINITY
	      || variable->rangemax ==  UINFINITY)
	  {
	    if (!variablename->fromGroup)
	      send_error(connection, this,
			 "Impossible to normalize:"
			 " no range defined for variable %s",
			 variablename->getFullname()->c_str());
	    delete rhs;
	    return UCOMPLETED;
	  }

	  if (targetval < 0)
	    targetval = 0;
	  if (targetval > 1)
	    targetval = 1;

	  targetval = variable->rangemin + targetval *
	    (variable->rangemax - variable->rangemin);
	}

	// Store init time
	starttime = currentTime;
	lastExec = starttime;

	// Handling FLAGS
	if (parameters)
	{
	  // Checking integrity (variable exists), if not sinusoidal
	  if (variable == 0 && !has_sinusoidal_modifier (parameters))
	  {
	    if (!variablename->fromGroup)
	      send_error(connection, this,
			 "Modificator error: %s unknown"
			 " (no start value)",
			 variablename->getFullname()->c_str());
	    delete rhs;
	    return UCOMPLETED;
	  }

	  speed = 0;

	  // Initialize modifiers
	  for (UNamedParameters* modif = parameters; modif; modif = modif->next)
	  {
	    if (!modif->expression || !modif->name)
	    {
	      send_error(connection, this, "Invalid modifier");
	      delete rhs;
	      return UCOMPLETED;
	    }

	    if (*modif->name == "sin")
	    {
	      modif_sin = modif->expression;
	      controlled = true;
	    }
	    else if (*modif->name == "cos")
	    {
	      modif_sin = modif->expression;
	      // FIXME: delete modif_phase before?
	      modif_phase = new UExpression(loc_,
					    UExpression::VALUE, PI/ufloat(2));
	      controlled = true;
	    }
	    else if (*modif->name == "ampli")
	    {
	      modif_ampli = modif->expression;
	    }
	    else if (*modif->name == "smooth")
	    {
	      modif_smooth = modif->expression;
	      controlled = true;
	    }
	    else if (*modif->name == "time")
	    {
	      modif_time = modif->expression;
	      controlled = true;
	    }
	    else if (*modif->name == "speed")
	    {
	      modif_speed = modif->expression;
	      controlled = true;
	    }
	    else if (*modif->name == "accel")
	    {
	      modif_accel = modif->expression;
	      controlled = true;
	    }
	    else if (*modif->name == "adaptive")
	    {
	      modif_adaptive = modif->expression;
	      controlled = true;
	    }
	    else if (*modif->name == "phase")
	    {
	      modif_phase = modif->expression;
	    }
	    else if (*modif->name == "getphase")
	    {
	      if (modif->expression->type != UExpression::VARIABLE)
	      {
		send_error(connection, this,
			   "a variable is expected for"
			   " the 'getphase' modifier");
		return UCOMPLETED;
	      }
	      modif_getphase = modif->expression->variablename;
	    }
	    else if (*modif->name == "timelimit")
	    {
	      UValue *modifier = modif->expression->eval(this, connection);
	      if (!modifier || modifier->dataType != DATA_NUM)
	      {
		send_error(connection, this,
			   "Invalid modifier value");
		delete modifier;
		delete rhs;
		return UCOMPLETED;
	      }
	      endtime = currentTime + modifier->val;
	      delete modifier;
	    }
	    else
	    {
	      send_error(connection, this,
			 "Unkown modifier name");
	      delete rhs;
	      return UCOMPLETED;
	    }
	  }
	}

	// create var if it does not already exist
	if (!variable)
	{
	  variable = new UVariable(variablename->getFullname()->c_str(),
				   rhs->copy());
	  if (!variable)
	    return UCOMPLETED;
	  connection->localVariableCheck(variable);
	}

	// correct the type of VOID variables (comming from a def)
	if (variable->value->dataType == DATA_VOID)
	  variable->value->dataType = DATA_NUM;

	// virtual "time:0" if no modifier specified (controlled == false)
	if (!controlled)
	  // no controlling modifier => time:0
	  // FIXME: delete modif_time before?
	  modif_time = new UExpression(loc(), UExpression::VALUE, ufloat(0));

	// clean the temporary rhs UValue
	delete rhs;

	// UDISCARD mode
	if (variable->blendType == urbi::UDISCARD &&
	    variable->nbAssigns > 0)
	  return UCOMPLETED;

	// init valarray for a "val" assignment
	ufloat *targetvalue = (!controlled
			       ? &variable->value->val // prevents a read access
			       : &variable->get()->val);

	if (variable->autoUpdate)
	  valtmp = targetvalue;	      // &variable->value->val
	else
	  valtmp = &(variable->target); // &variable->target

	++variable->nbAssigns;
	assigned = true;

      // use of previous value as a start value to ensure that the start value
      // will remain identical when several assignments are run during the same
      // cycle
      // old code: startval = *targetvalue;

      // the "fix" below is insane. I paste back the old code...
      //startval = variable->previous;
      if (variable->cycleBeginTime < currentTime)
      {
	variable->cyclevalue = *targetvalue;
	variable->cycleBeginTime = currentTime;
      }

      startval = variable->cyclevalue;

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
	return UCOMPLETED;

    // Cancel if needed
    if (variable->blendType == urbi::UCANCEL && variable->cancel != this)
      return UCOMPLETED;

    // Discard if needed
    if (variable->blendType == urbi::UDISCARD && variable->nbAverage > 0)
      return UCOMPLETED;

    // In normal mode, there is always only one value to consider
    if (variable->blendType == urbi::UNORMAL)
      variable->nbAverage = 0;

    // In add mode, the current value is always added
    if (variable->blendType == urbi::UADD && variable->nbAverage > 1)
      variable->nbAverage = 1;

    ///////////////////////////////
    // Process the active modifiers
    if (processModifiers(connection, currentTime) == UFAIL)
      return UCOMPLETED;
    lastExec = currentTime;
    ///////////////////////////////

    // absorb average and set reinit list to set
    // nbAverage back to 0 after work()
    if (variable->blendType != urbi::UADD)
      *valtmp = *valtmp / (ufloat)(variable->nbAverage+1);
    ++variable->nbAverage;

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

    if (variable->blendType != urbi::UMIX && variable->blendType != urbi::UADD)
      variable->selfSet(valtmp);

    first = false;

    if (finished)
      return variable->speedmax != UINFINITY ? URUNNING : UCOMPLETED;
    else
      return URUNNING;
  }
  return UCOMPLETED;
}

// Processing the modifiers in a URUNNING assignment
UErrorValue
UCommand_ASSIGN_VALUE::processModifiers(UConnection* connection,
					ufloat currentTime)
{
  ufloat deltaTime = connection->server->getFrequency();
  ufloat currentVal = controlled ? variable->get()->val : variable->value->val;

  if (frozen)
  {
    frozen = false;
    starttime += currentTime - lastExec;
  }

  // Adaptive mode? (only for "speed" and "time")
  bool adaptive = false;
  if (modif_adaptive)
    if (UValue* v = modif_adaptive->eval(this, connection))
    {
      adaptive = (v->val != 0);
      delete v;
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
    if (adaptive
	&& ABSF(currentVal - targetval) <= variable->delta)
    {
      finished = true;
      *valtmp = variable->nbAverage * *valtmp +
	targetval;
    }

    if (UValue* v = modif_time->eval(this, connection))
    {
      targettime = ABSF(v->val);
      //enforce speedmax here
      if (variable->speedmax != UINFINITY)
	if (targettime <  ABSF ((targetval-startval)/ variable->speedmax))
	{
	  if (errorFlag)
	    send_error(connection, this,
		       "Warning: request exceeds speedmax."
		       " Enforcing limitation");
	  targettime = ABSF ((targetval - startval)/ variable->speedmax);
	}
      delete v;
    }

    // check for speedmin
    if ((targettime > (currentTime - starttime)) &&
	(ABSF((targetval - currentVal) /
	      (targettime - (currentTime - starttime))) < speedmin))
    {
      targettime = currentTime - starttime +
	ABSF(targetval - currentVal)/ speedmin;

      if (errorFlag && first)
	send_error(connection, this, "low speed: increased to speedmin");
    }

    if (currentTime - starttime + deltaTime >= targettime)
    {
      if (!adaptive)
	finished = true;
      *valtmp = variable->nbAverage * *valtmp +	targetval;
    }
    else if (adaptive)
      *valtmp = variable->nbAverage * *valtmp +
	currentVal +
	deltaTime*
	((targetval - currentVal) /
	 (targettime - (currentTime - starttime)) );
    else
      *valtmp = variable->nbAverage * *valtmp +
	startval +
	(currentTime - starttime + deltaTime)*
	((targetval - startval) /
	 targettime );

    return USUCCESS;
  }

  // smooth
  if (modif_smooth)
  {
    if (UValue* v = modif_smooth->eval(this, connection))
    {
      targettime = ABSF(v->val);
      delete v;
    }

    // test for speedmin (with linear mvt approximation)
    if ((targettime > (currentTime - starttime)) &&
	(ABSF((targetval - currentVal) /
	      (targettime - (currentTime - starttime))) < speedmin))
    {
      targettime = currentTime - starttime +
	ABSF(targetval - currentVal)/speedmin;

      if (errorFlag && first)
	send_error(connection, this, "low speed: increased to speedmin");
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
	((targetval - startval) * 0.5 *
	 (ufloat(1)+sin(-(PI/ufloat(2))+ PI*(currentTime - starttime + deltaTime) /
			targettime
	   ))
	  );
    return USUCCESS;
  }

  //speed
  if (modif_speed)
  {
    if (adaptive && ABSF(currentVal - targetval) <= variable->delta)
    {
      finished = true;
      *valtmp = variable->nbAverage * *valtmp +
	targetval;
    }

    if (UValue* v = modif_speed->eval(this, connection))
    {
      speed = ABSF(v->val);
      delete v;
    }

    if (variablename->isnormalized)
      speed = speed * (variable->rangemax - variable->rangemin);

    if (speed != 0)
    {
      if (adaptive)
	targettime = currentTime - starttime +
	  ABSF(targetval - currentVal) / (speed/1000.);
      else
	targettime = ABSF(targetval - startval) / (speed/1000.);
    }
    else
      targettime= UINFINITY;

    // test for speedmin
    if ((targettime > (currentTime - starttime)) &&
	(ABSF((targetval - currentVal) /
	      (targettime - (currentTime - starttime))) < speedmin))
    {
      targettime = currentTime - starttime +
	ABSF(targetval - currentVal)/ speedmin;

      if (errorFlag && first)
	send_error(connection, this, "low speed: increased to speedmin");
    }

    if (currentTime - starttime + deltaTime >= targettime)
    {
      if (!adaptive)
	finished = true;
      *valtmp = variable->nbAverage * *valtmp +
	targetval;
    }
    else if (speed != 0)
    {
      if (adaptive)
	*valtmp = variable->nbAverage * *valtmp +
	  currentVal +
	  deltaTime*
	  ((targetval - currentVal) /
	   (targettime - (currentTime - starttime)) );
      else
	*valtmp = variable->nbAverage * *valtmp +
	  startval +
	  (currentTime - starttime + deltaTime)*
	  ((targetval - startval) /
	   targettime );
    }
    else
      *valtmp = variable->nbAverage * *valtmp + currentVal;

    return USUCCESS;
  }

  //accel
  if (modif_accel)
  {
    if (UValue* v = modif_accel->eval(this, connection))
    {
      accel = ABSF(v->val/1000.);
      delete v;
    }

    if (targetval < startval)
      accel = -accel;

    if (accel == 0)
      accel = 0.001;

    if (variablename->isnormalized)
      accel = accel * (variable->rangemax - variable->rangemin);

    targettime = sqrt (2 * ABSF(targetval - startval)
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
    if (UValue* v = modif_sin->eval(this, connection))
    {
      targettime = ABSF(v->val);
      delete v;
    }
    if (targettime == 0)
      targettime = 0.1;

    ufloat phase = 0;
    if (modif_phase)
      if (UValue* v = modif_phase->eval(this, connection))
      {
	phase = v->val;
	delete v;
      }

    ufloat amplitude = 0;
    if (modif_ampli)
      if (UValue* v = modif_ampli->eval(this, connection))
      {
	amplitude = v->val;
	delete v;
      }
    if (variablename->isnormalized)
      amplitude = amplitude * (variable->rangemax - variable->rangemin);

    if (expression)
      if (UValue* v = expression->eval(this, connection))
      {
	targetval = v->val;
	if (variablename->isnormalized)
	{
	  if (targetval < 0)
	    targetval = 0;
	  if (targetval > 1)
	    targetval = 1;

	  targetval = variable->rangemin + targetval *
	    (variable->rangemax - variable->rangemin);
	}
	delete v;
      }

    ufloat intermediary;
    intermediary = targetval +
      amplitude * sin(phase +
		      (PI*ufloat(2))*((currentTime - starttime + deltaTime) /
				      targettime ));
    if (modif_getphase)
    {
      UVariable *phasevari = modif_getphase->getVariable(this, connection);
      if (!phasevari)
      {
	if (!modif_getphase->getFullname())
	{
	  send_error(connection, this, "Invalid phase variable name");
	  return UFAIL;
	}
	phasevari = new UVariable(modif_getphase->getFullname()->c_str(),
				  ufloat(0));
	connection->localVariableCheck(phasevari);
      }

      UValue *phaseval = phasevari->value;

      phaseval->val = (phase +
		       (PI*ufloat(2))*((currentTime - starttime + deltaTime) /
				       targettime ));
      int n = static_cast<int>(phaseval->val / (PI*ufloat(2)));
      if (n < 0)
	--n;
      phaseval->val = phaseval->val - n * (PI * ufloat(2));
    }

    *valtmp = variable->nbAverage * *valtmp + intermediary;

    return USUCCESS;
  }
  return USUCCESS;
}


//! UCommand subclass hard copy function
UCommand*
UCommand_ASSIGN_VALUE::copy() const
{
  UCommand_ASSIGN_VALUE *ret =
    new UCommand_ASSIGN_VALUE(loc_, ucopy (variablename),
			      ucopy (expression),
			      ucopy (parameters));

  copybase(ret);
  ret->defkey = defkey;
  return ret;
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_ASSIGN_VALUE::print_(unsigned l) const
{
  DEBUG_ATTR (variablename);
  DEBUG_ATTR_I (expression);
  DEBUG_ATTR(parameters);
}

MEMORY_MANAGER_INIT(UCommand_ASSIGN_BINARY);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_ASSIGN_BINARY::UCommand_ASSIGN_BINARY(const location& l,
					       UVariableName *variablename,
					       libport::RefPt<UBinary> *refBinary)
  : UCommand(l, ASSIGN_BINARY),
    variablename (variablename),
    variable (0),
    refBinary (refBinary),
    method (0),
    devicename (0)
{
  ADDOBJ(UCommand_ASSIGN_BINARY);
}

//! UCommand subclass destructor.
UCommand_ASSIGN_BINARY::~UCommand_ASSIGN_BINARY()
{
  FREEOBJ(UCommand_ASSIGN_BINARY);
  delete variablename;
  LIBERATE(refBinary);
}

//! UCommand subclass execution function
UCommand::Status
UCommand_ASSIGN_BINARY::execute_(UConnection *connection)
{
  // General initializations
  if (!variable)
  {
    variable = variablename->getVariable(this, connection);
    if (!variablename->getFullname())
      return UCOMPLETED;
    method = variablename->getMethod();
    devicename = variablename->getDevice();
  }

  // Broadcasting
  if (scanGroups(&UCommand::refVarName, true)) return UMORPH;

  // Type checking
  UValue *value;
  if (variable
      && variable->value->dataType != DATA_BINARY
      && variable->value->dataType != DATA_VOID)
  {
    send_error(connection, this,
	       "%s type mismatch",
	       variablename->getFullname()->c_str());
    return UCOMPLETED;
  }

  // Create variable if it doesn't exist
  if (!variable)
  {
    value = new UValue();
    value->dataType = DATA_BINARY;
    variable = new UVariable(variablename->getFullname()->c_str(), value);
    if (!variable)
      return UCOMPLETED;
    variable->blendType = urbi::UQUEUE;

    connection->localVariableCheck(variable);
  }
  else if (variable->value->dataType == DATA_BINARY)
    LIBERATE(variable->value->refBinary);

  variable->value->dataType = DATA_BINARY;
  variable->value->refBinary = refBinary->copy();

  variable->updated();
  return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_ASSIGN_BINARY::copy() const
{
  return copybase(new UCommand_ASSIGN_BINARY(loc_, ucopy (variablename),
					     refBinary->copy()));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_ASSIGN_BINARY::print_(unsigned l) const
{
  DEBUG_ATTR (variablename);
  if (refBinary)
  {
    debug(l, "  Binary:");
    refBinary->ref()->print();
    debug("\n");
  }
}

MEMORY_MANAGER_INIT(UCommand_ASSIGN_PROPERTY);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_ASSIGN_PROPERTY::UCommand_ASSIGN_PROPERTY(const location& l,
						   UVariableName *variablename,
						   UString *oper,
						   UExpression *expression)
  : UCommand(l, ASSIGN_PROPERTY),
    variablename (variablename),
    variable (0),
    oper (oper),
    expression (expression),
    method (0),
    devicename (0)
{
  ADDOBJ(UCommand_ASSIGN_PROPERTY);
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
UCommand::Status
UCommand_ASSIGN_PROPERTY::execute_(UConnection *connection)
{
  UVariable* variable = variablename->getVariable(this, connection);
  if (!variablename->getFullname())
    return UCOMPLETED;
  variablename->getMethod();
  variablename->getDevice();

  // Broadcasting
  if (scanGroups(&UCommand::refVarName, true))
    return UMORPH;

  // variable existence checking
  if (!variable)
  {
    if (!variablename->fromGroup)
      send_error(connection, this,
		 "Variable %s does not exist",
		 variablename->getFullname()->c_str());
    return UCOMPLETED;
  }

  // Property handling
  // blend
  if (*oper == "blend")
  {
    UValue *blendmode = expression->eval(this, connection);
    if (blendmode == 0)
      return UCOMPLETED;

    if (blendmode->dataType != DATA_STRING)
    {
      send_error(connection, this,"Invalid blend mode.");
      return UCOMPLETED;
    }

    if (variable->value->dataType != DATA_NUM &&
	variable->value->dataType != DATA_BINARY)
    {
      send_error(connection, this,
		 "%s type is invalid for mixing",
		 variablename->getFullname()->c_str());
      return UCOMPLETED;
    }

    if (*blendmode->str == "normal")
      variable->blendType = urbi::UNORMAL;
    else if (*blendmode->str == "mix")
      variable->blendType = urbi::UMIX;
    else if (*blendmode->str == "add")
      variable->blendType = urbi::UADD;
    else if (*blendmode->str == "discard")
      variable->blendType = urbi::UDISCARD;
    else if (*blendmode->str == "queue")
      variable->blendType = urbi::UQUEUE;
    else if (*blendmode->str == "cancel")
      variable->blendType = urbi::UCANCEL;
    else
    {
      send_error(connection, this, "Unknown blend mode: %s",
		 blendmode->str->c_str());
      return UCOMPLETED;
    }

    return UCOMPLETED;
  }

  // rangemax
  if (*oper == "rangemax")
  {
    UValue *nb = expression->eval(this, connection);
    if (nb == 0)
      return UCOMPLETED;

    if (nb->dataType != DATA_NUM)
    {
      send_error(connection, this,
		 "Invalid range type. NUM expected.");
      return UCOMPLETED;
    }

    variable->rangemax = nb->val;
    return UCOMPLETED;
  }

  // delta
  if (*oper == "delta")
  {
    UValue *nb = expression->eval(this, connection);
    if (nb == 0)
      return UCOMPLETED;

    if (nb->dataType != DATA_NUM)
    {
      send_error(connection, this,
		 "Invalid delta type. NUM expected.");
      return UCOMPLETED;
    }

    variable->delta = nb->val;
    return UCOMPLETED;
  }


  // unit
  if (*oper == "unit")
  {
    UValue *unitval = expression->eval(this, connection);
    if (unitval == 0)
      return UCOMPLETED;

    if (unitval->dataType != DATA_STRING)
    {
      send_error(connection, this,
		 "Invalid unit type (must be a string).");
      return UCOMPLETED;
    }

    if (variable->value->dataType != DATA_NUM &&
	variable->value->dataType != DATA_BINARY)
    {
      send_error(connection, this,
		 "%s type is Invalid for unit attribution",
		 variablename->getFullname()->c_str());
      return UCOMPLETED;
    }

    variable->setUnit(unitval->str->c_str());

    return UCOMPLETED;
  }

  // rangemin
  if (*oper == "rangemin")
  {
    UValue *nb = expression->eval(this, connection);
    if (nb == 0)
      return UCOMPLETED;

    if (nb->dataType != DATA_NUM)
    {
      send_error(connection, this,
		 "Invalid range type. NUM expected.");
      return UCOMPLETED;
    }

    variable->rangemin = nb->val;
    return UCOMPLETED;
  }

  // speedmax
  if (*oper == "speedmax")
  {
    UValue *nb = expression->eval(this, connection);
    if (nb == 0)
      return UCOMPLETED;

    if (nb->dataType != DATA_NUM)
    {
      send_error(connection, this,
		 "Invalid speed type. NUM expected.");
      return UCOMPLETED;
    }

    variable->speedmax = nb->val;
    return UCOMPLETED;
  }

  // speedmin
  if (*oper == "speedmin")
  {
    UValue *nb = expression->eval(this, connection);
    if (nb == 0)
      return UCOMPLETED;

    if (nb->dataType != DATA_NUM)
    {
      send_error(connection, this,
		 "Invalid speed type. NUM expected.");
      return UCOMPLETED;
    }

    variable->speedmin = nb->val;
    return UCOMPLETED;
  }

  send_error(connection, this,
	     "Unknown property: %s", oper->c_str());
  return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_ASSIGN_PROPERTY::copy() const
{
  return copybase(new UCommand_ASSIGN_PROPERTY(loc_, ucopy (variablename),
					       ucopy (oper),
					       ucopy (expression)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_ASSIGN_PROPERTY::print_(unsigned l) const
{
  debug("[%s]:\n", oper->c_str());
  DEBUG_ATTR (variablename);
  DEBUG_ATTR_I (expression);
}

MEMORY_MANAGER_INIT(UCommand_AUTOASSIGN);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_AUTOASSIGN::UCommand_AUTOASSIGN(const location& l,
					 UVariableName* variablename,
					 UExpression* expression,
					 int assigntype)
  : UCommand(l, ASSIGN_VALUE),
    variablename (variablename),
    expression (expression),
    assigntype (assigntype)
{
  ADDOBJ(UCommand_AUTOASSIGN);
}

//! UCommand subclass destructor.
UCommand_AUTOASSIGN::~UCommand_AUTOASSIGN()
{
  FREEOBJ(UCommand_AUTOASSIGN);
  delete variablename;
  delete expression;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_AUTOASSIGN::execute_(UConnection*)
{
  if (!variablename || !expression)
    return UCOMPLETED;

  UExpression* extended_expression = 0;

  // FIXME: this is not safe, use better types.
  switch (assigntype)
  {
    case 0: /* += */
      extended_expression =
	new UExpression(loc(), UExpression::PLUS,
			new UExpression(loc(), UExpression::VARIABLE,
					variablename->copy()),
			expression->copy());
      break;
    case 1: /* -= */
      extended_expression =
	new UExpression(loc(), UExpression::MINUS,
			new UExpression(loc(), UExpression::VARIABLE,
					variablename->copy()),
			expression->copy());
      break;
  }

  if (!extended_expression)
    return UCOMPLETED;

  morph =
    new UCommand_ASSIGN_VALUE(loc_, variablename->copy(), extended_expression,
			      0, false);

  persistant = false;
  return UMORPH;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_AUTOASSIGN::copy() const
{
  return copybase(new UCommand_AUTOASSIGN(loc_, ucopy (variablename),
					  ucopy (expression),
					  assigntype));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_AUTOASSIGN::print_(unsigned l) const
{
  debug("(%d):", assigntype);
  DEBUG_ATTR (variablename);
  DEBUG_ATTR_I (expression);
}


MEMORY_MANAGER_INIT(UCommand_EXPR);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_EXPR::UCommand_EXPR(const location& l,
			     UExpression* expression)
  : UCommand(l, EXPR),
    expression (expression)
{
  ADDOBJ(UCommand_EXPR);
}

//! UCommand subclass destructor.
UCommand_EXPR::~UCommand_EXPR()
{
  FREEOBJ(UCommand_EXPR);
  delete expression;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_EXPR::execute_function_call(UConnection *connection)
{
  passert (expression->type, expression->type == UExpression::FUNCTION);

  // Execution & morphing
  UString* funname = expression->variablename->buildFullname(this, connection);
  if (!funname)
    return UCOMPLETED;

  // Broadcasting
  if (scanGroups(&UCommand::refVarName, false))
    return UMORPH;

  ////// INTERNAL /////

  ////// Native URBI: user-defined /////

  UFunction* fun = 0;
  HMfunctiontab::iterator hmf =
    ::urbiserver->getFunctionTab ().find(funname->c_str());
  if (hmf != ::urbiserver->getFunctionTab ().end())
    fun = hmf->second;
  else
  {
    //trying inheritance
    const char* devname = expression->variablename->getDevice()->c_str();
    HMobjtab::iterator itobj = ::urbiserver->getObjTab ().find(devname);
    if (itobj != ::urbiserver->getObjTab ().end())
    {
      bool ambiguous;
      fun = itobj->second->
	searchFunction(expression->variablename->getMethod()->c_str(),
		       ambiguous);

      // hack until we get proper nameresolution
      if (fun == kernel::remoteFunction)
	fun = 0;

      if (ambiguous)
      {
	send_error(connection, this,
		   "Ambiguous multiple inheritance"
		   " on function %s",
		   funname->c_str());
	return UCOMPLETED;
      }
    }
  }

  if (fun)
  {
    if ((expression->parameters
	 && fun->nbparam()
	 && expression->parameters->size() != fun->nbparam())
	|| (expression->parameters && !fun->nbparam())
	|| (!expression->parameters && fun->nbparam()))
    {
      send_error(connection, this,
		 "Invalid number of arguments for %s (should be %d)",
		 funname->c_str(), fun->nbparam());
      return UCOMPLETED;
    }

    persistant = false;
    UVariableName* resultContainer =
      new UVariableName(new UString("__UFnct"), new UString("__result__"),
			true, 0);

    UCommand_EXPR* cexp =
      new UCommand_EXPR(loc_, new UExpression(loc(), UExpression::VARIABLE,
					      resultContainer));

    cexp->setTag(this);
    morph = new UCommand_TREE(loc_, Flavorable::UPIPE,
			      fun->cmdcopy(getTag()), cexp);

    if (morph)
    {
      morph->morphed = true;
      morph->setTag(this);

      if (flags)
	morph->flags = flags->copy();

      UString* fundevice = expression->variablename->getDevice();
      if (!fundevice)
      {
	send_error(connection, this, "Function name evaluation failed");
	return UCOMPLETED;
      }

      UCommand_TREE* uc_tree = dynamic_cast<UCommand_TREE*> (morph);
      assert (uc_tree);

      // handle the :: case
      if (expression->variablename->doublecolon
	  && !connection->stack.empty ()
	  && libport::mhas(::urbiserver->getObjTab (),
			   connection->stack.front()->self().c_str()))
	*fundevice = connection->stack.front()->self();

      uc_tree->callid = new UCallid (unique ("__UFnct"),
                                     fundevice->c_str (), uc_tree);
      resultContainer->nameUpdate(uc_tree->callid->str(),
				  "__result__");
      // creates return variable
      uc_tree->callid->setReturnVar (
	new UVariable (uc_tree->callid->str().c_str(), "__result__",
		       new UValue ()));
      if (!uc_tree->callid)
	return UCOMPLETED;
      uc_tree->connection = connection;

      for (UNamedParameters *pvalue = expression->parameters,
	     *pname = fun->parameters;
	   pvalue != 0;
	   pvalue = pvalue->next, pname = pname->next)
	if (UValue* valparam = pvalue->expression->eval(this, connection))
	  uc_tree->callid->store(new UVariable(uc_tree->callid->str().c_str(),
					       pname->name->c_str(),
					       valparam));
	else
	{
	  send_error(connection, this, "EXPR evaluation failed");
	  return UCOMPLETED;
	}
    }
    return UMORPH;
  }
  else if (connection->receiving
	   && (*expression->variablename->id == "exec"
	       || *expression->variablename->id == "load"))
    // Some functions are executed at the same time as they are
    // received (e.g., ping).  For some reason, it is believed that
    // exec should not be executed asap.  For the same reasons, load
    // must not (otherwise several things do not work).
    //
    // JC thinks there is no reason to try to understand further:
    // this code is rewritten for k2.
    return URUNNING;

  // handle the :: case
  if (expression->variablename->doublecolon
      && !connection->stack.empty ()
      && libport::mhas(::urbiserver->getObjTab (),
		       connection->stack.front()->self().c_str()))
  {
    // rebuild name with parent class
    *expression->variablename->device = connection->stack.front()->self();
    expression->variablename->resetCache (); // this deletes funname pointeur
    funname = expression->variablename->buildFullname(this, connection);
    if (!funname)
      return UCOMPLETED;
  }

  ////// module-defined /////

  urbi::UTable::iterator hmfi = urbi::functionmap->find(funname->c_str());
  if (hmfi != urbi::functionmap->end())
  {
    for (std::list<urbi::UGenericCallback*>::iterator cbi =
	   hmfi->second.begin();
	 cbi != hmfi->second.end();
	 ++cbi)
    {
      if ((expression->parameters &&
	   expression->parameters->size() == (*cbi)->nbparam)
	  || (!expression->parameters && !(*cbi)->nbparam))
      {
	// here you could spawn a thread... if only Aperios knew how to!
	urbi::UList tmparray;
	for (UNamedParameters *pvalue = expression->parameters;
	     pvalue != 0;
	     pvalue = pvalue->next)
	  if (UValue* valparam = pvalue->expression->eval(this, connection))
	    // urbi::UValue do not see ::UValue,
	    // so it must be valparam who does the job.
	    tmparray.array.push_back(valparam->urbiValue());
	  else
	  {
	    send_error(connection, this, "EXPR evaluation failed");
	    return UCOMPLETED;
	  }

	UValue ret = (*cbi)->__evalcall(tmparray);
	if (ret.dataType != DATA_VOID)
	{
	  connection->sendPrefix(getTag().c_str());
	  ret.echo(connection);
	}
	if (ret.dataType != DATA_BINARY && ret.dataType != DATA_VOID)
	  connection->endline();
	else
	  connection->flush ();
	return UCOMPLETED;
      }
    }
    send_error(connection, this, "Invalid function call");
    return UCOMPLETED;
  }

  ////// EXTERNAL /////
  HMbindertab::iterator it =
    ::urbiserver->getFunctionBinderTab ().find(funname->c_str());
  if (it != ::urbiserver->getFunctionBinderTab ().end()
      && ((expression->parameters
	   && it->second->nbparam == expression->parameters->size())
	  || (!expression->parameters && it->second->nbparam == 0))
      && !it->second->monitors.empty())
  {
    std::string uid = unique ("__UFnctret.EXTERNAL_");
    {
      std::ostringstream o;
      o << "[0,\"" << funname->c_str() << "__" << it->second->nbparam
	<< "\",\"" << uid << "\"";
      const std::string n = o.str();
      for (std::list<UMonitor*>::iterator j = it->second->monitors.begin();
	   j != it->second->monitors.end();
	   ++j)
      {
	(*j)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
	(*j)->c->sendc(reinterpret_cast<const ubyte*>(n.c_str()), n.size());
	for (UNamedParameters *pvalue = expression->parameters;
	     pvalue != 0;
	     pvalue = pvalue->next)
	{
	  (*j)->c->sendc(reinterpret_cast<const ubyte*>(","), 1);
	  UValue* valparam = pvalue->expression->eval(this, connection);
	  valparam->echo((*j)->c);
	}
	(*j)->c->send(reinterpret_cast<const ubyte*>("]\n"), 2);
      }
    }
    persistant = false;
    {
      std::ostringstream o;
      o << "{waituntil(isdef(" << uid << "))|"
	<< getTag().c_str() << ":" << uid
	<< "|delete " << uid << "}";
      strMorph (o.str());
    }
    return UMORPH;
  }
  return UFALLTHRU;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_EXPR::execute_(UConnection *connection)
{
  if (expression->type == UExpression::FUNCTION)
  {
    Status s = execute_function_call(connection);
    if (s != UFALLTHRU)
      return s;
  }

  // Normal expression (no function)
  UValue* ret = expression->eval(this, connection);
  if (!ret)
  {
    send_error(connection, this, "EXPR evaluation failed");
    return UCOMPLETED;
  }

  // Expression morphing (currently used for load only)
  if (morph)
  {
    delete ret;
    return UMORPH;
  }

#if 1
  // "Display" the result.
  if (ret->dataType != DATA_VOID)
  {
    connection->sendPrefix(getTag().c_str());
    ret->echo(connection);
  }
  if (ret->dataType != DATA_BINARY && ret->dataType != DATA_VOID)
    connection->endline();
  else
    connection->flush ();
#else
  // "Display" the result.
  if (ret->dataType != DATA_VOID)
    connection << prefix(getTag()) << *ret;
  if (ret->dataType != DATA_BINARY && ret->dataType != DATA_VOID)
    connection << std::endl;
  else
    connection << std::flush;
#endif

  delete ret;
  return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_EXPR::copy() const
{
  return copybase(new UCommand_EXPR(loc_, ucopy (expression)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_EXPR::print_(unsigned l) const
{
  DEBUG_ATTR_I (expression);
}

MEMORY_MANAGER_INIT(UCommand_RETURN);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_RETURN::UCommand_RETURN(const location& l,
				 UExpression* expression)
  : UCommand(l, RETURN),
    expression (expression)
{
  ADDOBJ(UCommand_RETURN);
}

//! UCommand subclass destructor.
UCommand_RETURN::~UCommand_RETURN()
{
  FREEOBJ(UCommand_RETURN);
  delete expression;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_RETURN::execute_(UConnection *connection)
{
  if (!connection->stack.empty())
  {
    connection->returnMode = true;
    if (expression)
    {
      UValue *value = expression->eval(this, connection);
      if (!value)
	send_error(connection, this, "EXPR evaluation failed");
      else if (value->dataType == DATA_OBJ)
	send_error(connection, this, "Functions cannot return objects"
		   " with Kernel 1");
      else
	// FIXME: Why don't we use setReturnVar that calls "store"?
	ASSERT (connection->stack.front()->returnVar)
	  connection->stack.front()->returnVar->value = value;
    }
  }
  return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_RETURN::copy() const
{
  return copybase(new UCommand_RETURN(loc_, ucopy (expression)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_RETURN::print_(unsigned l) const
{
  DEBUG_ATTR_I (expression);
}

MEMORY_MANAGER_INIT(UCommand_ECHO);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_ECHO::UCommand_ECHO(const location& l,
			     UExpression* expression,
			     UNamedParameters *parameters,
			     UString *connectionTag)
  : UCommand(l, ECHO),
    expression (expression),
    parameters (parameters),
    connectionTag (connectionTag)
{
  ADDOBJ(UCommand_ECHO);
}

//! UCommand subclass destructor.
UCommand_ECHO::~UCommand_ECHO()
{
  FREEOBJ(UCommand_ECHO);
  delete expression;
  delete parameters;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_ECHO::execute_(UConnection *connection)
{
  UValue* ret = expression->eval(this, connection);

  if (!ret)
  {
    send_error(connection, this, "EXPR evaluation failed");
    return UCOMPLETED;
  }

  for (UNamedParameters *param = parameters; param; param = param->next)
    if (*param->name == "connection")
    {
      UValue *e1 = param->expression->eval(this, connection);
      if (e1 && e1->dataType == DATA_STRING)
	connectionTag = new UString(*e1->str);
      delete e1;
    }

  if (!connectionTag)
  {
    connection->sendc("*** ", getTag().c_str());
    ret->echo(connection, true);
    connection->endline();
  }
  else
  {
    bool ok = false;

    // Scan currently opened connections to locate the connection with the
    // appropriate tag (connectionTag)
    for (std::list<UConnection*>::iterator i =
	   connection->server->connectionList.begin();
	 i != connection->server->connectionList.end();
	 ++i)
      if ((*i)->isActive()
	  && (*(*i)->connectionTag == *connectionTag
	      || *connectionTag == "all"
	      || (!(*(*i)->connectionTag == *connection->connectionTag)
		  && *connectionTag == "other")))
      {
	ok = true;
	(*i)->sendc("*** ", getTag().c_str());
	ret->echo((*i), true);
	(*i)->endline();
      }

    if (!ok)
      send_error(connection, this,
		 "%s: no such connection", connectionTag->c_str());
  }

  delete ret;
  return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_ECHO::copy() const
{
  return copybase(new UCommand_ECHO(loc_, ucopy (expression),
				    ucopy (parameters),
				    ucopy (connectionTag)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_ECHO::print_(unsigned l) const
{
  DEBUG_ATTR_I (expression);
  DEBUG_ATTR(parameters);
}

MEMORY_MANAGER_INIT(UCommand_NEW);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_NEW::UCommand_NEW(const location& l,
			   UVariableName* varname,
			   UString* obj,
			   UNamedParameters *parameters,
			   bool noinit)
  : UCommand(l, NEW),
    id (0),
    varname (varname),
    obj (obj),
    parameters (parameters),
    noinit (noinit),
    remoteNew (false),
    sysCall (false)
{
  ADDOBJ(UCommand_NEW);
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
UCommand::Status
UCommand_NEW::execute_(UConnection *connection)
{
  if (remoteNew &&
      libport::mhas(::urbiserver->getObjWaitTab (), id->c_str()))
    return URUNNING;
  morph = 0;
  if (!id)
  {
    // init id
    UString* name = varname->buildFullname(this, connection, false);
    if (!varname->nostruct)
    {
      send_error(connection, this,
		 "Object names cannot be nested in Kernel 1");
      return UCOMPLETED;
    }
    if (!name)
    {
      send_error(connection, this, "Invalid object name");
      return UCOMPLETED;
    }
    id = new UString(name->c_str());
  }

  if (!id || !obj)
    return UCOMPLETED;

  if (!remoteNew && !sysCall)
  {
    if (*id == *obj)
    {
      send_error(connection, this,
		 "Object %s cannot new itself", obj->c_str());
      return UCOMPLETED;
    }

    if (libport::mhas(::urbiserver->getObjTab (), id->c_str()))
    {
      send_error(connection, this,
		 "Object %s already exists. Delete it first.",
		 id->c_str());
      return UCOMPLETED;
    }
  }

  HMobjtab::iterator objit = ::urbiserver->getObjTab ().find(obj->c_str());
  if (objit == ::urbiserver->getObjTab ().end()
      && !remoteNew)
  {
    const char* objname = obj->c_str();
    while (libport::mhas(::urbiserver->getObjAliasTab (), objname))
      objname = ::urbiserver->getObjAliasTab ()[objname]->c_str();

    objit = ::urbiserver->getObjTab ().find(objname);
    if (objit == ::urbiserver->getObjTab ().end())
    {
      int timeout = -1;

      if (!sysCall)
      {
	std::list<urbi::USystem*>& tmp_list =
	  ::urbiserver->systemObjects[urbi::NEW_CHANNEL];

	for (std::list<urbi::USystem*>::iterator it =
	       tmp_list.begin ();
	     it != tmp_list.end ();
	     ++it)
	{
	  int timeout_tmp = (*it)->receive_message (urbi::NEW_CHANNEL,
						    urbi::UStringSystemMessage
						    (objname));
	  if (timeout_tmp > timeout)
	    timeout = timeout_tmp;
	}
      }

      if (timeout < 0)
      {
	if (sysCall)
	  send_error(connection, this,
		     "Autoload timeout for object %s", objname);

	send_error(connection, this,
		   "Unknown object %s", obj->c_str());
	return UCOMPLETED;
      }
      else
      {
	sysCall = true;

	persistant = true;
	std::ostringstream oss;
	oss << "{ timeout (" << timeout
	    << ") waituntil(isdef(" << objname << "))"
	    << "}";

	strMorph (oss.str());
	morph = new UCommand_TREE(loc_, Flavorable::UPIPE, morph, this);
	return status;
      }
    }
  }

  // EXTERNAL
  if (objit->second->binder && !remoteNew)
  {
    std::ostringstream o;
    o << "[4,\"" << id->c_str() << "\",\""
      << objit->second->device->c_str() << "\"]\n";
    const std::string n = o.str();
    int nb=0;
    for (std::list<UMonitor*>::iterator i =
	   objit->second->binder->monitors.begin();
	 i != objit->second->binder->monitors.end();
	 ++i)
    {
      (*i)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
      (*i)->c->send(reinterpret_cast<const ubyte*>(n.c_str()), n.size());
      ++nb;
    }
    // Wait for remote new
    HMobjWaiting::iterator ow = ::urbiserver->getObjWaitTab ().find(id->c_str());
    if (ow != ::urbiserver->getObjWaitTab ().end())
      ow->second->nb += nb;
    else
    {
      UWaitCounter *wc = new UWaitCounter(*id, nb);
      ASSERT(wc)
	::urbiserver->getObjWaitTab () [wc->id.c_str()] = wc;
    }
    // initiate remote new waiting
    remoteNew = true;
    return URUNNING;
  }

  if (objit->second->internalBinder)
    objit->second->internalBinder->copy(std::string(id->c_str()));

  UObj* newobj = libport::find0(::urbiserver->getObjTab (), id->c_str());
  bool creation = false;
  if (!newobj)
  {
    newobj = new UObj(id);
    creation = true;
  }

  if (libport::has(newobj->up, objit->second))
  {
    send_error(connection, this,
	       "%s has already inherited from %s",
	       id->c_str(), obj->c_str());
    if (creation)
      delete newobj;
    return UCOMPLETED;
  }

  newobj->up.push_back(objit->second);
  objit->second->down.push_back(newobj);

  // Here the policy of "a = new b" which does not call init could be
  // enforced with 'noinit'
  // Currently, after discussions, all call to new tries to start init.
  // if (!parameters && noinit) return UCOMPLETED;

  // init constructor call
  //
  // For the moment, multiple inheritance with multiple constructors
  // will not be accepted. However, in principle there is no ambiguity since
  // we have a clear reference to the inherited object in this case. It will
  // be fixed later.

  persistant = false;
  std::string uid = unique ("__UInitret.tmpval_");

  std::ostringstream oss;
  oss << "{ ";

  bool component = false;
  UFunction* initfun = 0;
  // wait for init created if external component
  if (objit->second->binder || objit->second->internalBinder)
  {
    oss << "waituntil(isdef(" << id->c_str() << ".init)) | ";
    component = true;
  }
  else
  {
    // detects if init exists for the object or somewhere in the hierarchy
    bool ambiguous;
    initfun = newobj->searchFunction("init", ambiguous);

    //hack until we get proper nameresolution
    if (initfun == kernel::remoteFunction)
      initfun = 0;
  }

  if (parameters || initfun != 0 || component)
  {
    oss << uid << "=" << id->c_str() << ".init(";

    for (UNamedParameters *pvalue = parameters;
	 pvalue != 0;
	 pvalue = pvalue->next)
    {
      UValue* valparam = pvalue->expression->eval(this, connection);
      if (!valparam)
      {
	send_error(connection, this, "EXPR evaluation failed");
	std::ostringstream o;
	o << "{delete " << id->c_str() << "}";
	strMorph (o.str());
	return UMORPH;
      }

      oss << valparam->echo();
      if (pvalue->next)
	oss << ",";
    }

    oss << ") | if (!isdef("
	<< uid << ") || ((" << uid << "!=0) && (!isvoid("
	<< uid << ")))) { "
	<< "echo \"Error: Constructor failed, object deleted\";"
	<< " delete "
	<< id->c_str() << "} | if (isdef("
	<< uid << ")) delete " << uid
	<< "}";
  }
  else
    oss << "noop }";
  ECHO(oss.str());
  strMorph (oss.str());
  return UMORPH;
}


//! UCommand subclass hard copy function
UCommand*
UCommand_NEW::copy() const
{
  return copybase(new UCommand_NEW(loc_, ucopy (varname),
				   ucopy (obj),
				   ucopy (parameters)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_NEW::print_(unsigned l) const
{
  if (id)
    debug(l, "  Id:[%s]\n", id->c_str());
  if (obj)
    debug(l, "  Obj:[%s]\n", obj->c_str());
  DEBUG_ATTR(parameters);
}


MEMORY_MANAGER_INIT(UCommand_ALIAS);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_ALIAS::UCommand_ALIAS(const location& l,
			       UVariableName* aliasname,
			       UVariableName* id,
			       bool eraseit)
  : UCommand(l, ALIAS),
    aliasname (aliasname),
    id (id),
    eraseit (eraseit)
{
  ADDOBJ(UCommand_ALIAS);
}

//! UCommand subclass destructor.
UCommand_ALIAS::~UCommand_ALIAS()
{
  FREEOBJ(UCommand_ALIAS);
  delete id;
  delete aliasname;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_ALIAS::execute_(UConnection *connection)
{
  //alias setting
  if (aliasname && id)
  {
    UString *id0 = aliasname->buildFullname(this, connection, false);
    UString *id1 = id->buildFullname(this, connection, false);
    if (id0 && id1
	&& !connection->server->addAlias(id0->c_str(), id1->c_str()))
    {
      send_error(connection, this,
		 "Circular alias detected, abort command.");
      return UCOMPLETED;
    }
    return UCOMPLETED;
  }

  // full alias query
  if (!aliasname && !id)
  {
    for (HMaliastab::iterator i =
	   connection->server->getAliasTab ().begin();
	 i != connection->server->getAliasTab ().end();
	 ++i)
      connection->sendf(getTag(),
			"*** %25s -> %s\n",
			i->first, i->second->c_str());

    return UCOMPLETED;
  }

  // specific alias query
  if (aliasname && !id && !eraseit)
  {
    UString *id0 = aliasname->buildFullname(this, connection, false);
    HMaliastab::iterator i =
      connection->server->getAliasTab ().find(id0->c_str());
    if (i != connection->server->getAliasTab ().end())
    {
      connection->sendf (getTag(), "*** %25s -> %s\n",
			 i->first, i->second->c_str());
    }
    return UCOMPLETED;
  }

  // unalias query
  if (aliasname && !id && eraseit)
  {
    UString *id0 = aliasname->buildFullname(this, connection, false);
    HMaliastab::iterator i =
      connection->server->getAliasTab ().find(id0->c_str());
    if (i != connection->server->getAliasTab ().end())
      connection->server->getAliasTab ().erase(i);

    return UCOMPLETED;
  }
  return UCOMPLETED;
}


//! UCommand subclass hard copy function
UCommand*
UCommand_ALIAS::copy() const
{
  return copybase(new UCommand_ALIAS(loc_, ucopy (aliasname),
				     ucopy (id),
				     eraseit));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_ALIAS::print_(unsigned l) const
{
  debug("(%d) :\n", (int)eraseit);
  DEBUG_ATTR (aliasname);
  if (id)
  {
    debug(l+2, "id:");
    id->print();
    debug("\n");
  }
}

MEMORY_MANAGER_INIT(UCommand_INHERIT);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_INHERIT::UCommand_INHERIT(const location& l,
				   UVariableName* subclass,
				   UVariableName* theclass,
				   bool eraseit) :
  UCommand(l, INHERIT),
  subclass (subclass),
  theclass (theclass),
  eraseit (eraseit)
{
  ADDOBJ(UCommand_INHERIT);
}

//! UCommand subclass destructor.
UCommand_INHERIT::~UCommand_INHERIT()
{
  FREEOBJ(UCommand_INHERIT);
  delete subclass;
  delete theclass;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_INHERIT::execute_(UConnection *connection)
{
  UString *sub	  = subclass->buildFullname(this, connection, false);
  UString *parent = theclass->buildFullname(this, connection, false);

  if (!sub || !parent)
    return UCOMPLETED;

  HMobjtab::iterator objsub = ::urbiserver->getObjTab ().find(sub->c_str());
  if (objsub == ::urbiserver->getObjTab ().end ())
  {
    send_error(connection, this, "Object does not exist: %s", sub->c_str());
    return UCOMPLETED;
  }
  HMobjtab::iterator objparent = ::urbiserver->getObjTab ().find(parent->c_str());
  if (objparent == ::urbiserver->getObjTab ().end ())
  {
    send_error(connection, this, "Object does not exist: %s", parent->c_str());
    return UCOMPLETED;
  }

  if (!eraseit)
  {
    if (libport::has(objsub->second->up, objparent->second))
    {
      send_error(connection, this, "%s has already inherited from %s",
		 sub->c_str(), parent->c_str());
      return UCOMPLETED;
    }

    objsub->second->up.push_back(objparent->second);
    objparent->second->down.push_back(objsub->second);
  }
  else
  {
    if (!libport::has(objsub->second->up, objparent->second))
    {
      send_error(connection, this, "%s does not inherit from %s",
		 sub->c_str(), parent->c_str());
      return UCOMPLETED;
    }

    //clean
    objsub->second->up.remove(objparent->second);
    objparent->second->down.remove(objsub->second);
  }

  return UCOMPLETED;
}


//! UCommand subclass hard copy function
UCommand*
UCommand_INHERIT::copy() const
{
  return copybase(new UCommand_INHERIT(loc_, ucopy (subclass),
				       ucopy (theclass),
				       eraseit));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_INHERIT::print_(unsigned l) const
{
  debug("(%d) :\n", (int)eraseit);
  DEBUG_ATTR (subclass);
  DEBUG_ATTR (theclass);
}

MEMORY_MANAGER_INIT(UCommand_GROUP);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_GROUP::UCommand_GROUP(const location& l,
			       UString* id,
			       UNamedParameters* parameters,
			       int grouptype) :
  UCommand(l, GROUP),
  id (id),
  parameters (parameters),
  grouptype (grouptype)
{
  ADDOBJ(UCommand_GROUP);
}

//! UCommand subclass destructor.
UCommand_GROUP::~UCommand_GROUP()
{
  FREEOBJ(UCommand_GROUP);
  delete id;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_GROUP::execute_(UConnection *connection)
{
  if (parameters)
  {
    HMgrouptab::iterator hma = ::urbiserver->getGroupTab ().find(id->c_str());
    UGroup *g;
    if (hma != ::urbiserver->getGroupTab ().end())
      g = hma->second;
    else
    {
      g = new UGroup(*id);
      ::urbiserver->getGroupTab ()[g->name.c_str()] = g;
    }
    if (!grouptype)
      g->members.clear();

    for (UNamedParameters* param = parameters; param; param = param->next)
      if (grouptype == 2)
      {
	// del
	for (std::list<UString*>::iterator it = g->members.begin();
	     it != g->members.end(); )
	  if (**it == *param->name)
	    it =g->members.erase(it);
	  else
	    ++it;
      }
      else
      {
	const char* objname = param->name->c_str();
	while (libport::mhas(::urbiserver->getObjAliasTab (), objname))
	  objname = ::urbiserver->getObjAliasTab ()[objname]->c_str();

	g->members.push_back(new UString(objname));
      }

    return UCOMPLETED;
  }

  // full query
  if (!id)
  {
    for (HMgrouptab::iterator i = connection->server->getGroupTab ().begin();
	 i != connection->server->getGroupTab ().end();
	 ++i)
    {
      std::ostringstream o;
      o << "*** " << i->first << " = {";

      for (std::list<UString*>::iterator it = i->second->members.begin();
	   it !=  i->second->members.end(); )
      {
	o << (*it)->c_str();
	++it;
	if (it != i->second->members.end())
	  o << ',';
      }
      o << "}\n";
      connection->sendf(getTag(), o.str().c_str());
    }
    return UCOMPLETED;
  }

  // specific query
  HMgrouptab::iterator i = connection->server->getGroupTab ().find(id->c_str());
  if (i !=  connection->server->getGroupTab ().end())
  {
    UNamedParameters *ret = 0;

    std::list<UString*>::iterator it = i->second->members.begin();
    if (it != i->second->members.end())
    {
      ret = new UNamedParameters(new UExpression(loc(), UExpression::VALUE,
						 (*it)->copy()),
				 0);
      ++it;
    }

    for (; it != i->second->members.end(); ++it)
      ret = new UNamedParameters(new UExpression(loc(), UExpression::VALUE,
						 (*it)->copy()), ret);

    morph = new UCommand_EXPR(loc_, new UExpression(loc(),
						    UExpression::LIST, ret));

    persistant = false;
    return UMORPH;
  }

  return UCOMPLETED;
}


//! UCommand subclass hard copy function
UCommand*
UCommand_GROUP::copy() const
{
  return copybase(new UCommand_GROUP(loc_, ucopy (id),
				     ucopy (parameters),
				     grouptype));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_GROUP::print_(unsigned l) const
{
  if (id)
    debug(l, "  Id:[%s]\n", id->c_str());
}


/*-----------------------.
| UCommand_OPERATOR_ID.  |
`-----------------------*/

MEMORY_MANAGER_INIT(UCommand_OPERATOR_ID);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_OPERATOR_ID::UCommand_OPERATOR_ID(const location& l,
					   UString* oper,
					   UString* id)
  : UCommand(l, GENERIC),
    oper (oper),
    id (id)
{
  ADDOBJ(UCommand_OPERATOR_ID);
}

//! UCommand subclass destructor.
UCommand_OPERATOR_ID::~UCommand_OPERATOR_ID()
{
  FREEOBJ(UCommand_OPERATOR_ID);
  delete oper;
  delete id;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_OPERATOR_ID::execute_(UConnection *connection)
{
  if (*oper == "stop")
  {
    if (status == URUNNING)
      return UCOMPLETED;
    connection->server->mark(id);
    connection->server->hasSomethingToDelete ();
    return URUNNING;
  }
  else if (*oper == "killall")
  {
    bool ok = false;

    // Scan currently opened connections to locate the connection with the
    // appropriate tag (connectionTag)
    for (std::list<UConnection*>::iterator i =
	   connection->server->connectionList.begin();
	 i != connection->server->connectionList.end();
	 ++i)
      if ((*i)->isActive() &&
	  *(*i)->connectionTag == *id)
      {
	ok = true;
	(*i)->killall = true;
      }

    if (!ok)
    {
      send_error(connection, this,
		 "%s: no such connection", id->c_str());
      return UCOMPLETED;
    }
    return UCOMPLETED;
  }
  else if (*oper == "disconnect")
  {
    bool ok = false;
    // Scan currently opened connections to locate the connection with the
    // appropriate tag (connectionTag)
    for (std::list<UConnection*>::iterator i =
	   connection->server->connectionList.begin();
	 i != connection->server->connectionList.end();
	 ++i)
      if ((*i)->isActive() && *(*i)->connectionTag == *id)
      {
	ok = true;
	(*i)->disactivate();
	(*i)->closeConnection();
      }

    if (!ok)
    {
      send_error(connection, this,
		 "%s: no such connection", id->c_str());
      return UCOMPLETED;
    }
    return UCOMPLETED;
  }
  else if (*oper == "block")
  {
    if (status == URUNNING)
      return UCOMPLETED;

    if (*id == UNKNOWN_TAG)
      send_error(connection, this, "cannot block 'notag'");
    else
      connection->server->block(id->c_str());

    return URUNNING;
  }
  else if (*oper == "unblock")
  {
    connection->server->unblock(id->c_str());
    return UCOMPLETED;
  }
  else if (*oper == "freeze")
  {
    if (status == URUNNING)
      return UCOMPLETED;

    if (*id == UNKNOWN_TAG)
      send_error(connection, this, "cannot freeze 'notag'");
    else
      connection->server->freeze(id->c_str());

    return URUNNING;
  }
  else if (*oper == "unfreeze")
  {
    connection->server->unfreeze(id->c_str());
    return UCOMPLETED;
  }

  return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_OPERATOR_ID::copy() const
{
  return copybase(new UCommand_OPERATOR_ID(loc_, ucopy (oper),
					   ucopy (id)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_OPERATOR_ID::print_(unsigned l) const
{
  debug("%s:\n", oper->c_str());
  if (id)
    debug(l, "  Id:[%s]\n", id->c_str());
}

MEMORY_MANAGER_INIT(UCommand_DEVICE_CMD);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_DEVICE_CMD::UCommand_DEVICE_CMD(const location& l,
					 UVariableName* device,
					 ufloat *cmd)
  : UCommand(l, GENERIC),
    variablename (device),
    cmd (*cmd)
{
  ADDOBJ(UCommand_DEVICE_CMD);
}

//! UCommand subclass destructor.
UCommand_DEVICE_CMD::~UCommand_DEVICE_CMD()
{
  FREEOBJ(UCommand_DEVICE_CMD);
  delete variablename;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_DEVICE_CMD::execute_(UConnection *connection)
{
  if (!variablename)
    return UCOMPLETED;
  variablename->buildFullname(this, connection);

  if (variablename->nostruct)
  {
    UVariableName* recreatevar =
      new UVariableName(variablename->getMethod()->copy(),
			new UString("load"),
			variablename->rooted,
			0);
    delete variablename;
    variablename = recreatevar;
    variablename->buildFullname(this, connection);
  }

  // broadcasting
  if (scanGroups(&UCommand::refVarName, true))
    return UMORPH;

  // Main execution
  if (cmd == -1)
    morph =
      new UCommand_ASSIGN_VALUE
      (loc_,
       variablename->copy(),
       new UExpression(loc(), UExpression::MINUS,
		       new UExpression(loc(), UExpression::VALUE, ufloat(1)),
		       new UExpression(loc(), UExpression::VARIABLE, variablename->copy())),
       0,
       false);
  else
    morph =
      new UCommand_ASSIGN_VALUE(loc_,
				variablename->copy(),
				new UExpression(loc(), UExpression::VALUE,
						ufloat(cmd)),
				0,
				false);

  persistant = false;
  return UMORPH;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_DEVICE_CMD::copy() const
{
  return copybase(new UCommand_DEVICE_CMD(loc_, ucopy (variablename),
					  new ufloat(cmd)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_DEVICE_CMD::print_(unsigned l) const
{
  debug("%s:\n", variablename->device->c_str());
  if (cmd)
    debug(l, "  Cmd:[%f]\n", cmd);
}

MEMORY_MANAGER_INIT(UCommand_OPERATOR_VAR);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_OPERATOR_VAR::UCommand_OPERATOR_VAR(const location& l,
					     UString* oper,
					     UVariableName* variablename)
  : UCommand(l, GENERIC),
    oper (oper),
    variablename (variablename)
{
  ADDOBJ(UCommand_OPERATOR_VAR);
}

//! UCommand subclass destructor.
UCommand_OPERATOR_VAR::~UCommand_OPERATOR_VAR()
{
  FREEOBJ(UCommand_OPERATOR_VAR);
  delete oper;
  delete variablename;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_OPERATOR_VAR::execute_(UConnection *connection)
{
  UString *fullname = variablename->buildFullname(this, connection);
  if (!fullname)
    return UCOMPLETED;

  if (*oper == "undef" || *oper == "delete")
  {
    if (status != URUNNING)
    {
      variable = 0;
      fun = variablename->getFunction(this, connection);
      if (!fun)
      {
	variable = variablename->getVariable(this, connection);
	if (!variable && variablename->nostruct)
	{
	  UString* objname = variablename->getMethod();
	  if (libport::mhas(::urbiserver->getVariableTab (), objname->c_str()))
	    variable = ::urbiserver->getVariableTab ()[objname->c_str()];
	}
      }

      if (!fun && !variable)
      {
	send_error(connection, this,
		   "identifier %s does not exist",
		   fullname->c_str());
	return UCOMPLETED;
      }
    }

    if (variable)
    {
      // undef variable
      if (variable->toDelete)
      {
	delete variable;
	return UCOMPLETED;
      }

      // test if variable is an object with subclasses (and reject if yes)
      if (variable->value &&
	  variable->value->dataType == DATA_OBJ &&
	  variable->value->str)
      {
	HMobjtab::iterator idit =
	  ::urbiserver->getObjTab ().find(variable->value->str->c_str());
	if (idit != ::urbiserver->getObjTab ().end()
	    && !idit->second->down.empty())
	{
	  send_error(connection, this,
		     "This object has subclasses."
		     " Delete subclasses first.");
	  return UCOMPLETED;
	}
      }

      // variable is not an object or it does not have subclasses
      if (variable->nbAssigns == 0 && variable->uservar
	  && variable->useCpt == 0)
      {
	variable->toDelete = true;
	return URUNNING;
      }
      else
      {
	if (variable->useCpt)
	  send_error(connection, this,
		     "variable %s attached to a UVar, free it first",
		     fullname->c_str());
	else
	  send_error(connection, this,
		     "variable %s already in use or is a system var."
		     " Cannot delete.",
		     fullname->c_str());
	return UCOMPLETED;
      }
    }

    if (fun)
    {
      // undef function
      connection->server->getFunctionTab ().erase(
	connection->server->getFunctionTab ().find(fullname->c_str()));
      connection->server->getFunctionDefTab ().erase(
	connection->server->getFunctionDefTab ().find(fullname->c_str()));

      delete fun;
      return UCOMPLETED;
    }

    return UCOMPLETED;
  }

  //FIXME: Either remove or fix this code.
#if 0
  if (*oper == "info")
  {
    variable = variablename->getVariable(this,connection);
    if (!variablename->getFullname())
      return UCOMPLETED;
    UString* method = variablename->getMethod();
    UString* devicename = variablename->getDevice();
    UDevice* dev = 0;

    if (libport::has(connection->server->devicetab, devicename->c_str()))
      dev = connection->server->devicetab[devicename->c_str()];

    if (dev == 0 && devicename->equal(connection->connectionTag->c_str()))
      if (libport::has(connection->server->devicetab, method->c_str()))
	dev = connection->server->devicetab[method->c_str()];

    if (!variable && !dev)
    {
      send_error(connection, this,
		 "Unknown identifier: %s",
		 variablename->getFullname()->c_str());
      return UCOMPLETED;
    }

    if (dev && !variable)
      variable = dev->device_val;

    if (dev)
    {
      connection->sendf (getTag(),
			 "*** device description: %s\n",
			 dev->detail->c_str());
      connection->sendf (getTag(),
			 "*** device name: %s\n",
			 dev->device->c_str());
    }
    if (variable)
    {
      std::ostringstream tstr;
      switch (variable->value->dataType)
      {
	case DATA_NUM:
	  tstr << "*** current value: "
	       << variable->value->val << std::endl;
	  break;

	case DATA_STRING:
	  tstr << "*** current value: \""
	       << variable->value->str->c_str() <<"\"\n";
	  break;

	case DATA_BINARY:
	  tstr << "*** current value: binary\n";
	  break;
      }
      connection->sendf(getTag(), tstr.c_str().c_str());
    }

    if (dev)
    {
      std::ostringstream tstr;
      tstr << "*** current device load: " << dev->device_load->value->val<<'\n';
      connection->sendf(getTag(), tstr.c_str().c_str());
    }

    if (variable)
    {
      std::ostringstream tstr;
      if (variable->rangemin != -UINFINITY)
	tstr << "*** rangemin: " << variable->rangemin << '\n';
      else
	tstr << "*** rangemin: -INF\n";
      connection->sendf(getTag(), tstr.c_str().c_str());
      tstr.str("");

      if (variable->rangemax != UINFINITY)
	tstr << "*** rangemax: " << variable->rangemax << '\n';
      else
	tstr << "*** rangemax: INF\n";
      connection->sendf(getTag(), tstr.c_str().c_str());
      tstr.str("");

      if (variable->speedmin != -UINFINITY)
	tstr << "*** speedmin: " << variable->rangemin << '\n';
      else
	tstr << "*** speedmin: -INF\n";
      connection->sendf(getTag(), tstr.c_str().c_str());
      tstr.str("");

      if (variable->speedmax != UINFINITY)
	tstr << "*** speedmax: " << variable->rangemax << '\n';
      else
	tstr << "*** speedmax: INF\n";
      connection->sendf(getTag(), tstr.c_str().c_str());
      tstr.str("");

      if (variable->unit)
	connection->sendf(getTag(),
			  "*** unit: %s\n", variable->unit->c_str());
      else
	connection->sendf(getTag(),
			  "*** unit: unspecified\n");
    }

    return UCOMPLETED;
  }
#endif

  return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_OPERATOR_VAR::copy() const
{
  return copybase(new UCommand_OPERATOR_VAR(loc_, ucopy (oper),
			      ucopy (variablename)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_OPERATOR_VAR::print_(unsigned l) const
{
  debug("%s:\n", oper->c_str());
  DEBUG_ATTR (variablename);
}

MEMORY_MANAGER_INIT(UCommand_BINDER);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_BINDER::UCommand_BINDER(const location& l,
				 UVariableName* objname,
				 UString* binder,
				 UBindType type,
				 UVariableName* variablename,
				 int nbparam)
  : UCommand(l, GENERIC),
    binder (binder),
    variablename (variablename),
    objname (objname),
    type (type),
    nbparam (nbparam)
{
  ADDOBJ(UCommand_BINDER);
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
UCommand::Status
UCommand_BINDER::execute_(UConnection *connection)
{
#ifdef REMOTE_UOBJECT_DISABLED
  send_error(connection, this,
	     "Remote binding not allowed in the free version.");
  return UCOMPLETED;
#endif

  UString *fullname = variablename->buildFullname(this, connection);
  if (!fullname)
    return UCOMPLETED;

  UString *fullobjname = 0;
  if (objname)
  {
    fullobjname = objname->id;
    if (!fullobjname)
      return UCOMPLETED;
  }

  if (type != 3) // not object binder
    debug("BINDING: %s type(%d) %s[%d] from %s\n",
	  binder->c_str(), type, fullname->c_str(),
	  nbparam, fullobjname->c_str());
  else
    debug("BINDING: %s type(%d) %s\n",
	  binder->c_str(), type, variablename->id->c_str());

  UBindMode mode = UEXTERNAL;

  UString *key = new UString(*fullname);

  switch (type)
  {
    case UBIND_VAR:
    {
      HMvariabletab::iterator it = ::urbiserver->getVariableTab ().find(key->c_str());
      if (it == ::urbiserver->getVariableTab ().end())
      {
	UVariable *variable = new UVariable(key->c_str(), new UValue());
	variable->binder = new UBinder(*fullobjname, *fullname,
				       mode,
				       type, nbparam, connection);
      }
      else
      {
	if (it->second->binder)
	  it->second->binder->addMonitor(*fullobjname, connection);
	else
	  it->second->binder = new UBinder(*fullobjname, *fullname,
					   mode,
					   type,
					   nbparam,
					   connection);
	if (!it->second->internalAccessBinder.empty ()
	     && !libport::has (::urbiserver->access_and_change_varlist,
			       it->second))
	{
	  it->second->access_and_change = true;
	  ::urbiserver->access_and_change_varlist.push_back (it->second);
	}
      }
    }
    break;

    case UBIND_FUNCTION:
    {
      // Autodetect redefined members higher in the hierarchy of an object
      // If one is found, cancel the binding.
      HMobjtab::iterator it = ::urbiserver->getObjTab ().find(fullobjname->c_str());
      if (it != ::urbiserver->getObjTab ().end())
      {
	UObj* srcobj = it->second;
	bool ambiguous;
	std::string member (fullname->str ());
	member = member.substr (member.find ('.') + 1);
	UFunction* fun = srcobj->searchFunction (member.c_str (), ambiguous);
	if (fun && fun != kernel::remoteFunction && !ambiguous)
	  break;
      }

      // do the binding
      if (!libport::mhas(::urbiserver->getFunctionBinderTab (), key->c_str()))
	::urbiserver->getFunctionBinderTab ()[key->c_str()] =
	  new UBinder(*fullobjname, *fullname,
		      mode, type, nbparam, connection);
      else
	::urbiserver->getFunctionBinderTab ()[key->c_str()]->
	  addMonitor(*fullobjname, connection);
    }
    break;

    case UBIND_EVENT:
      if (!libport::mhas(::urbiserver->getEventBinderTab (), key->c_str()))
	::urbiserver->getEventBinderTab ()[key->c_str()] =
	    new UBinder(*fullobjname, *fullname,
			mode, type, nbparam, connection);
      else
	::urbiserver->getEventBinderTab ()[key->c_str()]->addMonitor(*fullobjname,
							     connection);
      break;

    case UBIND_OBJECT:
    {
      UObj* uobj;
      if (libport::mhas(::urbiserver->getObjTab (), variablename->id->c_str()))
	uobj = ::urbiserver->getObjTab ()[variablename->id->c_str()];
      else
	uobj = new UObj(variablename->id);
      if (uobj->binder)
	uobj->binder->addMonitor(*variablename->id, connection);
      else
	uobj->binder = new UBinder(*uobj->device, *uobj->device,
				   mode,
				   type,
				   0,
				   connection);
      break;
    }
  }

  return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_BINDER::copy() const
{
  return copybase(new UCommand_BINDER(loc_, ucopy (objname),
				      ucopy (binder),
				      type,
				      ucopy (variablename),
				      nbparam));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_BINDER::print_(unsigned l) const
{
  debug("%s type:%d nbparam:%d:\n", binder->c_str(), type, nbparam);
  DEBUG_ATTR (objname);
  DEBUG_ATTR (variablename);
}

MEMORY_MANAGER_INIT(UCommand_OPERATOR);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_OPERATOR::UCommand_OPERATOR(const location& l,
				     UString* oper)
  : UCommand(l, GENERIC),
    oper (oper)
{
  ADDOBJ(UCommand_OPERATOR);
}

//! UCommand subclass destructor.
UCommand_OPERATOR::~UCommand_OPERATOR()
{
  FREEOBJ(UCommand_OPERATOR);
  delete oper;
}

//#define ENABLE_BENCH
#ifdef ENABLE_BENCH
# include "testspeed.hh"
#endif
//! UCommand subclass execution function
UCommand::Status
UCommand_OPERATOR::execute_(UConnection *connection)
{
  if (*oper == "ping")
  {
#ifdef ENABLE_BENCH
    dotest(connection->server);
#endif
    std::ostringstream o;
    o <<  "*** pong time="<<std::left <<connection->server->getTime()<<'\n';

    connection->sendf(getTag(), o.str().c_str());
    return UCOMPLETED;
  }

  if (*oper == "commands")
  {
#if 0
    // Code disabled for k2.
    if (connection->activeCommand)
      connection->activeCommand->print(0);
    debug("*** LOCAL TREE ***\n");
    if (connection->parser().commandTree)
      connection->parser().commandTree->print(0);
#endif
    return UCOMPLETED;
  }

  if (*oper == "strict")
  {
    connection->server->setDefCheck (true);
    return UCOMPLETED;
  }

  if (*oper == "unstrict")
  {
    connection->server->setDefCheck (false);
    return UCOMPLETED;
  }

  if (*oper == "motoron")
  {
    if (connection->receiving)
      return URUNNING;
    send_error(connection, this, "This command is no longer valid."
	       " Please use \"motor on\" instead");
    return UCOMPLETED;
  }

  if (*oper == "motoroff")
  {
    send_error(connection, this, "This command is no longer valid."
	       " Please use \"motor off\" instead");
    return UCOMPLETED;
  }

  if (*oper == "stopall")
  {
    connection->sendf (getTag(), "*** All commands cleared\n");
    connection->server->stopall = true;
    return UCOMPLETED;
  }

  if (*oper == "undefall")
  {
    connection->sendf (getTag(),
		       "*** undefall is deprecated. Use 'reset' instead\n");
    return UCOMPLETED;
  }

  if (*oper == "reset")
  {
    connection->sendf (getTag(), "*** Reset in progress\n");
    ::urbiserver->reseting = true;

    return UCOMPLETED;
  }

  if (*oper == "devices")
  {
    connection->sendf (getTag(),
		       "*** devices is deprecated."
		       " Use 'group objects' instead.\n");
    return UCOMPLETED;
  }

  if (*oper == "functions")
  {
     for (HMfunctiontab::iterator i =
	   connection->server->getFunctionTab ().begin();
	 i != connection->server->getFunctionTab ().end();
	 ++i)
    {
      std::ostringstream o;
      o << "*** " << i->second->name().c_str() << " ["
	<< i->second->nbparam() << ']';
      o << '\n';
      connection->sendf(getTag(), o.str().c_str());
    }
    return UCOMPLETED;
  }

  if (*oper == "vars")
  {
    for (HMvariabletab::iterator i =
	   connection->server->getVariableTab ().begin();
	 i != connection->server->getVariableTab ().end();
	 ++i)
    {

      std::ostringstream o;
      o << "*** " <<  i->second->getVarname() << " = ";
      switch (i->second->value->dataType)
      {
      case DATA_NUM:
	o << i->second->value->val;
	break;

      case DATA_STRING:
	o << i->second->value->str->c_str();
	break;

      case DATA_BINARY:
	o << "BIN ";
	if (i->second->value->refBinary)
	  o << i->second->value->refBinary->ref()->bufferSize;
	else
	  o << "0 null";
	break;

      case DATA_LIST:
	o << "LIST";
	break;

      case DATA_OBJ:
	o << "OBJ";
	break;

      case DATA_VOID:
	o << "VOID";
	break;

      default:
	o << "UNKNOWN TYPE";
      }
      o << '\n';
      connection->sendf(getTag(), o.str().c_str());
    }

    return UCOMPLETED;
  }

  if (*oper == "events")
  {
    for (HMemittab::iterator i =
	   connection->server->emittab.begin();
	 i != connection->server->emittab.end();
	 ++i)
    {
      std::ostringstream o;
      o << "*** " << i->second->unforgedName->c_str() << "["
	   <<  i->second->nbarg () << "]\n";
      connection->sendf(getTag(), o.str().c_str());
    }

    return UCOMPLETED;
  }

  if (*oper == "taglist")
  {
    for (HMtagtab::iterator i = connection->server->getTagTab ().begin();
	 i != connection->server->getTagTab ().end();
	 ++i)
    {
      std::ostringstream tstr;
      if (i->second.name != "__system__" &&
	  i->second.name != "__node__" &&
	  i->second.name != "__UGrouped_set_of_commands__" &&
	  i->second.name != "notag")
      {
	tstr << "*** " << i->second.name << "\n";
	connection->sendf(getTag(), tstr.str().c_str());
      }
    }

    connection->sendf(getTag(), "*** end of tag list.\n");
    return UCOMPLETED;
  }

  if (*oper == "runningcommands")
  {
    for (HMtagtab::iterator i = connection->server->getTagTab ().begin();
	 i != connection->server->getTagTab ().end();
	 ++i)
    {
      for (std::list<UCommand *>::iterator j = i->second.commands.begin();
	   j != i->second.commands.end(); j++)
      {
	std::ostringstream tstr;
	tstr << "*** "<< i->second.name<<" " << (*j)->loc() << "\n";
	connection->sendf(getTag(), tstr.str().c_str());
      }
    }
  }

  if (*oper == "uservars")
  {
    for (HMvariabletab::iterator i = connection->server->getVariableTab ().begin();
	 i != connection->server->getVariableTab ().end();
	 ++i)
    {
      if (i->second->uservar)
      {
	std::ostringstream o;
	o << "*** " << i->second->getVarname() << " = ";
	switch (i->second->value->dataType)
	{
	  case DATA_NUM:
	    o << i->second->value->val;
	    break;

	  case DATA_STRING:
	    o << i->second->value->str->c_str();
	    break;

	  case DATA_BINARY:
	    o << "BIN ";
	    if (i->second->value->refBinary)
	      o << i->second->value->refBinary->ref()->bufferSize;
	    else
	      o << "0 null";
	    break;

	  case DATA_OBJ:
	    o << "OBJ " << i->second->value->str->c_str();
	    break;

	  default:
	    o << "UNKNOWN TYPE";
	}
	o << '\n';
	connection->sendf(getTag(), o.str().c_str());
      }
    }

    return UCOMPLETED;
  }

  if (*oper == "debugon")
  {
    connection->server->debugOutput = true;
    return UCOMPLETED;
  }

  if (*oper == "connections")
  {
    for (std::list<UConnection*>::iterator i =
	   ::urbiserver->connectionList.begin();
	 i != ::urbiserver->connectionList.end();
	 ++i)
      if ((*i)->isActive())
      {
	connection->sendf (getTag(), "*** %s (%d.%d.%d.%d)\n",
			   (*i)->connectionTag->c_str(),
			   static_cast<int>(((*i)->clientIP>>24) % 256),
			   static_cast<int>(((*i)->clientIP>>16) % 256),
			   static_cast<int>(((*i)->clientIP>>8) % 256),
			   static_cast<int>((*i)->clientIP     % 256)
	  );
      }

    return UCOMPLETED;
  }

  if (*oper == "debugoff")
  {
    connection->server->debugOutput = false;
    return UCOMPLETED;
  }

  if (*oper == "quit")
  {
    connection->closeConnection();
    return UCOMPLETED;
  }

  if (*oper == "reboot")
  {
    connection->server->reboot();
    return UCOMPLETED;
  }

  if (*oper == "shutdown")
  {
    connection->server->shutdown();
    return UCOMPLETED;
  }

  return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_OPERATOR::copy() const
{
  return copybase(new UCommand_OPERATOR(loc_, ucopy (oper)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_OPERATOR::print_(unsigned) const
{
  debug("%s:\n", oper->c_str());
}

MEMORY_MANAGER_INIT(UCommand_WAIT);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_WAIT::UCommand_WAIT(const location& l,
			     UExpression* expression) :
  UCommand(l, WAIT),
  expression (expression),
  endtime (0)
{
  ADDOBJ(UCommand_WAIT);
}

//! UCommand subclass destructor.
UCommand_WAIT::~UCommand_WAIT()
{
  FREEOBJ(UCommand_WAIT);
  delete expression;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_WAIT::execute_(UConnection *connection)
{
  if (status == UONQUEUE)
  {
    UValue *nb = expression->eval(this, connection);
    if (nb == 0)
      return UCOMPLETED;

    if (nb->dataType != DATA_NUM)
    {
      send_error(connection, this, "Invalid type. NUM expected.");
      return UCOMPLETED;
    }
    if (nb->val == 0)
      return UCOMPLETED;

    endtime = connection->server->lastTime() + nb->val;

    delete nb;
    status = URUNNING;
  }
  else
  {
    if (frozen)
    {
      frozen = false;
      endtime += connection->server->lastTime() - lastExec;
    }

    if (connection->server->lastTime() >= endtime)
      status = UCOMPLETED;
  }

  lastExec = connection->server->lastTime();
  return status;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_WAIT::copy() const
{
  return copybase(new UCommand_WAIT(loc_, ucopy (expression)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_WAIT::print_(unsigned l) const
{
  DEBUG_ATTR_I (expression);
}

MEMORY_MANAGER_INIT(UCommand_EMIT);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_EMIT::UCommand_EMIT(const location& l,
			     UVariableName *eventname,
			     UNamedParameters *parameters,
			     UExpression *duration) :
  UCommand(l, EMIT),
  eventname (eventname),
  parameters (parameters),
  duration (duration),
  eventnamestr (0),
  firsttime (true),
  event (0),
  eh (0)
{
  ADDOBJ(UCommand_EMIT);
}

//! UCommand subclass destructor.
UCommand_EMIT::~UCommand_EMIT()
{
  FREEOBJ(UCommand_EMIT);
  if (event && eh)
    removeEvent ();

  delete eventname;
  delete parameters;
  delete duration;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_EMIT::execute_(UConnection *connection)
{
  if (connection->receiving)
    return UONQUEUE;

  ufloat thetime = connection->server->lastTime();

  if (firsttime)
  {
    if (duration)
    {
      UValue *dur = duration->eval(this, connection);
      if (!dur)
      {
	send_error(connection, this, "Invalid event duration for event %s",
		   eventnamestr);
	return UCOMPLETED;
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
	  pevent->expression = new UExpression(loc(), UExpression::VALUE, e1->copy());
	  delete e1;
	}

	pevent = pevent->next;
      }
    }

    UString* ens = eventname->buildFullname(this, connection);

    // register event
    int nbargs = 0;
    if (parameters)
      nbargs = parameters->size ();

    eh = kernel::findEventHandler (ens, nbargs);
    if (!eh)
    {
      if (::urbiserver->isDefChecking ())
      {
	send_error(connection, this, "undefined event %s with %d param(s)",
		   ens->c_str(),
		   nbargs);
	return UCOMPLETED;
      }
      eh = new UEventHandler (ens, nbargs);
    }
    ASSERT (eh);
    event = eh->addEvent (parameters, this, connection);
    eventnamestr = ens->c_str();

    ////// EXTERNAL /////

    HMbindertab::iterator it =
      ::urbiserver->getEventBinderTab ().find(eventnamestr);

    if (it != ::urbiserver->getEventBinderTab ().end()
	&& ((parameters && it->second->nbparam == parameters->size())
	    ||(!parameters && it->second->nbparam == 0))
	&& !it->second->monitors.empty())
    {
      std::ostringstream o;
      o << "[2,\"" << eventnamestr << "__" << it->second->nbparam << "\"";
      for (std::list<UMonitor*>::iterator j = it->second->monitors.begin();
	   j != it->second->monitors.end();
	   ++j)
      {
	(*j)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
	(*j)->c->sendc(reinterpret_cast<const ubyte*>(o.str().c_str()),
		       o.str().size());
	for (UNamedParameters *pvalue = parameters;
	     pvalue != 0;
	     pvalue = pvalue->next)
	{
	  (*j)->c->sendc(reinterpret_cast<const ubyte*>(","), 1);
	  UValue* valparam = pvalue->expression->eval(this, connection);
	  valparam->echo((*j)->c);
	}
	(*j)->c->send(reinterpret_cast<const ubyte*>("]\n"), 2);
      }
    }


    ////// INTERNAL /////

    urbi::UTable::iterator hmfi = urbi::eventmap->find(eventnamestr);
    if (hmfi != urbi::eventmap->end())
    {
      for (std::list<urbi::UGenericCallback*>::iterator cbi =
	     hmfi->second.begin();
	   cbi != hmfi->second.end();
	   ++cbi)
      {
	if (parameters && parameters->size() == (*cbi)->nbparam
	    || !parameters && !(*cbi)->nbparam)
	{
	  urbi::UList tmparray;
	  for (UNamedParameters *pvalue = parameters;
	       pvalue != 0;
	       pvalue = pvalue->next)
	  {
	    UValue* valparam = pvalue->expression->eval(this, connection);
	    if (!valparam)
	    {
	      send_error(connection, this, "EXPR evaluation failed");
	      return UCOMPLETED;
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

  if (thetime > targetTime && !firsttime)
  {
    removeEvent ();
    return UCOMPLETED;
  }

  firsttime = false;
  return UBACKGROUND;
}

void
UCommand_EMIT::removeEvent ()
{
  // unregister event
  if (event && eh) // free the event
    eh->removeEvent (event);

  ////// EXTERNAL /////

  {
    HMbindertab::iterator i =
      ::urbiserver->getEventBinderTab ().find(eventnamestr);
    if (i != ::urbiserver->getEventBinderTab ().end()
	&& parameters
	&& i->second->nbparam == parameters->size()
	&& !i->second->monitors.empty())
    {
      std::ostringstream o;
      o << "[3,\"" << eventnamestr << "__" << i->second->nbparam << "\"]\n";
      for (std::list<UMonitor*>::iterator j = i->second->monitors.begin();
	   j != i->second->monitors.end();
	   ++j)
      {
	(*j)->c->sendPrefix(EXTERNAL_MESSAGE_TAG);
	(*j)->c->send(reinterpret_cast<const ubyte*>(o.str().c_str()),
		      o.str().size());
      }
    }
  }

  ////// INTERNAL /////
  {
    urbi::UTable::iterator i = urbi::eventendmap->find(eventnamestr);
    if (i != urbi::eventendmap->end())
      for (std::list<urbi::UGenericCallback*>::iterator j = i->second.begin();
	   j != i->second.end(); ++j)
      {
	urbi::UList tmparray;
	(*j)->__evalcall(tmparray);
      }
  }
}

//! UCommand subclass hard copy function
UCommand*
UCommand_EMIT::copy() const
{
  return copybase(new UCommand_EMIT(loc_,
				    ucopy (eventname),
				    ucopy (parameters)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_EMIT::print_(unsigned l) const
{
  DEBUG_ATTR (eventname);
}

MEMORY_MANAGER_INIT(UCommand_WAIT_TEST);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_WAIT_TEST::UCommand_WAIT_TEST(const location& l,
				       UExpression* test)
  : UCommand(l, WAIT_TEST),
    test (test),
    nbTrue (0)
{
  ADDOBJ(UCommand_WAIT_TEST);
}

//! UCommand subclass destructor.
UCommand_WAIT_TEST::~UCommand_WAIT_TEST()
{
  FREEOBJ(UCommand_WAIT_TEST);
  delete test;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_WAIT_TEST::execute_(UConnection *connection)
{
  if (!test)
    return UCOMPLETED;
  UTestResult testres = booleval(test->eval(this, connection));

  if (testres == UTESTFAIL)
    return URUNNING;

  if (testres == UTRUE)
  {
    if (nbTrue == 0)
      startTrue = connection->server->lastTime();
    ++nbTrue;
  }
  else
    nbTrue = 0;

  if ((test->softtest_time
       && nbTrue > 0
       && (connection->server->lastTime() - startTrue
	   >= test->softtest_time->val))
      || (nbTrue > 0 && test->softtest_time == 0))
    return UCOMPLETED;
  else
    return URUNNING;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_WAIT_TEST::copy() const
{
  UCommand_WAIT_TEST *ret = new UCommand_WAIT_TEST(loc_, ucopy (test));
  copybase(ret);
  ret->nbTrue  = 0;
  return ret;
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_WAIT_TEST::print_(unsigned l) const
{
  DEBUG_ATTR_I (test);
}


MEMORY_MANAGER_INIT(UCommand_INCDECREMENT);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_INCDECREMENT::UCommand_INCDECREMENT(const location& l,
					     Type type,
					     UVariableName *variablename)
  : UCommand(l, type),
    variablename (variablename)
{
  ADDOBJ(UCommand_INCDECREMENT);
}

//! UCommand subclass destructor.
UCommand_INCDECREMENT::~UCommand_INCDECREMENT()
{
  FREEOBJ(UCommand_INCDECREMENT);
  delete variablename;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_INCDECREMENT::execute_(UConnection *connection)
{
  variablename->getVariable(this, connection);
  if (!variablename->getFullname())
    return UCOMPLETED;
  variablename->getMethod();
  variablename->getDevice();

  // Broadcasting
  if (scanGroups(&UCommand::refVarName, true))
    return UMORPH;

  // Main execution
  if (type == INCREMENT)
  {
    morph =
      new UCommand_ASSIGN_VALUE(loc_,
				variablename->copy(),
				new UExpression(loc(),
				  UExpression::PLUS,
				  new UExpression(loc(),
						  UExpression::VARIABLE,
						  variablename->copy()),
				  new UExpression(loc(),
						  UExpression::VALUE,
						  ufloat(1))), 0);

    persistant = false;
    return UMORPH;
  }

  if (type == DECREMENT)
  {
    morph =
      new UCommand_ASSIGN_VALUE
      (loc_,
       variablename->copy(),
       new UExpression(loc(),
		       UExpression::MINUS,
		       new UExpression(loc(), UExpression::VARIABLE,
				       variablename->copy()),
		       new UExpression(loc(),
				       UExpression::VALUE, ufloat(1))), 0);

    persistant = false;
    return UMORPH;
  }

  return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_INCDECREMENT::copy() const
{
  return copybase(new UCommand_INCDECREMENT(loc_, type, ucopy (variablename)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_INCDECREMENT::print_(unsigned l) const
{
  if (type == INCREMENT)
    debug("INC\n");
  else if (type == DECREMENT)
    debug("DEC\n");
  else
    debug("UNKNOWN TYPE\n");

  DEBUG_ATTR (variablename);
}

MEMORY_MANAGER_INIT(UCommand_DEF);
// **************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_DEF::UCommand_DEF(const location& l,
			   UDefType deftype,
			   UVariableName *variablename,
			   UNamedParameters *parameters,
			   UCommand* command)
  : UCommand(l, DEF)
{
  ADDOBJ(UCommand_DEF);
  this->deftype	     = deftype;
  this->variablename = variablename;
  this->parameters   = parameters;
  this->command	     = command;
  this->device	     = 0;
  this->variablelist = 0;
}

//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_DEF::UCommand_DEF(const location& l,
			   UDefType deftype,
			   UString *device,
			   UNamedParameters *parameters)
  : UCommand(l, DEF)
{
  ADDOBJ(UCommand_DEF);
  this->deftype	     = deftype;
  this->variablename = 0;
  this->parameters   = parameters;
  this->command	     = 0;
  this->device	     = device;
  this->variablelist = 0;
}

//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_DEF::UCommand_DEF(const location& l,
			   UDefType deftype,
			   UVariableList *variablelist)
  : UCommand(l, DEF),
    variablename (0),
    parameters (0),
    command (0),
    device (0),
    variablelist (variablelist),
    deftype (deftype)
{
  ADDOBJ(UCommand_DEF);
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
UCommand::Status
UCommand_DEF::execute_(UConnection *connection)
{
  // Def list query
  if (!variablename && !command && !parameters && !variablelist)
  {
    for (HMfunctiontab::iterator i =
	   connection->server->getFunctionTab ().begin();
	 i != connection->server->getFunctionTab ().end();
	 ++i)
      connection->sendf (getTag(), "*** %s : %d param(s)\n",
			 i->second->name().c_str(),
			 i->second->nbparam());
    return UCOMPLETED;
  }

  // Function definition
  if (deftype == UDEF_FUNCTION && variablename && command)
  {
    UString* funname = variablename->buildFullname(this, connection);
    if (!funname)
      return UCOMPLETED;

    if (variablename->nostruct &&
	(libport::mhas(::urbiserver->getGroupTab (),
		       variablename->getMethod()->c_str())))
    {
      send_error(connection, this,
		 "function name conflicts with group %s ",
		 variablename->getMethod()->c_str());
      return UCOMPLETED;
    }

    if (libport::mhas(connection->server->getFunctionTab (), funname->c_str()))
    {
      if (::urbiserver->isDefChecking ())
	send_error(connection, this,
		 "Warning: function %s already exists", funname->c_str());

      // undef function
      UFunction* fun = variablename->getFunction(this, connection);
      connection->server->getFunctionTab ().erase(
	connection->server->getFunctionTab ().find(funname->c_str()));
      connection->server->getFunctionDefTab ().erase(
	connection->server->getFunctionDefTab ().find(funname->c_str()));
      delete fun;
    }

    UFunction *fun = new UFunction(*new UString(*funname), parameters, command);
    if (fun)
      connection->server->getFunctionDefTab ()[fun->name().c_str()] = fun;

    if (fun && command)
      connection->server->getFunctionTab ()[fun->name().c_str()] = fun;

    return UCOMPLETED;
  }

  // Event definition
  if (deftype == UDEF_EVENT && variablename)
  {
    UString* eventname = variablename->buildFullname(this, connection);
    if (!eventname)
      return UCOMPLETED;
    int eventnbarg = 0;
    if (parameters)
      eventnbarg = parameters->size();

    // FIXME: What is the logic here?  New, but not storing anything.
    UEventHandler* eh = kernel::findEventHandler(eventname, eventnbarg);
    if (!eh)
      eh = new UEventHandler(eventname, eventnbarg);

    return UCOMPLETED;
  }


  // Single Variable definition
  if (variablename
      && !command
      && !parameters
      && deftype != UDEF_FUNCTION)
  {
    UVariable* variable = variablename->getVariable(this, connection);
    if (!variablename->getFullname())
      return UCOMPLETED;
    if (variable) // the variable is already defined
      return UCOMPLETED;

    // Variable definition

    variable = new UVariable(variablename->getFullname()->c_str(), new UValue());
    connection->localVariableCheck(variable);

    return UCOMPLETED;
  }

  // Device variable set definition
  if (device && !command && parameters)
  {
    UNamedParameters * param = parameters;
    UCommand_DEF *cdef = new UCommand_DEF (loc_, UDEF_VAR,
					   new UVariableName(device->copy(),
							     param->name,
							     true,
							     0),
					   0,
					   0);
    cdef->setTag(this);
    morph = cdef;

    for (param = param->next; param; param = param->next)
      if (param->name)
      {
	cdef = new UCommand_DEF (loc_, UDEF_VAR,
				 new UVariableName(device->copy(),
						   param->name,
						   true,
						   0),
				 0,
				 0);
	cdef->setTag(this);
	morph = new UCommand_TREE(loc_, Flavorable::UAND, cdef, morph);
      }
    persistant = false;
    return UMORPH;
  }

  // Multi Variable definition
  if (variablelist)
  {
    UVariableList *list = variablelist;
    list->variablename->local_scope = true;
    UCommand_DEF *cdef = new UCommand_DEF (loc_, UDEF_VAR,
					   list->variablename->copy(),
					   0,
					   0);
    cdef->setTag(this);
    morph = cdef;
    for (list = list->next; list; list = list->next)
      if (list->variablename)
      {
	list->variablename->local_scope = true;
	cdef = new UCommand_DEF (loc_, UDEF_VAR,
				 list->variablename->copy(), 0, 0);
	cdef->setTag(this);
	morph = new UCommand_TREE(loc_, Flavorable::UAND, cdef, morph);
      }

    persistant = false;
    return UMORPH;
  }

  return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_DEF::copy() const
{
  UCommand_DEF *ret = new UCommand_DEF(loc_, deftype, ucopy (variablename),
				       ucopy (parameters),
				       ucopy (command));
  ret->variablelist = ucopy (variablelist);
  return copybase(ret);
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_DEF::print_(unsigned l) const
{
  DEBUG_ATTR (variablename);
  DEBUG_ATTR(variablelist);
  DEBUG_ATTR(parameters);
  DEBUG_ATTR_I(command);
}

MEMORY_MANAGER_INIT(UCommand_CLASS);
// **************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_CLASS::UCommand_CLASS(const location& l,
			       UString *object,
			       UNamedParameters *parameters)
  : UCommand(l, CLASS),
    object (object),
    parameters (parameters)
{
  ADDOBJ(UCommand_CLASS);
}

//! UCommand subclass destructor.
UCommand_CLASS::~UCommand_CLASS()
{
  FREEOBJ(UCommand_CLASS);
  delete object;
  delete parameters;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_CLASS::execute_(UConnection*)
{
  // remote new processing
  HMobjWaiting::iterator ow
    = ::urbiserver->getObjWaitTab ().find(object->c_str());
  if (ow != ::urbiserver->getObjWaitTab ().end())
  {
    --ow->second->nb;
    if (ow->second->nb == 0)
      ::urbiserver->getObjWaitTab ().erase(ow);
    else
      return URUNNING;
  }


  // add some object storage here based on 'object'
  new UObj(object);
  if (!parameters)
    return UCOMPLETED;

  // morph into a series of & for each element of the class
  morph = 0;

  for (UNamedParameters *param = parameters; param; param = param->next)
  {
    if (param->expression)
    {
      UCommand_DEF* cdef = 0;
      switch (param->expression->type)
      {
	case UExpression::VALUE:
	  cdef = new UCommand_DEF(loc_, UCommand_DEF::UDEF_VAR,
				  new UVariableName(
				    new UString(*object),
				    new UString(*param->expression->str),
				    true,
				    0),
				  0,
				  0);
	  break;
	case UExpression::FUNCTION:
	  cdef = new UCommand_DEF(loc_, UCommand_DEF::UDEF_FUNCTION,
				  new UVariableName(
				    new UString(*object),
				    new UString(*param->expression->variablename->id),
				    true,
				    0),
				  param->expression->parameters,
				  0);
	  break;
	case UExpression::EVENT:
	  cdef = new UCommand_DEF(loc_, UCommand_DEF::UDEF_EVENT,
				  new UVariableName(
				    new UString(*object),
				    new UString(*param->expression->variablename->id),
				    true,
				    0),
				  param->expression->parameters,
				  0);
	  break;
	  // FIXME: This list is sick, we need something else.
	case UExpression::ADDR_VARIABLE:
	case UExpression::COPY:
	case UExpression::DIV:
	case UExpression::EXP:
	case UExpression::GROUP:
	case UExpression::LIST:
	case UExpression::MINUS:
	case UExpression::MOD:
	case UExpression::MULT:
	case UExpression::NEG:
	case UExpression::PLUS:
	case UExpression::PROPERTY:
	case UExpression::TEST_AND:
	case UExpression::TEST_BANG:
	case UExpression::TEST_DEQ:
	case UExpression::TEST_EQ:
	case UExpression::TEST_GE:
	case UExpression::TEST_GT:
	case UExpression::TEST_LE:
	case UExpression::TEST_LT:
	case UExpression::TEST_NE:
	case UExpression::TEST_OR:
	case UExpression::TEST_PEQ:
	case UExpression::TEST_REQ:
	case UExpression::VARIABLE:
	  break;
      }
      if (cdef)
      {
	cdef->setTag(this);
	if (param == parameters)
	  morph = cdef;
	else
	  morph = new UCommand_TREE(loc_, Flavorable::UAND, cdef, morph);
      }
    }
  }

  if (morph)
  {
    persistant = false;
    return UMORPH;
  }
  else
    return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_CLASS::copy() const
{
  return copybase(new UCommand_CLASS(loc_, ucopy (object),
				     ucopy (parameters)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_CLASS::print_(unsigned l) const
{
  if (object)
    debug(l, "  Object name: %s\n", object->c_str());
  DEBUG_ATTR(parameters);
}

MEMORY_MANAGER_INIT(UCommand_IF);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_IF::UCommand_IF(const location& l,
			 UExpression *test,
			 UCommand* command1,
			 UCommand* command2)
  : UCommand(l, IF),
    test (test),
    command1 (command1),
    command2 (command2)
{
  ADDOBJ(UCommand_IF);
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
UCommand::Status
UCommand_IF::execute_(UConnection *connection)
{
  if (!test)
    return UCOMPLETED;

  UTestResult testres = booleval(test->eval(this, connection));

  if (testres == UTESTFAIL)
    return UCOMPLETED;

  if (testres == UTRUE)
  {
    morph = command1;
    setTag (command1);
    command1 = 0; // avoid delete of command when this is deleted
    persistant = false;
    return UMORPH;
  }
  else if (command2)
  {
    morph = command2;
    setTag (command2);
    command2 = 0; // avoid delete of command when this is deleted
    persistant = false;
    return UMORPH;
  }
  else
    return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_IF::copy() const
{
  return copybase(new UCommand_IF(loc_, ucopy (test),
				  ucopy (command1),
				  ucopy (command2)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_IF::print_(unsigned l) const
{
  DEBUG_ATTR_I (test);
  DEBUG_ATTR_I(command1);
  DEBUG_ATTR_I(command2);
}

MEMORY_MANAGER_INIT(UCommand_EVERY);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_EVERY::UCommand_EVERY(const location& l,
			       UExpression *duration,
			       UCommand* command)
  : UCommand(l, EVERY),
    duration (duration),
    command (command),
    firsttime (true),
    starttime (0)
{
  ADDOBJ(UCommand_EVERY);
}

//! UCommand subclass destructor.
UCommand_EVERY::~UCommand_EVERY()
{
  FREEOBJ(UCommand_EVERY);
  delete command;
  delete duration;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_EVERY::execute_(UConnection* connection)
{
  ufloat thetime = connection->server->lastTime();

  assert (command);

  UValue* interval = duration->eval(this, connection);
  if (!interval)
    return UCOMPLETED;

  if (starttime + interval->val <= thetime ||
      firsttime)
  {
    persistant = true;
    morph = new UCommand_TREE(loc_, Flavorable::UAND, command->copy(), this);
    starttime = thetime;
    firsttime = false;
    delete interval;
    return UMORPH;
  }

  delete interval;
  return UBACKGROUND;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_EVERY::copy() const
{
  return copybase(new UCommand_EVERY(loc_, ucopy (duration),
				     ucopy (command)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_EVERY::print_(unsigned l) const
{
  DEBUG_ATTR_I (duration);
  DEBUG_ATTR_I(command);
}

MEMORY_MANAGER_INIT(UCommand_TIMEOUT);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_TIMEOUT::UCommand_TIMEOUT(const location& l,
				   UExpression *duration,
				   UCommand* command) :
  UCommand(l, TIMEOUT),
  duration (duration),
  command (command)
{
  ADDOBJ(UCommand_TIMEOUT);
  tagRef = new UString(unique ("__TAG_timeout_"));
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
UCommand::Status
UCommand_TIMEOUT::execute_(UConnection*)
{
  assert (command);

  persistant = false;
  morph =
    new UCommand_TREE
    (loc_, Flavorable::UAND,
     new UCommand_TREE(loc_, Flavorable::UPIPE,
		       new UCommand_WAIT(loc_, duration->copy()),
		       new UCommand_OPERATOR_ID(loc_,
						new UString("stop"),
						tagRef->copy())),
     new UCommand_TREE(loc_, Flavorable::UPIPE, command->copy(),
		       new UCommand_OPERATOR_ID(loc_, new UString("stop"),
						tagRef->copy()))
      );
  // We can't tag morph as morphing engine will override us.
  static_cast<UCommand_TREE*>(morph)->command1->setTag(tagRef->c_str());
  static_cast<UCommand_TREE*>(morph)->command2->setTag(tagRef->c_str());

  return UMORPH;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_TIMEOUT::copy() const
{
  return copybase(new UCommand_TIMEOUT(loc_, ucopy (duration),
				       ucopy (command)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_TIMEOUT::print_(unsigned l) const
{
  DEBUG_ATTR_I (duration);
  DEBUG_ATTR_I(command);
}

MEMORY_MANAGER_INIT(UCommand_STOPIF);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_STOPIF::UCommand_STOPIF(const location& l,
				 UExpression *condition,
				 UCommand* command)
  : UCommand(l, STOPIF),
    condition (condition),
    command (command)
{
  ADDOBJ(UCommand_STOPIF);
  tagRef = new UString(unique ("__TAG_stopif_"));
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
UCommand::Status
UCommand_STOPIF::execute_(UConnection *connection)
{
  if (!command || !condition)
    return UCOMPLETED;

  UTestResult testres = booleval(condition->eval(this, connection));

  if (testres == UTRUE)
    return UCOMPLETED;

  persistant = false;
  morph =
    new UCommand_TREE(
      loc_, Flavorable::UAND,
      new UCommand_AT(loc_, Flavorable::USEMICOLON,
		      condition->copy(),
		      new UCommand_OPERATOR_ID(loc_, new UString("stop"),
					       tagRef->copy()),
		      0),
      command->copy());
  morph->setTag(tagRef->c_str());
  return UMORPH;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_STOPIF::copy() const
{
  return copybase(new UCommand_STOPIF(loc_, ucopy (condition),
				      ucopy (command)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_STOPIF::print_(unsigned l) const
{
  DEBUG_ATTR_I(condition);
  DEBUG_ATTR_I(command);
}

MEMORY_MANAGER_INIT(UCommand_FREEZEIF);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_FREEZEIF::UCommand_FREEZEIF(const location& l,
				     UExpression *condition,
				     UCommand* command)
  : UCommand(l, FREEZEIF),
    condition (condition),
    command (command)
{
  ADDOBJ(UCommand_FREEZEIF);
  tagRef = new UString(unique("__TAG_stopif_"));
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
UCommand::Status
UCommand_FREEZEIF::execute_(UConnection*)
{
  assert(command);

  persistant = false;
  UCommand* cmd = new UCommand_TREE(loc_, Flavorable::UPIPE,
				    command->copy(),
				    new UCommand_NOOP(loc_)
    );
  cmd->setTag(tagRef->c_str());
  morph =
    new UCommand_TREE
    (loc_, Flavorable::UAND,
     new UCommand_AT(loc_, Flavorable::USEMICOLON,
		     condition->copy(),
		     new UCommand_OPERATOR_ID(loc_, new UString("freeze"),
					      tagRef->copy()),
		     new UCommand_OPERATOR_ID(loc_, new UString("unfreeze"),
					      tagRef->copy())),
     cmd);

  return UMORPH;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_FREEZEIF::copy() const
{
  return copybase(new UCommand_FREEZEIF(loc_, ucopy (condition),
					ucopy (command)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_FREEZEIF::print_(unsigned l) const
{
  DEBUG_ATTR_I (condition);
  DEBUG_ATTR_I(command);
}

MEMORY_MANAGER_INIT(UCommand_AT);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_AT::UCommand_AT(const location& l,
			 UNodeType f,
			 UExpression *test,
			 UCommand* cmd1,
			 UCommand* cmd2)
  : UASyncCommand(l, AT),
    Flavorable (f),
    test (test),
    command1 (cmd1),
    command2 (cmd2),
    firsttime (true),
    reloop_ (false)
{
  ADDOBJ(UCommand_AT);
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
UCommand::Status
UCommand_AT::execute_(UConnection *connection)
{
  if (!test)
    return UCOMPLETED;

  if (firsttime)
  {
    firsttime = false;
    if (test->asyncScan (this, connection) == UFAIL)
    {
      send_error(connection, this,
		 "Invalid name resolution in test. "
		 "Did you define all events and variables?");
      return UCOMPLETED;
    }
  }

  UCommand* morph_onleave = 0;
  bool domorph = false;
  ufloat currentTime = connection->server->lastTime();
  if (reeval ())
  {
    UEventCompound* ec = 0;
    UValue *testeval = test->eval(this, connection, ec);
    if (!ec)
      ec = new UEventCompound (testeval);
    reset_reeval ();

    if (booleval(testeval, true) == UTESTFAIL)
      return UCOMPLETED;

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

    for (std::list<UMultiEventInstance*>::iterator i = mixlist.begin ();
	 i != mixlist.end ();
	 ++i)
    {
      bool ok = false;
      // FIXME: weird enough: we don't use break (which seems to mean
      // that we can find *i several times), but we free it though.
      for (std::list<UAtCandidate*>::iterator ic = candidates.begin ();
	   ic != candidates.end () && !ok;
	   ++ic)
	if ((*ic)->equal (*i))
	{
	  (*ic)->visited ();
	  ok = true;
	  delete *i;
	}

      if (!ok)
	candidates.push_back (new UAtCandidate (currentTime + duration, *i));
    }

    //cleanup of candidates that do not appear anymore in the mixlist
    for (std::list<UAtCandidate*>::iterator ic = candidates.begin ();
	 ic != candidates.end (); )
      if (!(*ic)->isVisited ())
      {
	delete *ic;
	ic = candidates.erase (ic);
	if (command2)
	{
	  if (!morph_onleave)
	    morph_onleave = command2->copy ();
	  else
	    morph_onleave =
	      new UCommand_TREE (loc_, Flavorable::UAND,
				 morph_onleave, command2->copy ());
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
  }

  if (reloop_)
  {
    reloop_ = false;
    morph = this;
    // scan triggering candidates
    for (std::list<UAtCandidate*>::iterator ic = candidates.begin ();
	 ic != candidates.end ();
	 ++ic)
    {
      UCommand* assigncmd;
      if (!(*ic)->hasTriggered())
      {
	if ((*ic)->trigger (currentTime, assigncmd))
	{
	  if (assigncmd)
	    morph =
	      new UCommand_TREE
	      (loc_, Flavorable::UAND,
	       new UCommand_TREE (loc_, Flavorable::UPIPE,
				  assigncmd, command1->copy()),
	       morph);
	  else
	    morph = new UCommand_TREE (loc_, Flavorable::UAND,
				       command1->copy (), morph);
	}
	else
	  reloop_ = true; // we should try again later
      }
    }

    // morph if necessary
    if (morph != this)
      domorph = true;
  }

  // morphing, if required
  if (domorph)
  {
    if (morph_onleave)
    {
      // at this point, morph is at least equal to "this"
      morph = new UCommand_TREE (loc_, Flavorable::UAND, morph, morph_onleave);
    }
    morph->background = true;
    persistant = true;
    return UMORPH;
  }

  return UBACKGROUND;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_AT::copy() const
{
  return copybase(new UCommand_AT(loc_, flavor(),
				  ucopy (test),
				  ucopy (command1),
				  ucopy (command2)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_AT::print_(unsigned l) const
{
  debug(l, "%s\n", flavor_string());
  DEBUG_ATTR_I(test);
  DEBUG_ATTR_I(command1);
  DEBUG_ATTR_I(command2);
}

MEMORY_MANAGER_INIT(UCommand_WHILE);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_WHILE::UCommand_WHILE(const location& l,
			       Flavorable::UNodeType flavor,
			       UExpression *test,
			       UCommand* command)
  : UCommand(l, WHILE),
    Flavorable (flavor),
    test (test),
    command (command)
{
  ADDOBJ(UCommand_WHILE);
}

//! UCommand subclass destructor.
UCommand_WHILE::~UCommand_WHILE()
{
  FREEOBJ(UCommand_WHILE);
  delete command;
  delete test;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_WHILE::execute_(UConnection *connection)
{
  assert (command);

  persistant = false;

  if (!test)
    return UCOMPLETED;
  UTestResult testres = booleval(test->eval(this, connection));

  if (testres == UTESTFAIL)
    return UCOMPLETED;

  if (testres == UTRUE)
  {
    if (flavor() == UPIPE)
      morph = new UCommand_TREE(loc_, Flavorable::UPIPE,
				command->copy(), this);
    else
      morph =
	new UCommand_TREE(loc_, flavor(),
			  new UCommand_TREE(loc_, Flavorable::UAND,
					    command->copy(),
					    new UCommand_NOOP(loc_)),
			  this);
    persistant = true;
    return UMORPH;
  }
  else
    return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_WHILE::copy() const
{
  return copybase(new UCommand_WHILE(loc_, flavor(),
				     ucopy (test), ucopy (command)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_WHILE::print_(unsigned l) const
{
  debug(l, "%s\n", flavor_string());
  DEBUG_ATTR_I(test);
  DEBUG_ATTR_I(command);
}

MEMORY_MANAGER_INIT(UCommand_WHENEVER);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_WHENEVER::UCommand_WHENEVER(const location& l,
				     UExpression *test,
				     UCommand* command1,
				     UCommand* command2)
  : UASyncCommand(l, WHENEVER),
    test (test),
    command1 (command1),
    command2 (command2),
    firsttime (true),
    reloop_ (false),
    active_ (false),
    theloop_ (0)
{
  ADDOBJ(UCommand_WHENEVER);
}

//! UCommand subclass destructor.
UCommand_WHENEVER::~UCommand_WHENEVER()
{
  FREEOBJ(UCommand_WHENEVER);
  delete command1;
  delete command2;
  delete test;
  if (theloop_)
    dynamic_cast<UCommand_LOOP*>(theloop_)->whenever_hook = 0;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_WHENEVER::execute_(UConnection *connection)
{
  if (!test)
    return UCOMPLETED;

  ufloat currentTime = connection->server->lastTime();

  // handle the 'else' construct
  if (command2)
  {
    morph =
      new UCommand_TREE
      (loc_, Flavorable::UAND, this,
       new UCommand_WHENEVER (loc_,
			      new UExpression (loc(), UExpression::TEST_BANG,
					       test->copy (), 0),
			      command2, 0));

    command2 = 0;
    persistant = true;
    return UMORPH;
  }

  // cache initilialization
  if (firsttime)
  {
    firsttime = false;
    if (test->asyncScan (dynamic_cast<UASyncCommand*>(this), connection) == UFAIL)
    {
      send_error(connection, this,
		 "Invalid name resolution in test. "
		 "Did you define all events and variables?");
      return UCOMPLETED;
    }
  }

  bool domorph = false;
  if (reeval ())
  {
    UEventCompound* ec = 0;
    UValue *testeval = test->eval(this, connection, ec);
    if (!ec)
      ec = new UEventCompound (testeval);
    reset_reeval ();

    UTestResult testres = booleval(testeval, true);
    if (testres == UTESTFAIL)
      return UCOMPLETED;

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

    for (std::list<UMultiEventInstance*>::iterator i = mixlist.begin ();
	 i != mixlist.end ();
	 ++i)
    {
      bool ok = false;
      for (std::list<UAtCandidate*>::iterator ic = candidates.begin ();
	   ic != candidates.end () && !ok;
	   ++ic)
      {
	if ((*ic)->equal (*i))
	{
	  (*ic)->visited ();
	  ok = true;
	  delete *i;
	}
      }
      if (!ok)
	candidates.push_back (new UAtCandidate (currentTime + duration, *i));
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
      connection->server->hasSomethingToDelete ();
      // theloop_ is 0 if something has deleted it from the outside (thanks
      // to the 'whenever_hook' attribute), that's why we test here
      if (theloop_)
	theloop_->toDelete = true;
      theloop_ = 0;
    }
  }

  if (reloop_)
  {
    reloop_ = false;
    bool trigger = false;
    UCommand* assign = 0;

    // scan triggering candidates
    for (std::list<UAtCandidate*>::iterator ic = candidates.begin ();
	 ic != candidates.end ();
	 ++ic)
    {
      UCommand* assigncmd = 0;
      if (!(*ic)->hasTriggered())
      {
	if ((*ic)->trigger (currentTime, assigncmd))
	{
	  trigger = true;
	  if (assigncmd)
	    if (!assign)
	      assign = assigncmd;
	    else
	      assign = new UCommand_TREE (loc_, Flavorable::UAND,
					  assigncmd, assign);
	}
	else
	  reloop_ = true; // we should try again later
      }
    }

    if (trigger && !active_) // we need to start the loop
    {
      active_ = true;
      ASSERT (!theloop_);
      theloop_ = new UCommand_LOOP (loc_, command1->copy ());
      theloop_->setTag (systemTagInfo); //untouchable
      ((UCommand_LOOP*)theloop_)->whenever_hook = this;
      if (assign)
	assign = new UCommand_TREE (loc_, Flavorable::UPIPE, assign, theloop_);
      else
	assign = theloop_;
    }

    if (assign)
    {
      domorph = true;
      morph = new UCommand_TREE (loc_, Flavorable::UPIPE, this, assign);
    }
  }

  // morphing, if required
  if (domorph)
  {
    morph->background = true;
    persistant = true;
    return UMORPH;
  }

  return UBACKGROUND;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_WHENEVER::copy() const
{
  return copybase(new UCommand_WHENEVER(loc_, ucopy (test),
					ucopy (command1),
					ucopy (command2)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_WHENEVER::print_(unsigned l) const
{
  DEBUG_ATTR_I(test);
  DEBUG_ATTR_I(command1);
  DEBUG_ATTR_I(command2);
}

MEMORY_MANAGER_INIT(UCommand_LOOP);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_LOOP::UCommand_LOOP(const location& l,
			     UCommand* command)
  : UCommand(l, LOOP),
    command (command),
    whenever_hook (0)
{
  ADDOBJ(UCommand_LOOP);
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
UCommand::Status
UCommand_LOOP::execute_(UConnection*)
{
  assert(command);

  morph = new UCommand_TREE(loc_, Flavorable::USEMICOLON,
			    new UCommand_TREE(loc_, Flavorable::UAND,
					      command->copy(),
					      new UCommand_NOOP(loc_)),
			    this);
  persistant = true;
  return UMORPH;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_LOOP::copy() const
{
  return copybase(new UCommand_LOOP(loc_, ucopy (command)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_LOOP::print_(unsigned l) const
{
  DEBUG_ATTR_I(command);
}

MEMORY_MANAGER_INIT(UCommand_LOOPN);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_LOOPN::UCommand_LOOPN(const location& l,
			       Flavorable::UNodeType flavor,
			       UExpression* expression,
			       UCommand* command)
  : UCommand(l, LOOPN),
    Flavorable (flavor),
    expression (expression),
    command (command)
{
  ADDOBJ(UCommand_LOOPN);
}

//! UCommand subclass destructor.
UCommand_LOOPN::~UCommand_LOOPN()
{
  FREEOBJ(UCommand_LOOPN);
  delete command;
  delete expression;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_LOOPN::execute_(UConnection *connection)
{
  assert (command);

  if (expression->type != UExpression::VALUE)
  {
    UValue *nb = expression->eval(this, connection);

    if (nb == 0)
      return UCOMPLETED;

    if (nb->dataType != DATA_NUM)
    {
      send_error(connection, this,
		 "number of loops is non numeric");
      delete nb;
      return UCOMPLETED;
    }

    expression->type = UExpression::VALUE;
    expression->dataType = DATA_NUM;
    expression->val = nb->val;
    delete nb;
  }

  if (expression->val < 1)
    return UCOMPLETED;

  expression->val = expression->val - 1;

  switch (flavor ())
  {
    case UPIPE:
    case UAND:
      morph = new UCommand_TREE(loc_, flavor(), command->copy(), this);
      break;
    default:
      morph = new UCommand_TREE
	(loc_, flavor(), new UCommand_TREE(loc_, Flavorable::UAND,
					   command->copy(),
					   new UCommand_NOOP(loc_)),
	 this);
  }
  persistant = true;
  return UMORPH;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_LOOPN::copy() const
{
  return copybase(new UCommand_LOOPN(loc_, flavor(),
				     ucopy (expression),
				     ucopy (command)));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_LOOPN::print_(unsigned l) const
{
  debug(l, "%s", flavor_string ());
  DEBUG_ATTR_I (expression);
  if (command)
  {
    debug(l, "  Command (%p:%d):\n", command, (int)command->status);
    command->print(l+3);
  }
}

MEMORY_MANAGER_INIT(UCommand_FOR);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_FOR::UCommand_FOR(const location& l,
			   UNodeType flavor,
			   UCommand* instr1,
			   UExpression* test,
			   UCommand* instr2,
			   UCommand* command)
  : UCommand(l, FOR),
    Flavorable (flavor),
    instr1 (instr1),
    instr2 (instr2),
    test (test),
    command (command),
    first (true)
{
  ADDOBJ(UCommand_FOR);
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
UCommand::Status
UCommand_FOR::execute_(UConnection *connection)
{
  if (first)
  {
    first = false;
    if (instr1)
      instr1->setTag(this);
    if (instr2)
      instr2->setTag(this);
  }

  assert (command);

  if (instr1)
  {
    UCommand *first_instruction = instr1;

    instr1 = 0;
    morph = new UCommand_TREE(loc_, Flavorable::USEMICOLON,
			      first_instruction, this);
    persistant = true;
    return UMORPH;
  }

  persistant = false;

  if (!test)
    return UCOMPLETED;
  UTestResult testres = booleval(test->eval(this, connection));

  if (testres == UTESTFAIL)
    return UCOMPLETED;

  if (testres == UTRUE)
  {
    UCommand *tmp_instr2 = 0;
    if (flavor() == UPIPE || flavor() == UAND)
    {
      if (instr2)
	morph =
	  new UCommand_TREE(loc_, flavor(), command->copy(),
			    new UCommand_TREE(loc_, UPIPE,
					      tmp_instr2 = instr2->copy(),
					      this));
      else
	morph = new UCommand_TREE(loc_, flavor(), command->copy(), this);
    }
    else
    {
      if (instr2)
	morph =
	  new UCommand_TREE
	  (loc_, flavor(),
	   new UCommand_TREE(loc_, UAND,
			     new UCommand_TREE
			     (loc_, UPIPE,
			      command->copy(), tmp_instr2 = instr2->copy()),
			     new UCommand_NOOP(loc_)),
	   this);
      else
	morph =
	  new UCommand_TREE
	  (loc_, flavor(),
	   new UCommand_TREE(loc_, UAND, command->copy(),
			     new UCommand_NOOP(loc_)),
	   this);
    }
    if (tmp_instr2)
      tmp_instr2->morphed = true;
    persistant = true;
    return UMORPH;
  }
  else
    return UCOMPLETED;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_FOR::copy() const
{
  UCommand_FOR *ret = new UCommand_FOR(loc_, flavor(),
				       ucopy (instr1),
				       ucopy (test),
				       ucopy (instr2),
				       ucopy (command));
  ret->first = first;
  return copybase(ret);
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_FOR::print_(unsigned l) const
{
  debug(l, "%s", flavor_string());
  DEBUG_ATTR_I(test);
  DEBUG_ATTR_I(instr1);
  DEBUG_ATTR_I(instr2);
  DEBUG_ATTR_I(command);
}

MEMORY_MANAGER_INIT(UCommand_FOREACH);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 */
UCommand_FOREACH::UCommand_FOREACH(const location& l,
				   Flavorable::UNodeType flavor,
				   UVariableName* variablename,
				   UExpression* expression,
				   UCommand* command)
  : UCommand(l, FOREACH),
    Flavorable(flavor),
    variablename (variablename),
    command (command),
    expression (expression),
    position (0),
    list(0),
    firsttime (true)
{
  ADDOBJ(UCommand_FOREACH);
}

//! UCommand subclass destructor.
UCommand_FOREACH::~UCommand_FOREACH()
{
  FREEOBJ(UCommand_FOREACH);
  delete command;
  delete variablename;
  delete expression;
  delete list;
}

//! UCommand subclass execution function
UCommand::Status
UCommand_FOREACH::execute_(UConnection *connection)
{
  if (firsttime)
  {
    firsttime = false;
    list =  expression->eval(this, connection);
    position = list;
    if (position == 0)
      return UCOMPLETED;
    if (position->dataType == DATA_LIST)
      position = position->liststart;
  }

  if (position == 0)
    return UCOMPLETED;

  UExpression* currentvalue =
    new UExpression(loc(), UExpression::VALUE, ufloat(0));
  if (!currentvalue)
    return UCOMPLETED;
  currentvalue->dataType = position->dataType;
  if (position->dataType == DATA_NUM)
    currentvalue->val = position->val;
  if (position->dataType == DATA_STRING)
    currentvalue->str = new UString(*position->str);
  if (position->dataType == DATA_BINARY)
  {
    // add support here
  }
  if (position->dataType != DATA_NUM && position->dataType != DATA_STRING)
  {
    send_error(connection, this, "This type is not supported yet");
    delete currentvalue;
    return UCOMPLETED;
  }

  morph =
    new UCommand_TREE
    (loc_, flavor(),
     new UCommand_TREE(loc_, Flavorable::UPIPE,
		       new UCommand_ASSIGN_VALUE
		       (loc_, variablename->copy(), currentvalue, 0),
		       command->copy()),
     this);
  ((UCommand_TREE*)((UCommand_TREE*)morph)->command1)->command1->setTag(this);

  position = position->next;
  persistant = true;
  return UMORPH;
}

//! UCommand subclass hard copy function
UCommand*
UCommand_FOREACH::copy() const
{
  UCommand_FOREACH *ret = new UCommand_FOREACH(loc_, flavor(),
					       ucopy (variablename),
					       ucopy (expression),
					       ucopy (command));
  copybase(ret);
#if 0
  // FIXME: Why do we change this attribute here?  Is it the one
  // of the result that is meant to be initialized?
  position = 0;
#endif
  return ret;
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_FOREACH::print_(unsigned l) const
{
  debug(l, "%s", flavor_string());
  DEBUG_ATTR (variablename);
  DEBUG_ATTR_I (expression);
  DEBUG_ATTR_I(command);
}

MEMORY_MANAGER_INIT(UCommand_NOOP);
// *********************************************************
//! UCommand subclass constructor.
/*! Subclass of UCommand with standard member initialization.
 If zerotime is true, the noop command will be terminated as soon as it is
 executed. It is a truely empty command, used to structure command trees
 like in the { commands... }  case.
 */
UCommand_NOOP::UCommand_NOOP(const location& l, kind k)
  : UCommand(l, NOOP),
    kind_ (k)
{
  ADDOBJ(UCommand_NOOP);
}

//! UCommand subclass destructor.
UCommand_NOOP::~UCommand_NOOP()
{
  FREEOBJ(UCommand_NOOP);
}

//! UCommand subclass execution function
UCommand::Status
UCommand_NOOP::execute_(UConnection *connection)
{
  switch (kind_)
  {
    case regular:
      if (status == UONQUEUE)
	return !connection->receiving ? URUNNING : UONQUEUE;
      else
	return UCOMPLETED;
    case zerotime:
    case spontaneous:
      return UCOMPLETED;
  }
  // Please GCC.
  abort();
}

//! UCommand subclass hard copy function
UCommand*
UCommand_NOOP::copy() const
{
  return copybase(new UCommand_NOOP(loc_, kind_));
}

//! Print the command
/*! This function is for debugging purpose only.
 It is not safe, efficient or crash proof. A better version will come later.
 */
void
UCommand_NOOP::print_(unsigned) const
{
}
