/*! \file ufunction.cc
 *******************************************************************************

 File: ufunction.cc\n
 Implementation of the UFunction class.

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

#include <string.h>
#include <stdlib.h>
#include "ufunction.h"
#include "userver.h"
#include "ucommand.h"

UFunction::UFunction(UString *funname,
                     UNamedParameters *parameters,
                     UCommand *command) {
    
  ADDOBJ(UFunction);
  this->funname = funname;
  this->parameters = parameters;
  this->command = command;
}


UFunction::~UFunction() {
  
  FREEOBJ(UFunction);
  if (funname) delete(funname);
  if (parameters) delete(parameters);
  if (command) delete(command);
}

UString*
UFunction::name() {
  
  return(funname);
}

int 
UFunction::nbparam() {
  
  if (!parameters) return(0);
  else
    return(parameters->size());
}

UCommand* 
UFunction::cmdcopy(UString *_tag, UNamedParameters *_flags) {
  
  UCommand* tmpcmd = command->copy();
  if (_tag) {
    if (tmpcmd->tag) delete tmpcmd->tag;
    tmpcmd->tag = _tag->copy();
  }
  if (_flags) {
    if (tmpcmd->flags) delete tmpcmd->flags;
    tmpcmd->flags = _flags->copy();
  }

  return(tmpcmd);
}
