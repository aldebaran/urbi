/*! \file uvariablelist.cc
 *******************************************************************************

 File: uvariablelist.cc\n
 Implementation of the UVariableList class.

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

#include "ucommand.hh"
#include "uconnection.hh"
#include "ucopy.hh"
#include "userver.hh"
#include "uvariablelist.hh"
#include "uvariablename.hh"

// **************************************************************************
//! UVariableList constructor.
UVariableList::UVariableList(UVariableName *variablename,
			     UVariableList* next)
  : variablename (variablename),
    next         (next)
{
  ADDOBJ(UVariableList);
}

//! UVariableList destructor.
UVariableList::~UVariableList()
{
  FREEOBJ(UVariableList);
  delete variablename;
  delete next;
}

//! UVariableList rank function
UVariableList*
UVariableList::rank(int n)
{
  if (n==0)
    return this;
  else if (next == 0)
    return 0;
  else
    return next->rank(n-1);
}

//! UVariableList size function
int
UVariableList::size()
{
  if (next)
    return next->size() + 1;
  else
    return 1;
}

//! UVariableList hard copy function
UVariableList*
UVariableList::copy()
{
  return new UVariableList (ucopy (variablename),
			    ucopy (next));
}

//! Print the list of parameters
/*! This function is for debugging purpose only.
    It is not safe, efficient or crash proof. A better version will come later.
*/
void
UVariableList::print()
{
  if (variablename)
    {
      ::urbiserver->debug("variablename=");
      variablename->print();
      ::urbiserver->debug(" ");
    }
  if (next)
    {
      ::urbiserver->debug(", ");
      next->print();
      ::urbiserver->debug(" ");
    }
}
