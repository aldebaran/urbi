#include <scheduler/scheduler.hh>
#include <scheduler/tag.hh>

namespace scheduler
{
  Tag::Tag(rTag parent, libport::Symbol name)
    : parent_(parent),
      blocked_(false),
      frozen_(false)
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

} // namespace scheduler
