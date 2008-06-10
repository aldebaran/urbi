#include <object/atom.hh>
#include <object/float-class.hh>
#include <object/semaphore-class.hh>
#include <runner/runner.hh>
#include <scheduler/fwd.hh>

namespace object {

  rObject semaphore_class;

  struct SemaphoreException : public scheduler::SchedulerException
  {
    COMPLETE_EXCEPTION(SemaphoreException);
  };

  static rObject
  semaphore_class_new(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(2);
    FETCH_ARG(1, Float);
    int count = ufloat_to_int(arg1->value_get(), "new");
    if (count < 0)
      throw PrimitiveError("new", "initial count must be non-negative");
    return Semaphore::fresh(make_pair(count, std::deque<scheduler::rJob>()));
  }

  static rObject
  semaphore_class_p(runner::Runner& r, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    FETCH_ARG(0, Semaphore);
    Semaphore::value_type& sem = arg0->value_get();
    if (--sem.first < 0)
    {
      sem.second.push_back(&r);
      try
      {
	r.yield_until_terminated(r);
      }
      catch (const SemaphoreException&)
      {
	// Regular wake up from a semaphore wait.
      }
    }
    return void_class;
  }

  static rObject
  semaphore_class_v(runner::Runner&, objects_type args)
  {
    CHECK_ARG_COUNT(1);
    FETCH_ARG(0, Semaphore);
    Semaphore::value_type& sem = arg0->value_get();
    if (++sem.first <= 0)
    {
      sem.second.front()->async_throw(SemaphoreException());
      sem.second.pop_front();
    }
    return void_class;
  }

  void
  semaphore_class_initialize()
  {
#define DECLARE(Name)				\
    DECLARE_PRIMITIVE(semaphore, Name)
    DECLARE(new);
    DECLARE(p);
    DECLARE(v);
#undef DECLARE
  }

} // namespace object
