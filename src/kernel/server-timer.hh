/**
 ** \file server/timer.hh
 */

#ifndef KERNEL_SERVER_TIMER_HH
# define KERNEL_SERVER_TIMER_HH

# include <kernel/config.h>

# if defined COMPILATION_MODE_SPEED

#  define TIMER_INIT()
#  define TIMER_PUSH(Name)
#  define TIMER_POP(Name)

# else // ! COMPILATION_MODE_SPEED

#  include <libport/timer.hh>

namespace kernel
{
  extern libport::timer server_timer;

  /// Request that server_timer be GD_DUMPed on destruction.
  void timer_log_on_destruction();
}

#  define TIMER_INIT()                          \
  do {                                          \
    kernel::server_timer.start();               \
    kernel::timer_log_on_destruction();         \
  } while (false)

#  define TIMER_PUSH(Name)			\
  kernel::server_timer.push(Name)

#  define TIMER_POP(Name)			\
  kernel::server_timer.pop(Name)

# endif // ! COMPILATION_MODE_SPEED

#endif // !KERNEL_SERVER_TIMER_HH
