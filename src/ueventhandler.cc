/*! \file ueventhandler.cc
 *******************************************************************************

 File: ueventhandler.cc\n
 Implementation of the UEventHanlder class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */

#include <sstream>

#include <boost/foreach.hpp>

#include "libport/containers.hh"

#include "kernel/utypes.hh"
#include "kernel/userver.hh"
#include "kernel/uvalue.hh"

#include "ueventhandler.hh"
#include "uexpression.hh"
#include "unamedparameters.hh"

std::string
kernel::forgeName (UString* name, int nbarg)
{
  if (!name)
    return std::string("error");

  std::stringstream s;
  s << name->c_str() << "|" << nbarg;
  return s.str();
}

UEventHandler*
kernel::findEventHandler(UString* name, int nbarg)
{
  HMemittab::iterator itevent = ::urbiserver->
    emittab.find(kernel::forgeName(name, nbarg).c_str());

  if (itevent != ::urbiserver->emittab.end())
    return itevent->second;

  return 0;
}

bool
kernel::eventSymbolDefined (const char* symbol)
{
  //FIXME this is a quick hack but must be optimized with an independent hash
  // table: there must be a boolean table that stores the fact that a given
  // event name is in use or not.

  HMemit2tab::iterator i = ::urbiserver->emit2tab.find(symbol);
  return (i!= ::urbiserver->emit2tab.end());
}

bool
kernel::isCoreFunction (UString *fullname)
{
  return (false
	  || *fullname == "abs"
	  || *fullname == "acos"
	  || *fullname == "asin"
	  || *fullname == "atan"
	  || *fullname == "atan2"
	  || *fullname == "cat"
	  || *fullname == "cos"
	  || *fullname == "cpuload"
	  || *fullname == "eval"
	  || *fullname == "exec"
	  || *fullname == "exp"
	  || *fullname == "freemem"
	  || *fullname == "getIndex"
	  || *fullname == "head"
	  || *fullname == "isdef"
	  || *fullname == "isvoid"
	  || *fullname == "load"
	  || *fullname == "loadwav"
	  || *fullname == "log"
	  || *fullname == "power"
	  || *fullname == "random"
	  || *fullname == "round"
	  || *fullname == "save"
	  || *fullname == "sgn"
	  || *fullname == "sin"
	  || *fullname == "size"
	  || *fullname == "sqr"
	  || *fullname == "sqrt"
	  || *fullname == "string"
	  || *fullname == "strlen"
	  || *fullname == "strsub"
	  || *fullname == "tail"
	  || *fullname == "tan"
	  || *fullname == "time"
	  || *fullname == "trunc"
    );
}

UEventHandler* kernel::eh_system_alwaystrue;
UEventHandler* kernel::eh_system_alwaysfalse;
UEvent* kernel::system_alwaystrue;

// **************************************************************************
// UEvent

UEvent::UEvent (UEventHandler* eventhandler,
		std::list<UValue*>& args):
  eventhandler_ (eventhandler),
  args_ (args)
{
  toDelete_ = false;
  id_ = unic ();
}

UEvent::~UEvent()
{
  libport::deep_clear(args_);
}


// **************************************************************************
// UEventHandler

UEventHandler::UEventHandler (UString* name, int nbarg)
  : UASyncRegister(),
    nbarg_ (nbarg)
{
  unforgedName = new UString (*name);
  ::urbiserver->emit2tab[unforgedName->str().c_str()];

  name_ = kernel::forgeName(name, nbarg);
  ::urbiserver->emittab[name_.c_str ()] = this;
}

UEventHandler::~UEventHandler()
{
}

UEvent*
UEventHandler::addEvent(UNamedParameters* parameters,
			UCommand* command,
			UConnection* connection)
{
  std::list<UValue*> args;
  for (UNamedParameters* param = parameters; param; param = param->next)
    if (UValue* e1 = param->expression->eval (command, connection))
      args.push_back (e1);
    else
      return 0;
  UEvent* e = new UEvent(this, args);
  ASSERT(e)
    eventlist_.push_back(e);

  // triggers associated commands update
  updateRegisteredCmd ();

  return e;
}

UEvent*
UEventHandler::addEvent(UEvent* e)
{
  ASSERT(e)
    eventlist_.push_back(e);
  return e;
}


bool
UEventHandler::noPositive ()
{
  BOOST_FOREACH (UEvent* ie, eventlist_)
    if (!ie->toDelete ())
      return false;

  return true;
}

void
UEventHandler::removeEvent(UEvent* event)
{
  eventlist_.remove(event);

  // triggers associated commands update
  updateRegisteredCmd ();
}
