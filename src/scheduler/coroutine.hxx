#ifndef SCHEDULER_COROUTINE_HXX
#define SCHEDULER_COROUTINE_HXX

# include "kernel/kernconf.hh"

inline Coro*
coroutine_new(size_t stack_size)
{
  Coro* res = Coro_new();
  Coro_setStackSize_(res, stack_size ? stack_size : kernconf.default_stack_size);
  return res;
}

inline void
coroutine_free(Coro* coro)
{
  Coro_free(coro);
}

inline bool
coroutine_stack_space_almost_gone(Coro* coro)
{
  return Coro_bytesLeftOnStack(coro) < kernconf.minimum_stack_size;
}

inline void
coroutine_initialize_main(Coro* coro)
{
  Coro_initializeMainCoro(coro);
}

inline void
coroutine_start(Coro* self, Coro* other, void* context,
		CoroStartCallback* callback)
{
  Coro_startCoro_(self, other, context, callback);
}

inline void
coroutine_switch_to(Coro* self, Coro* next)
{
  Coro_switchTo_(self, next);
}

#endif // SCHEDULER_COROUTINE_HXX
