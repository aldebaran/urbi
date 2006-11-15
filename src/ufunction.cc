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

#include "libport/cstring"
#include <cstdlib>
#include "ufunction.hh"
#include "userver.hh"
#include "ucommand.hh"

UFunction::UFunction(UString *funname,
		     UNamedParameters *parameters,
		     UCommand *command)
		     {
  ADDOBJ(UFunction);
  this->funname = funname;
  this->parameters = parameters;
  this->command = command;
}


UFunction::~UFunction()
{
  FREEOBJ(UFunction);
  delete funname;
  delete parameters;
  delete command;
}

UString*
UFunction::name()
{
  return(funname);
}

int
UFunction::nbparam()
{
  if (!parameters) return(0);
  else
    return(parameters->size());
}

UCommand*
UFunction::cmdcopy(std::string _tag, UNamedParameters *_flags)
{
  UCommand* tmpcmd = command->copy();
  if (_tag != "")
    tmpcmd->setTag(_tag);
  if (_flags)
    {
      delete tmpcmd->flags;
      tmpcmd->flags = _flags->copy();
    }

  return(tmpcmd);
}
