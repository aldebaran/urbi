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

#include <cassert> 
#include <iostream>
#include "userver.hh"
#include "libport/containers.hh"
#include "fwd.hh"
#include "utypes.hh"
#include "ucallid.hh"
#include "uvariable.hh"

// **************************************************************************
//! UCallid constructor.
UCallid::UCallid (const char *f, const char *s, UCommand_TREE* r)
  : returnVar (0),
    fun_id (f),
    self_id (s),
    root (r),
    dying(false)
{
}

//! UCallid destructor
UCallid::~UCallid()
{
  dying = true;
  libport::deep_clear (stack);
}

//! Add a variable to the list of variable to liberate
void
UCallid::store(UVariable *variable)
{
  stack.push_front(variable);
  variable->setContext(this);
}


//! Remove a variable from the list of variable to liberate
void
UCallid::remove(UVariable *variable)
{
  if (dying)
    return; //ignore remove call triggered by our dtor calling deep_clear
  stack.remove(variable);
}

//! Access to the call ID
const char*
UCallid::str()
{
  return fun_id.str();
}

//! Access to the call self ref
const char*
UCallid::self()
{
  return self_id.str();
}

//! Set the returnVar in a function call
void
UCallid::setReturnVar (UVariable *v)
{
  returnVar = v;
  store (returnVar);
}
