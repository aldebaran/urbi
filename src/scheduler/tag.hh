#ifndef SCHEDULER_TAG_HH
# define SCHEDULER_TAG_HH

# include <libport/symbol.hh>
# include <libport/weak-ptr.hh>

# include "scheduler/fwd.hh"

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

  class Tag
  {
  public:
    // Create a new tag, with or without a parent
    static rTag fresh (libport::Symbol name);
    static rTag fresh (rTag parent, libport::Symbol name);
    virtual ~Tag ();

    // Get a reference-counted pointer on the tag
    rTag self () const;

    // Is this tag directly or indirectly frozen or blocked?
    bool frozen () const;
    bool blocked () const;

    // Act on a tag and make the scheduler take it into account
    void freeze (Scheduler&);
    void unfreeze (Scheduler&);
    void block (Scheduler&);
    void unblock (Scheduler&);
    void stop (Scheduler&);

    // Return true if the tag has been directly blocked through a call
    // to "block". Return false if it is only indirectly blocked
    // through its parent.
    bool own_blocked () const;

    // Mark the tag as blocked. This is supposed to be used by the
    // scheduler to execute a "stop" command. It can also be used to
    // block a tag but without necessarily resuming its execution so that
    // it can rewind its call stack. For example, it can be used to stop
    // jobs if we know that the tag is permanently attached to the job:
    // if the job is currently sleeping, it may not be woken up just
    // so that it can die, it may die after it is done sleeping.
    void set_blocked (bool);

    const libport::Symbol& name_get () const;

  private:
    Tag (libport::Symbol name);
    Tag (rTag parent, libport::Symbol name);
    libport::weak_ptr<Tag> self_;
    rTag            parent_;
    bool            blocked_;
    bool            frozen_;
    libport::Symbol name_;
  };

} // namespace scheduler

# include "scheduler/tag.hxx"

#endif // SCHEDULER_TAG_HH
