/// \file urbi/utimer-table.cc

#include <urbi/utimer-table.hh>

// Timer and update maps.
  STATIC_INSTANCE_NS_EX(UTimerTable, timermap, urbi, URBI_SDK_API);
  STATIC_INSTANCE_NS_EX(UTimerTable, updatemap, urbi, URBI_SDK_API);
