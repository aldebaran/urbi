/**
 ** \file object/tag-class.hh
 ** \brief Definition of the URBI object tag.
 */

#ifndef OBJECT_TAG_CLASS_HH
# define OBJECT_TAG_CLASS_HH

# include <object/cxx-object.hh>
# include <object/fwd.hh>
# include <sched/tag.hh>

namespace object
{
  class Tag : public object::CxxObject
  {
  public:
    typedef sched::rTag value_type;

    Tag();
    Tag(const value_type& value);
    Tag(rTag model);
    const value_type& value_get() const;

    void block(const objects_type&);
    void init(const objects_type& args);
    void freeze();
    libport::Symbol name() const;
    static rTag new_flow_control(const objects_type&);
    sched::prio_type prio() const;
    sched::prio_type prio_set(sched::prio_type);
    void stop(const objects_type&);
    void unblock();
    void unfreeze();
    bool frozen() const;
    bool blocked() const;

    /// Return, potentially creating first, the enter event for \a this
    rObject enter();
    /// Return, potentially creating first, the leave event for \a this
    rObject leave();

    /// Trigger \a this' enter event
    void triggerEnter();
    /// Trigger \a this' leave event
    void triggerLeave();

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
