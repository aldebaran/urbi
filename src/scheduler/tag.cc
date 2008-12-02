#include <cstdlib>

#include <scheduler/scheduler.hh>
#include <scheduler/tag.hh>

namespace scheduler
{
  void
  Tag::stop(Scheduler& sched, const boost::any& payload) const
  {
    sched.signal_stop(*this, payload);
  }

  const boost::any&
  Tag::payload_get() const
  {
    if (blocked_)
      return payload_;
    // This is an internal error and can never happen.
    pabort("!blocked");
  }

} // namespace scheduler
