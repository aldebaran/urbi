/*! \file ucallid.cc
 *******************************************************************************

 File: ucallid.cc\n
 Implementation of the UCallid class.

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

#include "ucallid.h"
#include "ustring.h"
#include "ucommand.h"
                                                       	
// **************************************************************************	
//! UCallid constructor.
UCallid::UCallid (const char *fun_id, UCommand_TREE* root)
{
  this->fun_id = new UString(fun_id);
  this->root = root;
}

//! UCallid destructor
UCallid::~UCallid()
{
  if (fun_id) delete(fun_id);
  
  for (list<UVariable*>::iterator iter = stack.begin();
       iter != stack.end();iter++) 
    delete ((*iter));

  stack.clear();
}

//! Add a variable to the list of variable to liberate
void
UCallid::store(UVariable *variable)
{
  stack.push_front(variable);
}

//! Access to the call ID
const char*
UCallid::str()
{
  return (fun_id->str());
}
