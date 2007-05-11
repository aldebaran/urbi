/*! \file ugroup.cc
 *******************************************************************************

 File: ugroup.cc\n
 Implementation of the UGroup class.

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

#include "kernel/ustring.hh"
#include "kernel/userver.hh"
#include "kernel/uvalue.hh"

#include "ugroup.hh"
#include "uvariablename.hh"

// **************************************************************************
//! UGroup constructor.
UGroup::UGroup (const UString& name)
  : name (name)
{
}

//! UGroup destructor
UGroup::~UGroup()
{
}


#if 0
UValue * UGroup::list( UVariableName *variable)
{
  //do read
  UAlias * gr =  this;

  UValue * val = new UValue();
  val->dataType = DATA_LIST;
  UValue * current = val;
  bool first = true;

  for (std::list<UAlias*>::iterator it = gr->members.begin();
       (it != gr->members.end()) && current ;
       ++it)
    {
      UValue *n;
      if ((*it)->members.empty())
	{
	  //terminal group, handle it for him
	  //child node
	  char vname[1024];
	  strcpy(vname, (*it)->device->c_str());
	  strcat(vname, ".");
	  strcat(vname, variable->method->c_str());
	  if ( ::urbiserver->variabletab.find(vname) ==  ::urbiserver->variabletab.end())
	    //no variable? could be...
	    n=new UValue("null");
	  else
	    n = ::urbiserver->variabletab[vname]->get()->copy();
	}

      else
	n =  ::urbiserver->grouptab[(*it)->device->c_str()]->list(variable);

      //    while (n && n->dataType == DATA_LIST)
      {
	//      UValue * nn = n->list;
	//      n->list=  0;
	//      delete n;
	//      n=nn;
	//    }

	if (first)
	  current->liststart =n;
	else
	  current->next = n;
	current = n;

	//current->list = n;
	//    while (current->list)
	//      current = current->list;

	first = false;
      }
      return val;
    }
#endif
