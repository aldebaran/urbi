/*! \file uobj.hh
 *******************************************************************************

 File: uobj.h\n
 Definition of the UObj class.

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

#ifndef UOBJ_HH
# define UOBJ_HH

# include <list>

# include "kernel/fwd.hh"

//! Contains a group definition, as a result of a GROUP command
class UObj
{
public:
  UObj(UString *device);
  ~UObj();

  std::list<UObj*> down;
  std::list<UObj*> up;

  UString* device;
  UBinder* binder;
  urbi::baseURBIStarter* internalBinder;

  UFunction* searchFunction(const char* id, bool& ambiguous) const;
  UVariable* searchVariable(const char* id, bool& ambiguous) const;
  UEventHandler* searchEvent(const char* id, bool& ambiguous) const;
};

#endif
