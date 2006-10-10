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

#include <list>

#include "fwd.hh"
#include "ustring.h"
#include "utypes.h"

// *****************************************************************************
//! Contains a binder definition, as a result of a BINDER command
//! A binder associates a var name (function, event or var) to a callback
//! mechanism that must be called. Only for external mode.
class UBinder
{
public:

  UBinder(UString *objname, UString *id, UBindMode bindMode, UBindType type, int nbparam,
  	UConnection* c);
  ~UBinder();

  UString      *id;
  UBindMode    bindMode;
  UBindType    type;
  int          nbparam;
  std::list<UMonitor*> monitors;

  void addMonitor(UString *objname, UConnection *c);

  bool removeMonitor(UString *objname, UConnection *c);
  bool removeMonitor(UString *objname);
  bool removeMonitor(UConnection *c);

  UMonitor* locateMonitor(UConnection *c);
};

class UMonitor
{
public:

  UMonitor(UConnection *c);
  UMonitor(UString *objname, UConnection *c);
  ~UMonitor();

  void addObject(UString *objname);
  bool removeObject(UString *objname);

  UConnection* c;
  std::list<UString*> objects;
};

#endif
