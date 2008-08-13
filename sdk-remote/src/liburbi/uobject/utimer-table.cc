/// \file urbi/utimer-table.cc

#include <urbi/utimer-table.hh>

namespace urbi
{
  // Timer and update maps.
  STATIC_INSTANCE_(UTimerTable, timermap);
  STATIC_INSTANCE_(UTimerTable, updatemap);
}
