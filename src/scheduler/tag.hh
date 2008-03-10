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

    void freeze (Job&);
    void unfreeze (Job&);
    void block (Job&);
    void unblock (Job&);
    void stop (Job&);

    bool own_blocked () const;
    void set_blocked (bool);

  private:
    rTag parent_;
    bool blocked_;
    bool frozen_;
  };

} // namespace scheduler

#endif // SCHEDULER_TAG_HH
