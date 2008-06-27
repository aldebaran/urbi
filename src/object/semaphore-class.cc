#include <object/atom.hh>
#include <object/float-class.hh>
#include <object/semaphore-class.hh>
#include <runner/runner.hh>
#include <scheduler/fwd.hh>

namespace object
{

  rObject semaphore_class;

  Semaphore::Semaphore()
  {
    proto_add(semaphore_class);
  }

  Semaphore::Semaphore(rSemaphore model)
    : value_(model->value_)
  {
    proto_add(semaphore_class);
  }

  Semaphore::Semaphore(const value_type& value)
    : value_(value)
  {
    proto_add(semaphore_class);
  }

  struct SemaphoreException : public scheduler::SchedulerException
  {
    COMPLETE_EXCEPTION(SemaphoreException);
  };

  rSemaphore
  Semaphore::_new(rObject, rFloat c)
  {
    int count = c->to_int("new");
    if (count < 0)
      throw PrimitiveError("new", "initial count must be non-negative");
    return new Semaphore(make_pair(count, std::deque<scheduler::rJob>()));
  }

  void
  Semaphore::p(runner::Runner& r)
  {
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

  const std::string Semaphore::type_name = "Semaphore";

  std::string
  Semaphore::type_name_get() const
  {
    return type_name;
  }

  void Semaphore::initialize(CxxObject::Binder<Semaphore>& bind)
  {
    bind(SYMBOL(new), &Semaphore::_new);
    bind(SYMBOL(p),   &Semaphore::p   );
    bind(SYMBOL(v),   &Semaphore::v   );
  }

  bool Semaphore::semaphore_added =
    CxxObject::add<Semaphore>("Semaphore", semaphore_class);

} // namespace object
