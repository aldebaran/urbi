#ifndef LOCKABLE_H
# define LOCKABLE_H

# ifdef WIN32
//#  undef _WIN32_WINNT
//#  define _WIN32_WINNT 0x0400

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
/*
inline bool lockTryLock(Lock &l) {
  return TryEnterCriticalSection(&l);
}*/
# else

#  include <pthread.h>
#  include <semaphore.h>
typedef pthread_mutex_t Lock;
inline void initLock(Lock &l) {
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
  pthread_mutexattr_setkind_np(&ma, PTHREAD_MUTEX_RECURSIVE_NP);
#endif
  pthread_mutex_init(&l,&ma);
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
# endif

class Lockable {
 public:
  Lockable() { initLock(_lock);}
  ~Lockable() { deleteLock(_lock);}
  void lock() {lockLock(_lock);}
  void unlock() {lockUnlock(_lock);}

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

# define LOCKED(lock, cmd)			\
 do {						\
   lock.lock();					\
   cmd;						\
   lock.unlock();				\
 } while (0)

# ifdef WIN32
typedef HANDLE sem_t;
inline void sem_init(HANDLE *sem, int useless, int cnt) {
  *sem = CreateSemaphore(NULL, cnt, 100000, NULL);
}
inline void sem_post(HANDLE * sem) {
  ReleaseSemaphore(*sem, 1, NULL);
}
inline void sem_wait(HANDLE *sem) {
  WaitForSingleObject(*sem, INFINITE);
}
inline void sem_destroy(HANDLE * sem) {
  DeleteObject(*sem);
}
inline void sem_getvalue(HANDLE *sem, int *v) {
  *v=1; //TODO: implement
}
# endif

class Semaphore {
  public:
    Semaphore(int cnt=0) {
      sem_init(&sem,0,cnt);
    }
    ~Semaphore() {
      sem_destroy(&sem);
    }
    void operator ++(int) {sem_post(&sem);}
    void operator --(int) {sem_wait(&sem);}
    void operator ++() {sem_post(&sem);}
    void operator --() {sem_wait(&sem);}
    operator int()  {int t;sem_getvalue(&sem,&t); return t;}

    private:
    sem_t sem;
};


template<class T> class StartInfo {
	public:
	T * inst;
	void (T::*func)(void);
};

# ifdef WIN32
typedef DWORD ThreadStartRet;
#  define THREADSTARTCALL WINAPI
# else
typedef void * ThreadStartRet;
#  define THREADSTARTCALL
# endif

template<class T> ThreadStartRet THREADSTARTCALL _startThread2(void * data) {
  StartInfo<T> * st = (StartInfo<T>*)data;
  ((*st->inst).*st->func)();
  delete st;
  return (ThreadStartRet) 0;
}

template<class T> ThreadStartRet THREADSTARTCALL _startThread(void * data) {
  T * t = (T*)data;
  (*t)();
}




template<class T> void * startThread(T * obj, void (T::*func)(void)) {
  StartInfo<T> * si = new StartInfo<T>();
  si->inst = obj;
  si->func = func;

# ifdef WIN32
  unsigned long id;
  void *r = CreateThread(NULL, 0,  &_startThread2<T> ,si, 0, &id);
# else
  pthread_t *pt = new pthread_t;
  pthread_create(pt,0, &_startThread2<T> ,si);
  void *r = pt;
# endif

  if (false) { //force instanciation
    _startThread2<T>(0);
  }

  return r;
}

template<class T> void *startThread(T * obj) {
# ifdef WIN32
  unsigned long id;
  void *r = CreateThread(NULL, 0,  &_startThread<T> ,obj, 0, &id);
# else
  pthread_t *pt = new pthread_t;
  pthread_create(pt,0, &_startThread<T> ,obj);
  void *r=pt;
# endif
  if (false) { //force instanciation
    _startThread<T>(0);
  }

  return r;
}


inline void joinThread(void *t) {
# ifdef WIN32
	WaitForSingleObject(t, INFINITE);
# else
	pthread_join(*(pthread_t*)t, 0);
# endif
}


#endif
