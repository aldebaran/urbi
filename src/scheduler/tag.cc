#include <scheduler/scheduler.hh>
#include <scheduler/tag.hh>

namespace scheduler
{
  Tag::Tag(const rTag& parent, libport::Symbol name)
    : parent_(parent)
    , blocked_(false)
    , frozen_(false)
    , prio_(parent->prio_)
  {
    if (parent)
      name_ = libport::Symbol::Symbol
	(parent->name_get().name_get() + "." + name.name_get());
    else
      name_ = name;
  }

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
    else if (parent_)
      return parent_->payload_get();
    else
      throw object::SchedulingError
	("attempt to retrieve payload of an unblocked tag");
  }

  void
  Tag::apply_tag(tags_type& tags, libport::Finally* finally)
  {
    tags.push_back(this);
    if (finally)
      *finally << boost::bind(&tags_type::pop_back, boost::ref(tags));
  }

} // namespace scheduler
