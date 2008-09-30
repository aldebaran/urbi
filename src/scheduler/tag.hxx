#ifndef SCHEDULER_TAG_HXX
# define SCHEDULER_TAG_HXX

# include <boost/bind.hpp>

# include <scheduler/scheduler.hh>

namespace scheduler
{

  inline
  Tag::Tag(libport::Symbol name)
    : parent_(0)
    , blocked_(false)
    , frozen_(false)
    , name_(name)
    , prio_(UPRIO_DEFAULT)
    , flow_control_(false)
  {
  }

  inline
  Tag::~Tag()
  {
  }

  inline bool
  Tag::frozen() const
  {
    return frozen_ || (parent_ && parent_->frozen());
  }

  inline bool
  Tag::blocked() const
  {
    return blocked_ || (parent_ && parent_->blocked());
  }

  inline bool
  Tag::derives_from(const Tag& other) const
  {
    return this == &other || (parent_ && parent_->derives_from(other));
  }

  inline void
  Tag::freeze()
  {
    frozen_ = true;
  }

  inline void
  Tag::unfreeze()
  {
    frozen_ = false;
  }

  inline void
  Tag::block(Scheduler& sched, const boost::any& payload)
  {
    blocked_ = true;
    payload_ = payload;
    stop(sched, payload);
  }

  inline void
  Tag::unblock()
  {
    payload_ = 0;
    blocked_ = false;
  }

  inline const libport::Symbol&
  Tag::name_get() const
  {
    return name_;
  }

  inline void
  Tag::name_set(const libport::Symbol& name)
  {
    name_ = name;
  }

  inline prio_type
  Tag::prio_get() const
  {
    return prio_;
  }

  inline prio_type
  Tag::prio_set(Scheduler& sched, prio_type prio)
  {
    if (prio >= UPRIO_RT_MIN)
      sched.real_time_behaviour_set();
    prio_ = std::min(std::max(prio, prio_type(UPRIO_MIN)),
		     prio_type(UPRIO_MAX));
    return prio_;
  }

  inline void
  Tag::flow_control_set()
  {
    flow_control_ = true;
  }

  inline bool
  Tag::flow_control_get() const
  {
    return flow_control_;
  }

} // namespace scheduler

#endif // SCHEDULER_TAG_HXX
