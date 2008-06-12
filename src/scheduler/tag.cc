#include <scheduler/scheduler.hh>
#include <scheduler/tag.hh>

namespace scheduler
{
  Tag::Tag(rTag parent, libport::Symbol name)
    : parent_(parent)
    , blocked_(false)
    , frozen_(false)
  {
    if (parent)
      name_ = libport::Symbol::Symbol
	(parent->name_get().name_get() + "." + name.name_get());
    else
      name_ = name;
  }

  void
  Tag::stop(Scheduler& sched, boost::any payload)
  {
    sched.signal_stop(this, payload);
  }

  const boost::any&
  Tag::payload_get() const
  {
    if (blocked_)
      return payload_;
    else if (parent_)
      return parent_->payload_get();
    else
      throw object::SchedulingError
	("attempt to retrieve payload of an unblocked tag");
  }

} // namespace scheduler
