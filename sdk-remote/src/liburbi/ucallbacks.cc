/*! \file uobject.cc
 *******************************************************************************

 File: uobject.cc\n
 Implementation of the UObject class.

 This file is part of LIBURBI\n
 (c) Jean-Christophe Baillie, 2004-2006.

 Permission to use, copy, modify, and redistribute this software for
 non-commercial use is hereby granted.

 This software is provided "as is" without warranty of any kind,
 either expressed or implied, including but not limited to the
 implied warranties of fitness for a particular purpose.

 For more information, comments, bug reports: http://www.urbiforge.com

 **************************************************************************** */

#include <iostream>
#include <sstream>
#include <list>

#include <libport/program-name.hh>

#include <urbi/ucallbacks.hh>
#include <urbi/usyncclient.hh>
#include <urbi/uexternal.hh>

namespace urbi
{

  // **************************************************************************
  //! UGenericCallback constructor.
  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string& type,
				     const std::string& name,
				     int size, UTable &, bool)
    : objname (objname), name (name)
  {
    nbparam = size;

    if (type == "function" || type== "event" || type == "eventend")
    {
      std::ostringstream oss;
      oss << size;
      this->name += "__" + oss.str();
    }

    std::cerr << libport::program_name
	      << ": Registering " << type << " " << name << " " << size
	      << " into " << this->name
	      << " from " << objname
	      << std::endl;

    if (type == "var")
      URBI(()) << "external " << type << " "
	       << name << " from " << objname << ";";

    if (type == "event" || type == "function")
      URBI(()) << "external " << type << "(" << size << ") "
	       << name << " from " << objname << ";";

    if (type == "varaccess")
      echo("Warning: NotifyAccess facility is not available for modules in "
	   "remote mode.\n");
  };

  //! UGenericCallback constructor.
  UGenericCallback::UGenericCallback(const std::string& objname,
				     const std::string& type,
				     const std::string& name, UTable&)
    : objname(objname), name(name)
  {
    URBI(()) << "external " << type << " " << name << ";";
  };

  UGenericCallback::~UGenericCallback()
  {
  };

  void
  UGenericCallback::registerCallback(UTable& t)
  {
     t[this->name].push_back(this);
  }

  // **************************************************************************
  //! UTimerCallbacl constructor.

  UTimerCallback::UTimerCallback(const std::string& objname,
				 ufloat period, UTimerTable& tt)
    : period(period),
      objname(objname)
  {
    tt.push_back(this);
    lastTimeCalled = -9999999;
    std::ostringstream os;
    os << "timer"<< tt.size();
    //register oursselves as an event
    std::string cbname = os.str();

    createUCallback(objname, "event", this, &UTimerCallback::call,
		    objname + "." + cbname, eventmap, false);
    //new UCallbackvoid0<UTimerCallback> (objname, "event", this,
    //				&UTimerCallback::call,
    //				objname + '.' + cbname, eventmap);

    os.str("");
    os.clear();
    os << "timer_" << objname << ": every(" << period << ") { emit "
       << (objname + '.' + cbname) << ";};";
    URBI(()) << os.str();
  }

  UTimerCallback::~UTimerCallback()
  {
  }

} // namespace urbi
