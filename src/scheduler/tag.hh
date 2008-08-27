#ifndef SCHEDULER_TAG_HH
# define SCHEDULER_TAG_HH

# include <vector>

# include <boost/any.hpp>

# include <libport/finally.hh>
# include <libport/symbol.hh>

# include <scheduler/fwd.hh>

namespace scheduler
{
  /// Tag priorities
  typedef unsigned int prio_type;

  // The following names stay away from PRIO_* which is used in
  // sys/resource.h under OSX.

  enum
  {
    /// Unused for priority computations.
    UPRIO_NONE = 0,
    /// Minimum non-real-time priority.
    UPRIO_MIN = UPRIO_NONE,
    /// Default job priority when none is given.
    UPRIO_DEFAULT = UPRIO_MIN + 1,
    /// Lowest real-time priority.
    UPRIO_RT_MIN = UPRIO_DEFAULT + 1,
    /// Highest real-time priority.
    UPRIO_RT_MAX = 16,
    /// Highest priority.
    UPRIO_MAX = UPRIO_RT_MAX
  };

  typedef std::vector<rTag> tags_type;

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
    explicit Tag(libport::Symbol name);
    Tag(const rTag& parent, libport::Symbol name);
    virtual ~Tag();

    // Is this tag directly or indirectly frozen or blocked?
    bool frozen() const;
    bool blocked() const;

    // If the tag is blocked, what is its payload? It is a fatal error
    // to call this method if the tag is not blocked.
    const boost::any& payload_get() const;

    // Is this tag identical to \a other, or does it derive from it?
    bool derives_from(const Tag& other) const;

    // Set and get the priority. When setting the priority, cap it with
    // the minimum and maximum values and return the chosen one.
    prio_type prio_set(Scheduler&, prio_type);
    prio_type prio_get() const;

    // Apply a tag by changing the \a tags list and register the action
    // to be taken when the tag is removed into the \a finally object if
    // given.
    virtual void apply_tag(tags_type& tags, libport::Finally* finally);

    // Set and get the flow_control property.
    void flow_control_set();
    bool flow_control_get() const;

    // Act on a tag and make the scheduler take it into account
    void freeze();
    void unfreeze();
    void block(Scheduler&, const boost::any&);
    void unblock();
    void stop(Scheduler&, const boost::any&) const;

    const libport::Symbol& name_get() const;

  private:
    explicit Tag(const Tag&);

    rTag parent_;
    bool blocked_;
    bool frozen_;
    libport::Symbol name_;
    boost::any payload_;
    prio_type prio_;
    bool flow_control_;
  };

} // namespace scheduler

# include <scheduler/tag.hxx>

#endif // SCHEDULER_TAG_HH
