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

#include "ucallid.hh"
#include "ustring.hh"
#include "ucommand.hh"

// **************************************************************************
//! UCallid constructor.
UCallid::UCallid (const char *fun_id, const char *self_id, UCommand_TREE* root)
{
  this->fun_id = new UString(fun_id);
  this->self_id = new UString(self_id);
  this->root = root;

  returnVar = 0;
}

//! UCallid destructor
UCallid::~UCallid()
{
  delete fun_id;
  delete self_id;

  for (std::list<UVariable*>::iterator iter = stack.begin();
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

//! Access to the call self ref
const char*
UCallid::self()
{
  return (self_id->str());
}

//! Set the returnVar in a function call
void
UCallid::setReturnVar (UVariable *v)
{
  returnVar = v;
  store (returnVar);
}

