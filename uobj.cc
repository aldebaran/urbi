/*! \file uobj.cc
 *******************************************************************************

 File: uobj.cc\n
 Implementation of the UObj class.

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

#include "uobj.h"
#include "ustring.h"
#include "uvalue.h"
#include "uvariablename.h"
#include "userver.h"
                                                       	
// **************************************************************************	
//! UObj constructor.
UObj::UObj (UString *device)
{
  this->device = new UString(device);

  ::urbiserver->objtab[this->device->str()] = this;
}

//! UObj destructor
UObj::~UObj()
{
  if (device) delete(device);
}


