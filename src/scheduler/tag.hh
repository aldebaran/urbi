#ifndef SCHEDULER_TAG_HH
# define SCHEDULER_TAG_HH

# include <libport/symbol.hh>

# include "scheduler/fwd.hh"

namespace scheduler
{

  class Tag
  {
  public:
    Tag (libport::Symbol name);
    Tag (rTag parent, libport::Symbol name);
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

    const libport::Symbol& name_get () const;

  private:
    rTag            parent_;
    bool            blocked_;
    bool            frozen_;
    libport::Symbol name_;
  };

} // namespace scheduler

#endif // SCHEDULER_TAG_HH
