#include <libport/foreach.hh>

#include "scheduler/job.hh"
#include "scheduler/scheduler.hh"
#include "scheduler/tag.hh"

namespace scheduler
{
  Tag::Tag (libport::Symbol name)
    : parent_ (0),
      blocked_ (false),
      frozen_ (false),
      name_ (name)
  {
  }

  Tag::Tag (rTag parent, libport::Symbol name)
    : parent_ (parent),
      blocked_ (false),
      frozen_ (false)
  {
    if (parent)
      name_ = libport::Symbol::Symbol (parent->name_get ().name_get () + "." + name.name_get ());
    else
      name_ = name;
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
  Tag::freeze (const Job&, rTag)
  {
    frozen_ = true;
  }

  void
  Tag::unfreeze (const Job&, rTag)
  {
    frozen_ = false;
  }

  void
  Tag::block (const Job& job, rTag self)
  {
    blocked_ = true;
    stop (job, self);
  }

  void
  Tag::unblock (const Job&, rTag)
  {
    blocked_ = false;
  }

  void
  Tag::stop (const Job& job, rTag self)
  {
    job.scheduler_get ().signal_stop (self);
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

  const libport::Symbol&
  Tag::name_get () const
  {
    return name_;
  }

} // namespace scheduler
