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
    static rTag _new(runner::Runner&, objects_type&);
    void freeze(runner::Runner&);
    libport::Symbol name();
    static rTag new_flow_control(runner::Runner& r, objects_type&);
    scheduler::prio_type prio();
    scheduler::prio_type prio_set(runner::Runner&, scheduler::prio_type);
    void stop(runner::Runner&, objects_type&);
    void unblock();
    void unfreeze();

    /// Return, potentially creating first, the enter event for \a this
    rObject enter(runner::Runner& r);
    /// Return, potentially creating first, the leave event for \a this
    rObject leave(runner::Runner& r);

    /// Trigger \a this' enter event
    void triggerEnter(runner::Runner& r);
    /// Trigger \a this' leave event
    void triggerLeave(runner::Runner& r);

    /*---------------.
    | Binding system |
    `---------------*/

    static void initialize(CxxObject::Binder<Tag>& bind);
    static const std::string type_name;
    static bool tag_added;
    virtual std::string type_name_get() const;
    static rObject proto;

  private:
    value_type value_;
  };

  const scheduler::rTag&
  extract_tag(const rObject& o, runner::Runner& r);

} // namespace object

# include <object/cxx-object.hxx>

#endif // !OBJECT_TAG_CLASS_HH
