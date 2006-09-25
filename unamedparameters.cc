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

#include <math.h>

#include "unamedparameters.h"
#include "ucommand.h"
#include "uconnection.h"
#include "userver.h"
                                      
	
// **************************************************************************	
//! UNamedParameters constructor.
UNamedParameters::UNamedParameters(UString* name, 
                                   UExpression *expression, 
                                   UNamedParameters* next) 
{
  ADDOBJ(UNamedParameters);
  this->name       = name;
  this->expression = expression;
  this->next       = next;
}

//! UNamedParameters constructor (for expression list only)
UNamedParameters::UNamedParameters(UExpression *expression, 
                                   UNamedParameters* next) 
{
  ADDOBJ(UNamedParameters);
  this->name       = 0;
  this->expression = expression;
  this->next       = next;	
}

//! UNamedParameters destructor.
UNamedParameters::~UNamedParameters()
{
  FREEOBJ(UNamedParameters);
  if (name) delete(name);
  if (expression) delete expression;
  if (next) delete next;
}

UNamedParameters* 
UNamedParameters::rank(int n)
{
  if (n==0) return (this);
  else
    if (next == 0) return 0;
    else
      return (next->rank(n-1));
}

int 
UNamedParameters::size()
{
  if (next) return (next->size() + 1);
  else 
    return(1);
}

//! UNamedParameters hard copy function
UNamedParameters*
UNamedParameters::copy()
{
  UNamedParameters* ret = new UNamedParameters((UExpression*)0,
                                               (UNamedParameters*)0);  

  if (expression)  ret->expression = expression->copy(); 
  if (name)        ret->name = new UString(name);
  if (next)        ret->next = next->copy();

  return (ret);
}

//! Print the list of parameters
/*! This function is for debugging purpose only. 
    It is not safe, efficient or crash proof. A better version will come later.
*/
void 
UNamedParameters::print()
{
  if (name) ::urbiserver->debug("%s:",name->str());
  if (expression) {::urbiserver->debug("expr="); expression->print(); ::urbiserver->debug(" ");}
  if (next) {::urbiserver->debug(", "); next->print(); ::urbiserver->debug(" ");} 
}

