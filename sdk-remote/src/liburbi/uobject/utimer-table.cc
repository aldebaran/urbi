/// \file urbi/utimer-table.cc

#include <urbi/utimer-table.hh>

namespace urbi
{
  // Timer and update maps.
  STATIC_INSTANCE(UTimerTable, timermap);
  STATIC_INSTANCE(UTimerTable, updatemap);
}
