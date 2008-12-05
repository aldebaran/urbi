/// \file urbi/utimer-table.cc

#include <urbi/utimer-table.hh>

namespace urbi
{
  // Timer and update maps.
  UTimerTable& timermap()
  {
    static UTimerTable instance;
    return instance;
  }

  UTimerTable& updatemap()
  {
    static UTimerTable instance;
    return instance;
  }

} // end namespace urbi
