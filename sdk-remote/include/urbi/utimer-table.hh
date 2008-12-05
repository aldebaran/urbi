/// \file urbi/utimer-table.hh

#ifndef URBI_UTIMER_TABLE_HH
# define URBI_UTIMER_TABLE_HH

# include <list>
# include <urbi/fwd.hh>

namespace urbi
{
  typedef std::list<UTimerCallback*> UTimerTable;

  // Timer and update maps.
  URBI_SDK_API UTimerTable& timermap();
  URBI_SDK_API UTimerTable& updatemap();

} // end namespace urbi

#endif // ! URBI_UTIMER_TABLE_HH
