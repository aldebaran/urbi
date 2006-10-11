#ifndef LOCKABLE_H
# define LOCKABLE_H


# ifdef WIN32
#  define _WIN32_WINNT 0x0400
#  include <windows.h>

typedef CRITICAL_SECTION Lock;
inline void initLock(Lock &l) {
  InitializeCriticalSection(&l);
}
inline void lockLock(Lock &l) {
  EnterCriticalSection(&l);
}

inline void lockUnlock(Lock &l) {
  LeaveCriticalSection(&l);
}

inline void deleteLock(Lock &l) {
  DeleteCriticalSection(&l);
}

inline bool lockTryLock(Lock &l) {
  return TryEnterCriticalSection(&l);
}

# elif defined OS && (OS == aibo)

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
#error "in aibo mode"
# else
# include <pthread.h>
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
# endif



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
