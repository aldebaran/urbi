#include <libport/config.h>
#include <libport/compiler.hh>
#include <libport/thread.hh>
#include <libport/semaphore.hh>

#include <scheduler/coroutine.hh>
/* Os-thread implementation of coroutines, using semaphores to ensure that only
 one coroutine is running at the same time.
*/
inline
Coro* coroutine_new(size_t)
{
  return new Coro;
}

inline
void coroutine_free(Coro* c)
{
  c->die_ = true;
  c->sem++;
}

template<typename T>
inline void
coroutine_start(Coro* self, Coro*, void (*callback)(T*), T* context)
{
#if defined WIN32
  unsigned long id;
  void* r = CreateThread(NULL, 0, void*(*)(void*))callback, (void*)context, 0, &id);
#else
  pthread_t*pt = new pthread_t;
  pthread_create(pt, 0, (void*(*)(void*))callback, (void*)context);
#endif
  self->sem--;
}

inline
void coroutine_switch_to(Coro* self, Coro* next)
{
  next->sem++;
  self->sem--;
  if (self->die_)
  {
    delete self;
    pthread_exit(0);
  }
}

inline
bool coroutine_stack_space_almost_gone(Coro*)
{
  return false;
}

inline
void coroutine_initialize_main(Coro*)
{
}
