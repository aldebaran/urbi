/*
 * Copyright (C) 2009-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file urbi/uobject.hxx

#ifndef URBI_UOBJECT_HXX
# define URBI_UOBJECT_HXX

# include <libport/debug.hh>

# define URBI_BINDVARS(r, obj, v) UBindVar(obj, v);
# define URBI_BINDFUNCTIONS(r, obj, v) UBindFunction(obj, v);
# define URBI_BINDEVENTS(r, obj, v) UBindEvent(obj, v);

namespace urbi
{

  /*----------.
  | UObject.  |
  `----------*/

  inline
  int
  UObject::update()
  {
    return 0;
  }

  inline
  int
  UObject::voidfun()
  {
    return 0;
  }

  inline
  void
  UObject::clean()
  {
    aver(impl_);
    impl_->clean();
  }

  inline
  void
  UObject::USetUpdate(ufloat period)
  {
    aver(impl_);
    impl_->setUpdate(period);
  }

  inline
  void
  UObject::USync(UVar &v)
  {
    v.keepSynchronized();
  }

  inline
  bool
  UObject::removeTimer(TimerHandle h)
  {
    return impl_->removeTimer(h);
  }

  inline
  impl::UObjectImpl*
  UObject::impl_get()
  {
    return impl_;
  }

  inline
  libport::ThreadPool::rTaskLock
  UObject::getTaskLock(LockMode m, const std::string& what)
  {
    return getTaskLock(LockSpec(m), what);
  }

  inline
  libport::ThreadPool::rTaskLock
  UObject::getTaskLock(LockSpec m, const std::string& what)
  {
    GD_CATEGORY(Urbi.UObject);
    typedef libport::ThreadPool::rTaskLock rTaskLock;
    typedef libport::ThreadPool::TaskLock TaskLock;
    // Static in inlined functions are per-module.
    static rTaskLock module_lock(new TaskLock);
    switch(m.lockMode)
    {
    case LOCK_NONE:
      return 0;
      break;
    case LOCK_FUNCTION:
      {
        rTaskLock& res = taskLocks_[what];
        if (!res)
        {
          res = new TaskLock(m.maxQueueSize);
          GD_FINFO_TRACE("Creating taskLock for %s with %s: %s", what,
                         m.maxQueueSize, res.get());
        }
        return res;
      }
      break;
    case LOCK_INSTANCE:
      return taskLock_;
      break;
    case LOCK_CLASS:
      return getClassTaskLock();
      break;
    case LOCK_MODULE:
      return module_lock;
      break;
    }
    return 0;
  }


#ifndef NO_UOBJECT_CASTER
  inline UObject*
  uvalue_caster<UObject*>::operator()(UValue& v)
  {
    if (v.type != DATA_STRING && v.type != DATA_SLOTNAME)
      return 0;
    return getUObject(*v.stringValue);
  }
  inline
  UValue& operator,(UValue&a, const UObject* b)
  {
    if (!b)
      a = "nil";
    else
      a = b->__name;
    a.type = DATA_SLOTNAME;
    return a;
  }

#ifndef NO_ANY_POINTER_CASTER
  template<typename T> struct
  uvalue_caster<T*> {
    T* operator()(UValue& v)
    {
      UObject* res = uvalue_caster<UObject*>()(v);
      return dynamic_cast<T*>(res);
    }
  };
#endif
#endif
}

#endif // !URBI_UOBJECT_HXX
