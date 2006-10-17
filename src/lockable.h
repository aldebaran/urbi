#ifndef LOCKABLE_H
# define LOCKABLE_H

# if defined WIN32
#  define _WIN32_WINNT 0x0400
#  include <windows.h>

typedef CRITICAL_SECTION Lock;
inline void initLock(Lock &l)
{
  InitializeCriticalSection(&l);
}

inline void lockLock(Lock &l)
{
  EnterCriticalSection(&l);
}

inline void lockUnlock(Lock &l)
{
  LeaveCriticalSection(&l);
}

inline void deleteLock(Lock &l)
{
  DeleteCriticalSection(&l);
}

inline bool lockTryLock(Lock &l)
{
  return TryEnterCriticalSection(&l);
}


# elif defined OS && (OS == aibo)

typedef int Lock;
inline void initLock(Lock &l)
{
}

inline void lockLock(Lock &l)
{
}

inline void lockUnlock(Lock &l)
{
}

inline void deleteLock(Lock &l)
{
}

inline bool lockTryLock(Lock &l)
{
  return true;
}

#error "in aibo mode"

# else

# include <pthread.h>
typedef pthread_mutex_t Lock;
 inline void initLock(Lock &l)
  {
    pthread_mutexattr_t ma;
    pthread_mutexattr_init(&ma);
    /* See
     * http://www.nabble.com/Compiling-on-MacOS-10.4-Tiger-t284385.html
     * for more about this code.  Yes, the second #if is very
     * suspicious, I don't know why it's like this.  */
#if defined  __APPLE__ || defined __FreeBSD__
    pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
#elif defined PTHREAD_MUTEX_RECURSIVE_NP
    // cygwin
    pthread_mutexattr_setkind_np(&ma, PTHREAD_MUTEX_RECURSIVE);
#else
    // deprecated according to man page and fails to compile
    // pthread_mutexattr_setkind_np(&ma, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE_NP);
#endif
    pthread_mutex_init(&l,&ma);
  }
inline void lockLock(Lock &l)
{
  pthread_mutex_lock(&l);
}

inline void lockUnlock(Lock &l)
{
  pthread_mutex_unlock(&l);
}

inline void deleteLock(Lock &l)
{
  pthread_mutex_destroy(&l);
}

inline bool lockTryLock(Lock &l)
{
  return !pthread_mutex_trylock(&l);
}

# endif


class Lockable
{
 public:
  Lockable() { initLock(_lock);}
  ~Lockable() { deleteLock(_lock);}
  void lock() {lockLock(_lock);}
  void unlock() {lockUnlock(_lock);}
  bool tryLock() {return lockTryLock(_lock);}
 private:
  Lock _lock;
};

class BlockLock
{
 public:
 BlockLock(Lockable& l): l(l) {l.lock();}
 BlockLock(Lockable* l): l(*l)
    {
      //std::cerr <<"lock "<<l<<std::endl;
      l->lock();
    }
  ~BlockLock()
    {
      //std::cerr <<"unlock "<<&l<<std::endl;
      l.unlock();
    }

 private:
  Lockable &l;
};

#endif
