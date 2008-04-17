#ifndef SCHEDULER_TAG_HH
# define SCHEDULER_TAG_HH

# include <libport/symbol.hh>
# include <libport/weak-ptr.hh>

# include "scheduler/fwd.hh"

namespace scheduler
{

  class Tag
  {
  public:
    static rTag fresh (libport::Symbol name);
    static rTag fresh (rTag parent, libport::Symbol name);
    virtual ~Tag ();

    rTag self () const;

    bool frozen () const;
    bool blocked () const;

    void freeze (const Job&);
    void unfreeze (const Job&);
    void block (const Job&);
    void unblock (const Job&);
    void stop (const Job&);

    bool own_blocked () const;
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
