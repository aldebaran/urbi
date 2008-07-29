#ifndef SCHEDULER_PTHREAD_CORO_HXX
# define SCHEDULER_PTHREAD_CORO_HXX

// Implemented in the *.cc.
void
coroutine_start(Coro* self, Coro* other,
                void (*callback)(void*), void* context);

template <typename T>
inline
void
coroutine_start(Coro* self, Coro* other, void (*callback)(T*), T* context)
{
  coroutine_start(self, other,
                  reinterpret_cast<void(*)(void*)>(callback), context);
}


#endif // !SCHEDULER_PTHREAD_CORO_HH
