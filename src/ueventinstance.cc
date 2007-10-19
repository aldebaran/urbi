/*! \file ueventinstance.cc
 *******************************************************************************

 File: ueventinstance.cc\n
 Implementation of the UEventInstance and UMultiEventInstance class.

 This file is part of
 %URBI Kernel, version __kernelversion__\n
 (c) Gostai S.A.S., 2004-2006

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.net

 **************************************************************************** */
#include "libport/cstdio"
#include <sstream>

#include <boost/foreach.hpp>

#include "libport/containers.hh"

//FIXME for debugging purposes only
#include "kernel/userver.hh"
#include "kernel/utypes.hh"
#include "kernel/uvalue.hh"

#include "uasynccommand.hh"
#include "ueventhandler.hh"
#include "ueventinstance.hh"
#include "ueventmatch.hh"


// **************************************************************************
// UEventInstance

UEventInstance::UEventInstance (UEventMatch *match, UEvent* e):
  e_ (e)
{
  id_ = e_->id ();
  BOOST_FOREACH (UValue* iuv, match->filter ())
    if (iuv->dataType == DATA_VARIABLE)
      filter_.push_back (std::string (iuv->str->c_str()));
    else
      filter_.push_back (std::string ());
}

UEventInstance::UEventInstance (UEventInstance* uei):
  filter_ (uei->filter_),
  e_ (uei->e_),
  id_ (uei->id_)
{
}

UEventInstance::~UEventInstance ()
{
}

bool
UEventInstance::operator== (const UEventInstance& ei)
{
  return ei.id_ == id_;
}


// **************************************************************************
// UMultiEventInstance

UMultiEventInstance::UMultiEventInstance ()
{
}

UMultiEventInstance::UMultiEventInstance (UMultiEventInstance *mei1,
					  UMultiEventInstance *mei2)
{
  BOOST_FOREACH (UEventInstance* i, mei1->instances_)
    instances_.push_back (new UEventInstance (i));

  BOOST_FOREACH (UEventInstance* i, mei2->instances_)
    instances_.push_back (new UEventInstance (i));
}


UMultiEventInstance::~UMultiEventInstance ()
{
  libport::deep_clear(instances_);
}

void
UMultiEventInstance::addInstance(UEventInstance *instance)
{
  instances_.push_back (instance);
}

bool
UMultiEventInstance::operator== (UMultiEventInstance& mei)
{
  if (mei.instances_.size () != instances_.size ())
    return false;

  for (std::list<UEventInstance*>::iterator
	 i1 = instances_.begin (),
	 i2 = mei.instances_.begin ();
       i1 != instances_.end ();
       ++i1, ++i2)
    // Yes, there is no !=.
    if (!(**i1 == **i2))
      return false;

  return true;
}
