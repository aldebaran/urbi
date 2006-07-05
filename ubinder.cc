/*! \file ubinder.cc
 *******************************************************************************

 File: ubinder.cc\n
 Implementation of the UBinder class.

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
#include <algorithm>
using std::find;
#include "ubinder.h"
#include "ustring.h"
#include "uvalue.h"
#include "uvariablename.h"
#include "userver.h"
                                                       	
// **************************************************************************	
//! UBinder constructor.
UBinder::UBinder (UString *id, UBindMode bindMode,
		  UBindType type, int nbparam,
		  UConnection* c)
{
  this->id       = new UString(id);
  this->bindMode = bindMode;
  this->type     = type;
  this->nbparam  = nbparam;
  addMonitor(c);
}

//! UBinder destructor
UBinder::~UBinder()
{
  if (id) delete(id);
}

//! Add a monitoring connection to the list if it is not already there
void UBinder::addMonitor(UConnection *c)
{
  list<UConnection *>::iterator monitorit = find (monitors.begin(),
  monitors.end(), c);
  if (monitorit == monitors.end())
    monitors.push_back(c);  
}

//! Remove a monitoring connection.
/** Returns true if the UBinder itself can be freed
  */
bool UBinder::removeMonitor(UConnection *c)
{ 
  monitors.remove(c);
  return( monitors.empty() );
}
/*
void UBinder::applyParameters(UNamedParameters *param)
{
  for (list<UConnection *>::iterator monitorit = monitors.begin();
      monitorit != monitors.end();
      monitorit++) {
    
  }
}
*/


