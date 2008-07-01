#include <boost/lambda/lambda.hpp>

#include <libport/containers.hh>

#include <object/barrier-class.hh>
#include <object/float-class.hh>

namespace object
{

  rObject barrier_class;

  Barrier::Barrier()
  {
    proto_add(barrier_class);
  }

  Barrier::Barrier(rBarrier model)
    : value_(model->value_)
  {
    proto_add(barrier_class);
  }

  Barrier::Barrier(const value_type& value)
    : value_(value)
  {
    proto_add(barrier_class);
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

  rFloat
  Barrier::signal(rObject payload)
  {
    if (value_.empty())
      return new Float(0);

    value_.front()->async_throw(BarrierException(payload));
    value_.pop_front();
    return new Float(1);
  }

  rFloat
  Barrier::signalAll(rObject payload)
  {
    rFloat res = new Float(value_.size());

    foreach (scheduler::rJob job, value_)
      job->async_throw(BarrierException(payload));
    value_.clear();

    return res;
  }

  const std::string Barrier::type_name = "Barrier";

  std::string
  Barrier::type_name_get() const
  {
    return type_name;
  }

  void Barrier::initialize(CxxObject::Binder<Barrier>& bind)
  {
    bind(SYMBOL(new),       &Barrier::_new);
    bind(SYMBOL(signal),    &Barrier::signal);
    bind(SYMBOL(signalAll), &Barrier::signalAll);
    bind(SYMBOL(wait),      &Barrier::wait);
  }

  bool Barrier::barrier_added =
    CxxObject::add<Barrier>("Barrier", barrier_class);

} // namespace object
