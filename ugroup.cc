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

#include "ugroup.h"
#include "ustring.h"
                                                       	
// **************************************************************************	
//! UGroup constructor.
UGroup::UGroup (UString *device)
{
  this->device = new UString(device);
}

//! UGroup destructor
UGroup::~UGroup()
{
  if (device) delete(device);
}
