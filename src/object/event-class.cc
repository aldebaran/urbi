/**
 ** \file object/event-class.cc
 ** \brief Creation of the URBI object event.
 */

#include <boost/assign.hpp>

#include <object/code-class.hh>
#include <object/event-class.hh>
#include <object/float-class.hh>
#include <object/list-class.hh>
#include <object/object.hh>
#include <runner/runner.hh>

namespace object
{
  using namespace boost::assign;

  rObject event_class;

  Event::Event()
    : value_(new List)
    , active_(false)
    , live_(new List)
  {
    proto_add(event_class);
  }

  Event::Event(const value_type& value)
    : value_(value)
    , active_(false)
    , live_(new List)
  {
    proto_add(event_class);
  }

  Event::Event(rEvent model)
    : value_(model->value_)
    , active_(false)
    , live_(model->live_)
  {
    proto_add(event_class);
  }

  rObject
  Event::active()
  {
    return active_get() ? true_class : false_class;
  }

  bool
  Event::active_get() const
  {
    return active_;
  }

  rList
  Event::alive()
  {
    return live_;
  }

  void
  Event::stop()
  {
    active_ = false;
    live_->remove_by_id(this);
  }

  rEvent
  Event::trigger(runner::Runner& r, objects_type payload)
  {
    rEvent instance = new Event(this);
    instance->value_ = new List(payload);
    instance->active_ = true;
    live_->push_back(instance);
    urbi_call(r, this, SYMBOL(emit), payload);
    return instance;
  }

  rList
  Event::values()
  {
    return value_;
  }

  const Event::value_type&
  Event::value_get() const
  {
    return value_;
  }

  void
  Event::initialize(CxxObject::Binder<Event>& bind)
  {
    bind(SYMBOL(active), &Event::active);
    bind(SYMBOL(alive), &Event::alive);
    bind(SYMBOL(stop), &Event::stop);
    bind(SYMBOL(trigger), &Event::trigger);
    bind(SYMBOL(values), &Event::values);
  }

  bool Event::event_added = CxxObject::add<Event>("Event", event_class);
  const std::string Event::type_name = "Event";
  std::string Event::type_name_get() const
  {
    return type_name;
  }

} // namespace object
