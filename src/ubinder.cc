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
#include "libport/containers.hh"

#include "ubinder.hh"
#include "ustring.hh"
#include "uvalue.hh"
#include "uvariablename.hh"
#include "userver.hh"

// **************************************************************************
//! UBinder constructor.
UBinder::UBinder (const UString& objname,
                  const UString& id,
                  UBindMode bindMode,
		  UBindType type, int nbparam,
		  UConnection* c)
  : id (id),
    bindMode(bindMode),
    type(type),
    nbparam (nbparam)
{
  addMonitor(objname, c);
}

//! UBinder destructor
UBinder::~UBinder ()
{
  libport::deep_clear (monitors);
}

//! Add a monitoring connection to the list if it is not already there
void
UBinder::addMonitor (const UString& objname, UConnection *c)
{
  UMonitor *m = 0;

  for (monitors_type::iterator i = monitors.begin();
       i != monitors.end() && !m;
       ++i)
    if ((*i)->c == c)
      m = *i;

  if (!m)
  {
    m = new UMonitor(objname, c);
    monitors.push_back(m);
  }
  else
    m->addObject(objname);
}

//! Locate a monitor based on its connection or zero if not found
UMonitor*
UBinder::locateMonitor (UConnection *c)
{
  UMonitor *m = 0;

  // locate the connection
  for (monitors_type::iterator i = monitors.begin();
       i != monitors.end();
       ++i)
  {
    if ((*i)->c == c)
      m = (*i);
  }

  return m;
}

//! Remove a monitoring connection.
/** Returns true if the UBinder itself can be freed
  */
bool
UBinder::removeMonitor (const UString& objname, UConnection *c)
{
  UMonitor *m = locateMonitor(c);
  if (!m)
    return false;

  if (m->removeObject(objname))
  {
    monitors.remove(m);
    delete m;
  }

  return monitors.empty();
}

//! Remove a monitoring connection.
/** Returns true if the UBinder itself can be freed
  */
bool
UBinder::removeMonitor(UConnection *c)
{
  UMonitor *m = locateMonitor(c);
  if (!m)
    return false;

  monitors.remove(m);
  delete m;
  return monitors.empty();
}

//! Remove a monitored object
/** Returns true if the UBinder itself can be freed
  */
bool
UBinder::removeMonitor (const UString& objname)
{
  for (monitors_type::iterator i = monitors.begin();
       i != monitors.end();
       )
    if ((*i)->removeObject(objname))
    {
      delete *i;
      i = monitors.erase(i);
    }
    else
      ++i;

  return monitors.empty();
}


/*-----------.
| UMonitor.  |
`-----------*/

//! UMonitor constructor
UMonitor::UMonitor(UConnection *c): c(c)
{
}

//! UMonitor constructor
UMonitor::UMonitor(const UString& objname, UConnection *c)
  : c (c)
{
  addObject(objname);
}

//! UMonitor destructor
UMonitor::~UMonitor()
{
  libport::deep_clear (objects);
}

//! UMonitor addObject
void
UMonitor::addObject(const UString& objname)
{
  objects.push_back(new UString(objname));
}

//! Monitor removeObject, returns true if objects is empty
bool
UMonitor::removeObject(const UString& objname)
{
  for (std::list<UString*>::iterator i = objects.begin();
       i != objects.end();
       ++i)
    if (objname.equal(*i))
    {
      objects.remove(*i);
      delete *i;
      return objects.empty();
    }
  return false;
}
