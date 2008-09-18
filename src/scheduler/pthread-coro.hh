#ifndef SCHEDULER_PTHREAD_CORO_HH
# define SCHEDULER_PTHREAD_CORO_HH

# include <kernel/config.h>

// Define SCHEDULER_CORO_OSTHREAD to use the os-thread implementation of coros.
# ifdef SCHEDULER_CORO_OSTHREAD
#  include <libport/semaphore.hh>

class Coro
{
public:
  Coro();
  libport::Semaphore sem_;
  bool die_;
  pthread_t thread_;
};

#  include <scheduler/pthread-coro.hxx>

# endif // SCHEDULER_CORO_OSTHREAD

#endif // !SCHEDULER_PTHREAD_CORO_HH
