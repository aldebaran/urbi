#ifndef SCHEDULER_TAG_HH
# define SCHEDULER_TAG_HH

# include <vector>

# include <libport/symbol.hh>

# include <scheduler/fwd.hh>

namespace scheduler
{

  // A Tag is an entity attached to zero or more scheduler jobs. Each job
  // can have zero or more tags. When a new job is created, it usually
  // inherits the tags from its creator.
  //
  // The Tag state is represented by two parameters:
  //
  //   - frozen: jobs with this tag are not supposed to do any useful work;
  //
  //   - blocked: jobs with this tag are supposed to rewind their call
  //     stack until they die or until they are no longer affected by
  //     the tag.
  //
  // A tag is frozen when it has been explicitly frozen or when its parent
  // is frozen. Ditto for blocked.
  //
  // Stopping a tag is an immediate operation: all the jobs holding this
  // tag must act as if they were blocked, but only once. For example, they
  // must rewind their call stack in order not to be blocked anymore, but
  // when they resume execution, if they get the same tag again, they will
  // not act as if they were blocked again.

  class Tag: public libport::RefCounted
  {
  public:
    // Create a new tag, with or without a parent
    static rTag fresh(libport::Symbol name);
    static rTag fresh(rTag parent, libport::Symbol name);
    virtual ~Tag();

    // Is this tag directly or indirectly frozen or blocked?
    bool frozen() const;
    bool blocked() const;

    // If the tag is blocked, what is its payload? It is a fatal error
    // to call this method if the tag is not blocked.
    const boost::any& payload_get() const;

    // Is this tag identical to \a other, or does it derive from it?
    bool derives_from(const Tag& other) const;

    // Act on a tag and make the scheduler take it into account
    void freeze(Scheduler&);
    void unfreeze(Scheduler&);
    void block(Scheduler&, boost::any);
    void unblock(Scheduler&);
    void stop(Scheduler&, boost::any);

    const libport::Symbol& name_get() const;

  private:
    explicit Tag(const Tag&);
    Tag(libport::Symbol name);
    Tag(rTag parent, libport::Symbol name);
    rTag parent_;
    bool blocked_;
    bool frozen_;
    libport::Symbol name_;
    boost::any payload_;
  };

  typedef std::vector<rTag> tags_type;

} // namespace scheduler

# include <scheduler/tag.hxx>

#endif // SCHEDULER_TAG_HH
