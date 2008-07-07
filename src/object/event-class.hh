/**
 ** \file object/event-class.hh
 ** \brief Definition of the URBI object event.
 */

#ifndef OBJECT_EVENT_CLASS_HH
# define OBJECT_EVENT_CLASS_HH

# include <libport/utime.hh>

# include <object/cxx-object.hh>
# include <object/fwd.hh>
# include <object/list-class.hh>

namespace object
{
  extern rObject event_class;

  class Event : public object::CxxObject
  {
  public:
    typedef rList value_type;

    Event();
    Event(const value_type& value);
    Event(rEvent model);
    const value_type& value_get() const;

    bool active_get() const;

    // Urbi primitives.
    rObject active();
    rList alive();
    void stop();
    rEvent trigger(runner::Runner&, objects_type);
    rList values();

    static void initialize(CxxObject::Binder<Event>& bind);
    static const std::string type_name;
    static bool event_added;
    virtual std::string type_name_get() const;

  private:
    value_type value_;
    bool       active_;
    rList      live_;
  };

} // namespace object

#endif // OBJECT_EVENT_CLASS_HH
