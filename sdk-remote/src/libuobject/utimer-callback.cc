/// \file libuobject/utimer-callback.cc

#include <iostream>
#include <sstream>
#include <list>

#include <libport/program-name.hh>

#include <urbi/utimer-callback.hh>
#include <urbi/uobject.hh>
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

    URBI_SEND_COMMAND("timer_" << objname << ": every(" << period << ") { emit "
		      << (objname + '.' + cbname) << ";}");
  }

  UTimerCallback::~UTimerCallback()
  {
  }

} // namespace urbi
