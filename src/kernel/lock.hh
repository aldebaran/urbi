#ifndef KERNEL_LOCK_HH
# define KERNEL_LOCK_HH

# include <kernel/fwd.hh>

namespace kernel
{
  /// Die if this server is not allowed to run.
  void lock_check(const UServer& s);
}

#endif // !KERNEL_LOCK_HH
