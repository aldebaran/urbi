#ifndef LOCKABLE_H
#define LOCKABLE_H


#ifdef WIN32
#include <windows.h>
#error "write this code"
typedef CRITICAL_SECTION Lock;
inline void initLock(Lock &l) {
  InitializeCriticalSection(&l,0);
}
inline void lockLock(Lock &l) {
  pthread_mutex_lock(&l);
}

inline void lockUnlock(Lock &l) {
  pthread_mutex_unlock(&l);
}

inline void deleteLock(Lock &l) {
  pthread_mutex_destroy(&l);
}


#else
#if (OS == aibo)
typedef int Lock;
inline void initLock(Lock &l) {
  
}
inline void lockLock(Lock &l) {
  
}

inline void lockUnlock(Lock &l) {
  
}

inline void deleteLock(Lock &l) {
 
}

inline bool lockTryLock(Lock &l) {
  return true;
}

#else
#include <pthread.h>
typedef pthread_mutex_t Lock;
inline void initLock(Lock &l) {
  pthread_mutex_init(&l,0);
}
inline void lockLock(Lock &l) {
  pthread_mutex_lock(&l);
}

inline void lockUnlock(Lock &l) {
  pthread_mutex_unlock(&l);
}

inline void deleteLock(Lock &l) {
  pthread_mutex_destroy(&l);
}

inline bool lockTryLock(Lock &l) {
  return !pthread_mutex_trylock(&l);
}
#endif
#endif
class Lockable {
 public:
  Lockable() { initLock(_lock);}
  ~Lockable() { deleteLock(_lock);}
  void lock() {lockLock(_lock);}
  void unlock() {lockUnlock(_lock);}
  bool tryLock() {return lockTryLock(_lock);}
 private:
  Lock _lock;
};

class BlockLock {
  public:
    BlockLock(Lockable & l): l(l) {l.lock();}
    BlockLock(Lockable * l): l(*l) {l->lock();}
    ~BlockLock() {l.unlock();}

    private:
    Lockable &l;
};

#endif
