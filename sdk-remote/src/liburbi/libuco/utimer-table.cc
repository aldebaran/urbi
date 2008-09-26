/// \file urbi/utimer-table.cc

#include <urbi/utimer-table.hh>

// Timer and update maps.
  STATIC_INSTANCE_NS_EX(UTimerTable, timermap, urbi, USDK_API);
  STATIC_INSTANCE_NS_EX(UTimerTable, updatemap, urbi, USDK_API);
