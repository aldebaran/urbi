#include <libport/foreach.hh>

#include <scheduler/job.hh>
#include <scheduler/scheduler.hh>
#include <scheduler/tag.hh>

namespace scheduler
{
  Tag::Tag (rTag parent, libport::Symbol name)
    : parent_ (parent),
      blocked_ (false),
      frozen_ (false)
  {
    if (parent)
      name_ = libport::Symbol::Symbol
	(parent->name_get ().name_get () + "." + name.name_get ());
    else
      name_ = name;
  }

  void
  Tag::stop (Scheduler& sched)
  {
    sched.signal_stop (self ());
  }

} // namespace scheduler
