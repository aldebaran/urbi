/*! \file ueventmatch.cc
 *******************************************************************************

 File: ueventmatch.cc\n
 Implementation of the UEventMatch class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */
#include "libport/cstdio"
#include <sstream>

#include "libport/containers.hh"

#include "ueventmatch.hh"
#include "uasynccommand.hh"
#include "utypes.hh"
#include "userver.hh"

UEventMatch* kernel::eventmatch_true;
UEventMatch* kernel::eventmatch_false;

// **************************************************************************
// UEventMatch

UEventMatch::UEventMatch (UString* eventname,
			  UNamedParameters* filter,
			  UCommand* command,
			  UConnection* connection)
   :  eventhandler_ (kernel::findEventHandler (eventname,
					       filter ? filter->size () : 0)),
      state_ (true), // default is positive event
      deleteable_(true) // can be deleted
{
  // Build the args list by evaluating the UNamedParameters
  for (UNamedParameters* param = filter; param; param = param->next)
  {
    UValue* e1;
    if (param->expression->type == UExpression::VARIABLE)
    {
      UString* varname = 0;
      ASSERT (param->expression->variablename)
	varname = param->expression->variablename->
		 buildFullname (command, connection);
      ASSERT (varname);
      e1 = new UValue (varname->str ());
      // this is a dirty hack. It means that the UValue does not
      // contain a value, but a variable name instead.
      e1->dataType = DATA_VARIABLE;
    }
    else
      e1 = param->expression->eval (command, connection);
    ASSERT (e1)
      filter_.push_back (e1);
  }

  // applies the filter to known events
  findMatches_ ();
}

UEventMatch::UEventMatch (UEventHandler* eh)
  : eventhandler_ (eh),
    state_ (true), // default is positive event
    deleteable_ (false) // cannot be deleted, this is a system eventmatch
{
  // applies an empty filter to all events in eh
  findMatches_ ();
}

UEventMatch::~UEventMatch ()
{
  libport::deep_clear(filter_);
}

void
UEventMatch::findMatches_ ()
{
  if (!eventhandler_)
    return;

  for (std::list<UEvent*>::iterator
	 itevent = eventhandler_->eventlist().begin ();
       itevent != eventhandler_->eventlist().end ();
       ++itevent)
  {
    bool ok = true;
    std::list<UValue*>::iterator
      ifilter_arg = filter_.begin (),
      itevent_arg = (*itevent)->args().begin();

    while (ifilter_arg != filter_.end ()
	   && itevent_arg != (*itevent)->args().end ()
	   && ok)
    {
      if ((*ifilter_arg)->dataType != DATA_VARIABLE
	  && !(*ifilter_arg)->equal (*itevent_arg))
	ok = false;

      ++ifilter_arg;
      ++itevent_arg;
    }

    if (ok)
      matches_.push_back (*itevent);
  }
}

void
UEventMatch::reduce (bool st)
{
  for (std::list<UEvent*>::iterator itevent = matches_.begin ();
       itevent != matches_.end ();
       )
    if ((*itevent)->toDelete() == st)
      itevent = matches_.erase (itevent);
    else
      ++itevent;
}

void
UEventMatch::reduce ()
{
  reduce (state_);
}
