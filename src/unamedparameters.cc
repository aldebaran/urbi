/*! \file unamedparameters.cc
 *******************************************************************************

 File: unamedparameters.cc\n
 Implementation of the UNamedParameters class.

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

#include "ucopy.hh"
#include "uexpression.hh"
#include "unamedparameters.hh"
#include "userver.hh"
#include "ustring.hh"
#include "utypes.hh"

// **************************************************************************
//! UNamedParameters constructor.
UNamedParameters::UNamedParameters(UString* name,
				   UExpression *expression,
				   UNamedParameters* next)
  : name       (name),
    expression (expression),
    next       (next),
    notifyEnd  (false)
{
  ADDOBJ(UNamedParameters);
}

//! UNamedParameters constructor (for expression list only)
UNamedParameters::UNamedParameters(UExpression *expression,
				   UNamedParameters* next)
  : name       (0),
    expression (expression),
    next       (next),
    notifyEnd  (false)
{
  ADDOBJ(UNamedParameters);
}

//! UNamedParameters destructor.
UNamedParameters::~UNamedParameters()
{
  FREEOBJ(UNamedParameters);
  delete name;
  delete expression;
  delete next;
}

UNamedParameters*
UNamedParameters::rank(int n)
{
  if (n==0)
    return this;
  else if (next == 0)
    return 0;
  else
    return next->rank(n-1);
}

int
UNamedParameters::size() const
{
  if (next)
    return next->size() + 1;
  else
    return 1;
}

//! UNamedParameters hard copy function
UNamedParameters*
UNamedParameters::copy() const
{
  return
    new UNamedParameters(ucopy (name),
			 ucopy (expression),
			 ucopy (next));
}

//! Print the list of parameters
/*! This function is for debugging purpose only.
    It is not safe, efficient or crash proof. A better version will come later.
*/
void
UNamedParameters::print() const
{
  if (name)
    debug("%s:", name->str());
  if (expression)
  {
    debug("expr=");
    expression->print(0);
    debug(" ");
  }
  if (next)
  {
    debug(", ");
    next->print();
    debug(" ");
  }
}
