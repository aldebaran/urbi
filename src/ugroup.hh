/*! \file ugroup.hh
 *******************************************************************************

 File: ugroup.h\n
 Definition of the UGroup class.

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

#ifndef UGROUP_HH
#define UGROUP_HH

#include <list>

#include "fwd.hh"
#include "ustring.hh"

// *****************************************************************************
//! Contains a group definition, as a result of a GROUP command
class UGroup
{
public:

  UGroup(UString *name);
  UGroup(char *name);

  ~UGroup();

  std::list<UString*> members;
  UString      *name;
};

#endif
