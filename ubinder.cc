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

#include "ubinder.h"
#include "ustring.h"
#include "uvalue.h"
#include "uvariablename.h"
#include "userver.h"

// **************************************************************************
//! UBinder constructor.
UBinder::UBinder (UString *objname, UString *id, UBindMode bindMode,
		  UBindType type, int nbparam,
		  UConnection* c)
{
  this->id       = new UString(id);
  this->bindMode = bindMode;
  this->type     = type;
  this->nbparam  = nbparam;
  addMonitor(objname, c);
}

//! UBinder destructor
UBinder::~UBinder ()
{
  if (id)
    delete id;

  for (std::list<UMonitor*>::iterator mit = monitors.begin();
      mit != monitors.end();
      mit++)
    delete (*mit);
}

//! Add a monitoring connection to the list if it is not already there
void
UBinder::addMonitor (UString *objname, UConnection *c)
{
  UMonitor *m = 0;

  for (std::list<UMonitor*>::iterator mit = monitors.begin();
       mit != monitors.end() && !m;
       mit++)
  {
    if ((*mit)->c == c)
      m = (*mit);
  }//for

  if (!m)
  {
    m = new UMonitor(objname,c);
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
  for (std::list<UMonitor*>::iterator mit = monitors.begin();
       mit != monitors.end();
       mit++)
  {
    if ((*mit)->c == c)
      m = (*mit);
  }//for

  return (m);
}

//! Remove a monitoring connection.
/** Returns true if the UBinder itself can be freed
  */
bool
UBinder::removeMonitor (UString *objname, UConnection *c)
{
  UMonitor *m = locateMonitor(c);
  if (!m) return false;

  if (m->removeObject(objname))
  {
    monitors.remove(m);
    delete m;
  }//if

  return( monitors.empty() );
}

//! Remove a monitoring connection.
/** Returns true if the UBinder itself can be freed
  */
bool
UBinder::removeMonitor(UConnection *c)
{
  UMonitor *m = locateMonitor(c);
  if (!m) return false;

  monitors.remove(m);
  delete m;
  return( monitors.empty() );
}

//! Remove a monitored object
/** Returns true if the UBinder itself can be freed
  */
bool
UBinder::removeMonitor (UString *objname)
{
  for (std::list<UMonitor*>::iterator mit = monitors.begin();
       mit != monitors.end();
       )
  {
    if ((*mit)->removeObject(objname))
    {
      delete (*mit);
      mit = monitors.erase(mit);
    }//if
    else
      mit++;
  }
  return( monitors.empty() );
}


/*****************************************************************/
/* UMonitor */
/*****************************************************************/

//! UMonitor constructor
UMonitor::UMonitor(UConnection *c): c(c)
{
}

//! UMonitor constructor
UMonitor::UMonitor(UString *objname, UConnection *c) :
  c(c)
{
  addObject(objname);
}

//! UMonitor destructor
UMonitor::~UMonitor()
{
  for (std::list<UString*>::iterator sit = objects.begin();
       sit != objects.end();
       sit++)
    delete (*sit);
}

//! UMonitor addObject
void
UMonitor::addObject(UString *objname)
{
  objects.push_back(new UString(objname));
}

//! Monitor removeObject, returns true if objects is empty
bool
UMonitor::removeObject(UString *objname)
{
  // locate the object
  UString *s = 0;
  for (std::list<UString*>::iterator sit = objects.begin();
       sit != objects.end() && !s;
       sit++)
  {
    if (objname->equal(*sit))
      s = (*sit);
  }//for

  if (!s) return (false);

  objects.remove(s);
  delete s;

  return( objects.empty() );
}
