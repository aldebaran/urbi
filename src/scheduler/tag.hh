#ifndef SCHEDULER_TAG_HH
# define SCHEDULER_TAG_HH

# include "scheduler/fwd.hh"

namespace scheduler
{

  class Tag
  {
  public:
    Tag ();
    Tag (rTag parent);
    virtual ~Tag ();

    bool frozen () const;
    bool blocked () const;

    // Actions must take a self parameter allowing the scheduler to retain
    // a counted reference on the tag if needed.
    void freeze (const Job&, rTag);
    void unfreeze (const Job&, rTag);
    void block (const Job&, rTag);
    void unblock (const Job&, rTag);
    void stop (const Job&, rTag);

    bool own_blocked () const;
    void set_blocked (bool);

  private:
    rTag parent_;
    bool blocked_;
    bool frozen_;
  };

} // namespace scheduler

#endif // SCHEDULER_TAG_HH
