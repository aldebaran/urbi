/// \file urbi/utimer-table.hh

#ifndef URBI_UTIMER_TABLE_HH
# define URBI_UTIMER_TABLE_HH

# include <list>
# include <libport/singleton-ptr.hh>

# include <urbi/uvalue.hh>

namespace urbi
{
  class UTimerCallback;
  typedef std::list<UTimerCallback*> UTimerTable;

  // Timer and update maps.
  EXTERN_STATIC_INSTANCE_EX(UTimerTable, timermap, USDK_API);
  EXTERN_STATIC_INSTANCE_EX(UTimerTable, updatemap, USDK_API);

} // end namespace urbi

#endif // ! URBI_UTIMER_TABLE_HH
