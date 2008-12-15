#include <kernel/userver.hh>
#include <object/code.hh>
#include <object/float.hh>
#include <object/semaphore.hh>
#include <runner/runner.hh>
#include <sched/fwd.hh>

namespace object
{
  Semaphore::Semaphore()
  {
    proto_add(proto ? proto : object_class);
  }

  Semaphore::Semaphore(rSemaphore model)
    : value_(model->value_)
  {
    proto_add(proto);
  }

  Semaphore::Semaphore(const value_type& value)
    : value_(value)
  {
    proto_add(proto);
  }

  struct SemaphoreException : public sched::SchedulerException
  {
    COMPLETE_EXCEPTION(SemaphoreException);
  };

  rSemaphore
  Semaphore::_new(rObject, rFloat c)
  {
    int count = c->to_unsigned_int();
    return new Semaphore(make_pair(count, std::deque<sched::rJob>()));
  }

  void
  Semaphore::p()
  {
    runner::Runner& r = ::urbiserver->getCurrentRunner();

    if (--value_.first < 0)
    {
      value_.second.push_back(&r);
      try
      {
	r.yield_until_terminated(r);
      }
      catch (const SemaphoreException&)
      {
	// Regular wake up from a semaphore wait.
      }
    }
  }

  void
  Semaphore::v()
  {
    if (++value_.first <= 0)
    {
      value_.second.front()->async_throw(SemaphoreException());
      value_.second.pop_front();
    }
  }

  rObject
  Semaphore::criticalSection(rCode f)
  {
    objects_type args;
    args.push_back(this);

    p();
    libport::Finally finally(boost::bind(&Semaphore::v, this));
    return (*f)(args);
  }

  void Semaphore::initialize(CxxObject::Binder<Semaphore>& bind)
  {
    bind(SYMBOL(new),             &Semaphore::_new);
    bind(SYMBOL(criticalSection), &Semaphore::criticalSection);
    bind(SYMBOL(p),               &Semaphore::p);
    bind(SYMBOL(v),               &Semaphore::v);
  }

  rObject
  Semaphore::proto_make()
  {
    return new Semaphore();
  }

  URBI_CXX_OBJECT_REGISTER(Semaphore);
} // namespace object
