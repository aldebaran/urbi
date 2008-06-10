namespace scheduler
{

  inline
  Tag::Tag(libport::Symbol name)
    : parent_(0),
      blocked_(false),
      frozen_(false),
      name_(name)
  {
  }

  inline
  Tag::~Tag()
  {
  }

  inline rTag
  Tag::fresh(libport::Symbol name)
  {
    rTag res = new Tag(name);
    return res;
  }

  inline rTag
  Tag::fresh(rTag parent, libport::Symbol name)
  {
    rTag res = new Tag(parent, name);
    return res;
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
  Tag::freeze(Scheduler&)
  {
    frozen_ = true;
  }

  inline void
  Tag::unfreeze(Scheduler&)
  {
    frozen_ = false;
  }

  inline void
  Tag::block(Scheduler& sched, boost::any payload)
  {
    blocked_ = true;
    stop(sched, payload);
  }

  inline void
  Tag::unblock(Scheduler&)
  {
    blocked_ = false;
  }

  inline const libport::Symbol&
  Tag::name_get() const
  {
    return name_;
  }

} // namespace scheduler
