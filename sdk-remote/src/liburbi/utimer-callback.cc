/*! \file utimer-callback.cc
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

#include <urbi/utimer-callback.hh>
#include <urbi/usyncclient.hh>
#include <urbi/uexternal.hh>

namespace urbi
{
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
