#ifndef SINGLETON_H
#define SINGLETON_H

/** Singleton smart pointer that creates the object on demmand
 * */
template<class T> class SingletonPtr {
  public:
    operator T* () {return check();}
    operator T& () {return *check();}

    T* operator ->() {return check();}
    T * check() {static T * ptr=0; if(ptr) return ptr; else return (ptr=new T());}
  private:
};

#define STATIC_INSTANCE(cl, name) \
  SingletonPtr<cl##name> name

#define EXTERN_STATIC_INSTANCE(cl, name)\
  class cl##name: public cl{}; \
extern SingletonPtr<cl##name> name


#endif
