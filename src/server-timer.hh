/**
 ** \file server/timer.hh
 */

#ifndef SERVER_TIMER_HH
# define SERVER_TIMER_HH

# if defined NDEBUG

# define TIMER_PUSH(Name)
# define TIMER_POP(Name)

# else

#  include <libport/timer.hh>

extern libport::timer server_timer;
#   define TIMER_PUSH(Name)			\
  server_timer.push(Name)
#   define TIMER_POP(Name)			\
  server_timer.pop(Name)

# endif // ! NDEBUG

#endif // !SERVER_TIMER_HH
