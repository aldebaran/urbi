#include <libport/foreach.hh>

#include "scheduler/job.hh"
#include "scheduler/scheduler.hh"
#include "scheduler/tag.hh"

namespace scheduler
{
  Tag::Tag ()
    : parent_ (0),
      blocked_ (false),
      frozen_ (false)
  {
  }

  Tag::Tag (rTag parent)
    : parent_ (parent),
      blocked_ (false),
      frozen_ (false)
  {
  }

  Tag::~Tag ()
  {
  }

  bool
  Tag::frozen () const
  {
    return frozen_ || (parent_ && parent_->frozen ());
  }

  bool
  Tag::blocked () const
  {
    return blocked_ || (parent_ && parent_->blocked ());
  }

  void
  Tag::freeze (Job&)
  {
    frozen_ = true;
  }

  void
  Tag::unfreeze (Job&)
  {
    frozen_ = false;
  }

  void
  Tag::block (Job& job)
  {
    blocked_ = true;
    stop (job);
  }

  void
  Tag::unblock (Job&)
  {
    blocked_ = false;
  }

  void
  Tag::stop (Job& job)
  {
    job.scheduler_get ().signal_stop (this);
  }

  bool
  Tag::own_blocked () const
  {
    return blocked_;
  }

  void
  Tag::set_blocked (bool b)
  {
    blocked_ = b;
  }

} // namespace scheduler
