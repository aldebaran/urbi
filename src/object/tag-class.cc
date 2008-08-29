/**
 ** \file object/tag-class.cc
 ** \brief Creation of the URBI object tag.
 */

#include <boost/any.hpp>

#include <object/tag-class.hh>

#include <object/global-class.hh>
#include <object/object.hh>
#include <object/string-class.hh>

#include <runner/call.hh>
#include <runner/runner.hh>

#include <scheduler/tag.hh>

namespace object
{

  rObject tag_class;

  Tag::Tag()
    : value_(0)
  {
    proto_add(tag_class);
  }

  Tag::Tag(const value_type& value)
    : value_(value)
  {
    proto_add(tag_class);
  }

  Tag::Tag(rTag model)
    : value_(model->value_)
  {
    proto_add(tag_class);
  }

  const Tag::value_type&
  Tag::value_get() const
  {
    return value_;
  }

  void
  Tag::block(runner::Runner& r, objects_type& args)
  {
    CHECK_ARG_COUNT_RANGE(0, 1, SYMBOL(block));
    rObject payload = boost::any_cast<rObject>(args.empty() ? void_class : args[0]);
    value_->block(r.scheduler_get(), payload);
  }

  void
  Tag::freeze(runner::Runner& r)
  {
    value_->freeze();
    if (r.frozen())
      r.yield();
  }

  rString
  Tag::name()
  {
    return new String(value_->name_get());
  }

  rTag
  Tag::_new(objects_type& args)
  {
    CHECK_ARG_COUNT_RANGE(1, 2, SYMBOL(new));
    libport::Symbol tag_short_name;

    if (args.size() > 1)
    {
      type_check<String>(args[1], SYMBOL(new));
      tag_short_name =
	libport::Symbol(args[1]->as<String>()->value_get());
    }
    else
      tag_short_name = libport::Symbol::fresh("tag");

    rTag res = new Tag(args[0] == tag_class ?
		       new scheduler::Tag(tag_short_name) :
		       extract_tag(args[0]));
    return res;
  }

  rTag
  Tag::new_flow_control(objects_type& args)
  {
    rTag res = _new(args);
    res->value_get()->flow_control_set();
    return res;
  }

  scheduler::prio_type
  Tag::prio()
  {
    return value_->prio_get();
  }

  scheduler::prio_type
  Tag::prio_set(runner::Runner& r, scheduler::prio_type prio)
  {
    return value_->prio_set(r.scheduler_get(), prio);
  }

  void
  Tag::stop(runner::Runner& r, objects_type& args)
  {
    CHECK_ARG_COUNT_RANGE(0, 1, SYMBOL(stop));
    rObject payload = boost::any_cast<rObject>(args.empty() ? void_class : args[0]);
    value_->stop(r.scheduler_get(), payload);
  }

  void
  Tag::unblock()
  {
    value_->unblock();
  }

  void
  Tag::unfreeze()
  {
    value_->unfreeze();
  }

  static inline rObject
  tag_event(Tag* owner, runner::Runner& r, const libport::Symbol& field)
  {
    if (!owner->slot_has(field))
    {
      rObject evt =
        urbi_call(r, global_class->slot_get(SYMBOL(Event)), SYMBOL(new));
      owner->slot_set(field, evt);
      return evt;
    }
    return owner->slot_get(field);
  }

  rObject
  Tag::enter(runner::Runner& r)
  {
    return tag_event(this, r, SYMBOL(enterEvent));
  }

  rObject
  Tag::leave(runner::Runner& r)
  {
    return tag_event(this, r, SYMBOL(leaveEvent));
  }

  void
  Tag::triggerEnter(runner::Runner& r)
  {
    if (slot_has(SYMBOL(enterEvent)))
      urbi_call(r, slot_get(SYMBOL(enterEvent)), SYMBOL(syncEmit));
  }

  void
  Tag::triggerLeave(runner::Runner& r)
  {
    if (slot_has(SYMBOL(leaveEvent)))
      urbi_call(r, slot_get(SYMBOL(leaveEvent)), SYMBOL(syncEmit));
  }

  void
  Tag::initialize(CxxObject::Binder<Tag>& bind)
  {
    bind(SYMBOL(block), &Tag::block);
    bind(SYMBOL(enter), &Tag::enter);
    bind(SYMBOL(freeze), &Tag::freeze);
    bind(SYMBOL(leave), &Tag::leave);
    bind(SYMBOL(name), &Tag::name);
    bind(SYMBOL(new), &Tag::_new);
    bind(SYMBOL(newFlowControl), &Tag::new_flow_control);
    bind(SYMBOL(prio), &Tag::prio);
    bind(SYMBOL(prio_set), &Tag::prio_set);
    bind(SYMBOL(stop), &Tag::stop);
    bind(SYMBOL(unblock), &Tag::unblock);
    bind(SYMBOL(unfreeze), &Tag::unfreeze);
  }

  bool Tag::tag_added = CxxObject::add<Tag>("Tag", tag_class);
  const std::string Tag::type_name = "Tag";
  std::string Tag::type_name_get() const
  {
    return type_name;
  }

  const scheduler::rTag&
  extract_tag(const rObject& o)
  {
    type_check<Tag>(o, SYMBOL(extract_tag));
    return o->as<Tag>()->value_get();
  }

}; // namespace object
