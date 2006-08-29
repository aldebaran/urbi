/*! \file ubinder.h
 *******************************************************************************

 File: ubinder.h\n
 Definition of the UBinder class.

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

#ifndef UBINDER_H_DEFINED
#define UBINDER_H_DEFINED

#include "ustring.h"
#include "utypes.h"
#include <list>
using  std::list;

class UNamedParameters;
class UConnection;
class UVariableName;
class UValue;

// *****************************************************************************
//! Contains a binder definition, as a result of a BINDER command
//! A binder associates a var name (function, event or var) to a callback
//! mechanism that must be called, either internal of external
class UBinder
{
public:

  UBinder(UString *id, UBindMode bindMode, UBindType type, int nbparam,
  	UConnection* c);
  ~UBinder(); 
  
  UString      *id;
  UBindMode    bindMode;
  UBindType    type;
  int          nbparam;
  list<UConnection*> monitors;

  void addMonitor(UConnection *c);
  bool removeMonitor(UConnection *c);
};

#endif
