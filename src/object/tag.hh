/**
 ** \file object/tag-class.hh
 ** \brief Definition of the URBI object tag.
 */

#ifndef OBJECT_TAG_CLASS_HH
# define OBJECT_TAG_CLASS_HH

# include <object/cxx-object.hh>
# include <object/fwd.hh>
# include <scheduler/tag.hh>

namespace object
{
  class Tag : public object::CxxObject
  {
  public:
    typedef scheduler::rTag value_type;

    Tag();
    Tag(const value_type& value);
    Tag(rTag model);
    const value_type& value_get() const;

    void block(runner::Runner&, objects_type&);
    void init(objects_type& args);
    void freeze(runner::Runner&);
    libport::Symbol name();
    static rTag new_flow_control(runner::Runner& r, objects_type&);
    scheduler::prio_type prio();
    scheduler::prio_type prio_set(runner::Runner&, scheduler::prio_type);
    void stop(runner::Runner&, objects_type&);
    void unblock();
    void unfreeze();
    bool frozen();
    bool blocked();

    /// Return, potentially creating first, the enter event for \a this
    rObject enter(runner::Runner& r);
    /// Return, potentially creating first, the leave event for \a this
    rObject leave(runner::Runner& r);

    /// Trigger \a this' enter event
    void triggerEnter(runner::Runner& r);
    /// Trigger \a this' leave event
    void triggerLeave(runner::Runner& r);

    /// Manipulate parent tag.
    rTag parent_get();

  private:
    value_type value_;
    rTag parent_;

  URBI_CXX_OBJECT(Tag);
  };

} // namespace object

# include <object/cxx-object.hxx>

#endif // !OBJECT_TAG_CLASS_HH
