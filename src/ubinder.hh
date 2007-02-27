/*! \file ubinder.hh
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

#ifndef UBINDER_HH
# define UBINDER_HH

# include <list>

# include "fwd.hh"
# include "ustring.hh"
# include "utypes.hh"

// ****************************************************************************
//! Contains a binder definition, as a result of a BINDER command
//! A binder associates a var name (function, event or var) to a callback
//! mechanism that must be called. Only for external mode.
class UBinder
{
public:

  UBinder(const UString& objname, const UString& id,
	  UBindMode bindMode, UBindType type, int nbparam,
	  UConnection* c);
  ~UBinder();

  UString      id;
  UBindMode    bindMode;
  UBindType    type;
  int          nbparam;

  typedef std::list<UMonitor*> monitors_type;
  monitors_type monitors;

  void addMonitor(const UString& objname, UConnection *c);

  bool removeMonitor(const UString& objname, UConnection *c);
  bool removeMonitor(const UString& objname);
  bool removeMonitor(UConnection *c);

  UMonitor* locateMonitor(UConnection *c);
};

class UMonitor
{
public:
  UMonitor(UConnection *c);
  UMonitor(const UString& objname, UConnection *c);
  ~UMonitor();

  void addObject(const UString& objname);
  bool removeObject(const UString& objname);

  UConnection* c;
  std::list<UString> objects;
};

#endif
