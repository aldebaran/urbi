#include <scheduler/pthread-coro.hh>

#if defined SCHEDULER_CORO_OSTHREAD

#  include <cerrno>

#  include <libport/config.h>
#  include <libport/assert.hh>
#  include <libport/compiler.hh>
#  include <libport/semaphore.hh>
#  include <libport/thread.hh>

#  include <kernel/kernconf.hh>
#  include <scheduler/coroutine.hh>

/* Os-thread implementation of coroutines, using semaphores to ensure
   that only one coroutine is running at the same time.  */

Coro::Coro()
  : die_(false)
{}

Coro*
coroutine_new(size_t)
{
  return new Coro;
}

void
coroutine_free(Coro* c)
{
  void *status;
  c->die_ = true;
  c->sem_++;
  if (pthread_join(c->thread_, &status))
    errabort("pthread_join");
}

void
coroutine_start(Coro* self, Coro* other,
                void (*callback)(void*), void* context)
{
#if defined WIN32
  unsigned long id;
  void* r =
    CreateThread(NULL, 0,
                 reinterpret_cast<void* (*)(void*)>(callback), context,
                 0, &id);
#else
  // Adjust to the requested stack size.
  static pthread_attr_t attr;
  static bool first = true;
  if (first)
  {
    first = false;
    if (pthread_attr_init(&attr))
      errabort("pthread_attr_init");

    if (pthread_attr_setstacksize(&attr, kernconf.default_stack_size))
      errabort("pthread_attr_setstacksize");
  }

  if (pthread_create(&other->thread_, &attr,
		     reinterpret_cast<void* (*)(void*)>(callback), context))
    errabort("pthread_create");
#endif
  self->sem_--;
}

void
coroutine_switch_to(Coro* self, Coro* next)
{
  next->sem_++;
  self->sem_--;
  if (self->die_)
  {
    delete self;
    pthread_exit(0);
  }
}

bool
coroutine_stack_space_almost_gone(Coro*)
{
  return false;
}

void
coroutine_initialize_main(Coro*)
{
}

#endif //! SCHEDULER_CORO_OSTHREAD
