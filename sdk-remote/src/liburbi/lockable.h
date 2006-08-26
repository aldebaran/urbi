#ifndef LOCKABLE_H
#define LOCKABLE_H
#include <pthread.h>
#include <semaphore.h>
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

#define LOCKED(lock, cmd) lock.lock();cmd; lock.unlock()

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
	T * inst;
	void (T::*func)(void);
};



template<class T> void * _startThread2(void * data) {
  StartInfo<T> * st = (StartInfo<T>*)data;
  ((*st->inst).*st->func)();
  delete st;
}

template<class T> void * _startThread(void * data) {
  T * t = (T*)data;
  (*t)();
}


template<class T> void startThread(T * obj, void (T::*func)(void)) {
  pthread_t *pt = new pthread_t;
  StartInfo<T> * si = new StartInfo<T>();
  si->obj = obj;
  si->func = func;
  pthread_create(pt,0, &_startThread2<T> ,si);

  if (false) { //force instanciation
    _startThread2<T>(0);
  }

}

template<class T> void startThread(T * obj) {
  pthread_t *pt = new pthread_t;
  pthread_create(pt,0, &_startThread<T> ,obj);

  if (false) { //force instanciation
    _startThread<T>(0);
  }

}

#endif
