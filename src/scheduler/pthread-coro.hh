#ifndef SCHEDULER_PTHREAD_CORO_HH
# define SCHEDULER_PTHREAD_CORO_HH

# include <libport/config.h>

// Define LIBPORT_SCHEDULER_CORO_OSTHREAD to use the os-thread implementation
// of coros.
# ifdef LIBPORT_SCHEDULER_CORO_OSTHREAD
#  include <libport/semaphore.hh>

class Coro
{
public:
  Coro();
  libport::Semaphore sem_;
  bool started_;
  bool die_;
  pthread_t thread_;
};

#  include <scheduler/pthread-coro.hxx>

# endif // LIBPORT_SCHEDULER_CORO_OSTHREAD

#endif // !SCHEDULER_PTHREAD_CORO_HH
