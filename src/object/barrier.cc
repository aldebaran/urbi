#include <boost/lambda/lambda.hpp>

#include <libport/containers.hh>

#include <object/barrier.hh>
#include <object/float.hh>

namespace object
{

  Barrier::Barrier(rBarrier model)
    : value_(model->value_)
  {
    proto_add(model);
  }

  Barrier::Barrier(const value_type& value)
    : value_(value)
  {
    proto_add(proto ? proto : object_class);
  }

  struct BarrierException : public scheduler::SchedulerException
  {
    BarrierException(rObject payload) : payload_(payload) {};
    ADD_FIELD(rObject, payload)
    COMPLETE_EXCEPTION(BarrierException)
  };

  rBarrier
  Barrier::_new(rObject)
  {
    return new Barrier(std::deque<scheduler::rJob>());
  }

  rObject
  Barrier::wait(runner::Runner& r)
  {
    value_.push_back(&r);
    try
    {
      r.yield_until_terminated(r);
      // We cannot return from here.
      abort();
    }
    catch (const BarrierException& be)
    {
      // Regular wake up from a barrier wait.
      return be.payload_get();
    }
    catch (...)
    {
      // Signal that we should no longer stay queued.
      libport::erase_if(value_, boost::lambda::_1 == &r);
      throw;
    }
  }

  unsigned int
  Barrier::signal(rObject payload)
  {
    if (value_.empty())
      return 0;

    value_.front()->async_throw(BarrierException(payload));
    value_.pop_front();
    return 1;
  }

  unsigned int
  Barrier::signalAll(rObject payload)
  {
    const unsigned int res = value_.size();

    foreach (scheduler::rJob job, value_)
      job->async_throw(BarrierException(payload));
    value_.clear();

    return res;
  }

  void Barrier::initialize(CxxObject::Binder<Barrier>& bind)
  {
    bind(SYMBOL(new),       &Barrier::_new);
    bind(SYMBOL(signal),    &Barrier::signal);
    bind(SYMBOL(signalAll), &Barrier::signalAll);
    bind(SYMBOL(wait),      &Barrier::wait);
  }

  rObject
  Barrier::proto_make()
  {
    return new Barrier(Barrier::value_type());
  }

  URBI_CXX_OBJECT_REGISTER(Barrier);

} // namespace object
