#ifndef URBI_UOBJECT_H
# define URBI_UOBJECT_H

// There is no macro expansion after #warning, we can't factor
// these messages.

# ifndef _MSC_VER
#  warning please include <urbi/uobject.hh> instead of <uobject.h>.
#  include <urbi/uobject.hh>
# else
#  error include <urbi/uobject.hh> instead of <uobject.h>.
# endif

#endif // ! URBI_UOBJECT_HH

/// Local Variables:
/// mode: c++
/// End:
