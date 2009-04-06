/// \file libuobject/utimer-callback.cc

#include <iostream>
#include <sstream>
#include <list>

#include <libport/lexical-cast.hh>
#include <libport/program-name.hh>

#include <liburbi/compatibility.hh>

#include <urbi/utimer-callback.hh>
#include <urbi/uobject.hh>
#include <urbi/usyncclient.hh>
#include <urbi/uexternal.hh>

namespace urbi
{
  UTimerCallback::UTimerCallback(const std::string& objname,
				 ufloat period,
                                 UTimerTable& tt)
    : period(period)
    , objname(objname)
  {
    tt.push_back(this);
    lastTimeCalled = -9999999;

    // Register ourself as an event.
    std::string cbname = "timer" + string_cast(tt.size());
    std::string event = objname + "." + cbname;
    createUCallback(objname, "event", this, &UTimerCallback::call,
		    event, eventmap(), false);
    URBI_SEND_COMMAND("timer_" << objname << ": every(" << period << "ms)"
                      "{ " << compatibility::emit(event) << ";}");
  }

  UTimerCallback::~UTimerCallback()
  {
  }

} // namespace urbi
