/**
 ** \file object/tag-class.cc
 ** \brief Creation of the URBI object tag.
 */

#include <object/tag.hh>

#include <object/global.hh>
#include <object/object.hh>
#include <object/string.hh>

#include <runner/call.hh>
#include <runner/runner.hh>

#include <scheduler/tag.hh>

namespace object
{
  Tag::Tag()
    : value_(new scheduler::Tag(libport::Symbol::make_empty()))
  {
    proto_add(proto ? proto : object_class);
  }

  Tag::Tag(const value_type& value)
    : value_(value)
  {
    proto_add(proto);
  }

  Tag::Tag(rTag model)
    : value_(new scheduler::Tag(model->value_->name_get()))
  {
    proto_add(model);
    if (model.get() != proto.get())
      parent_ = model;
  }

  const Tag::value_type&
  Tag::value_get() const
  {
    return value_;
  }

  void
  Tag::block(runner::Runner& r, objects_type& args)
  {
    check_arg_count(args.size(), 0, 1);
    rObject payload = args.empty() ? void_class : args[0];
    value_->block(r.scheduler_get(), payload);
  }

  void
  Tag::freeze(runner::Runner& r)
  {
    value_->freeze();
    if (r.frozen())
      r.yield();
  }

  libport::Symbol
  Tag::name()
  {
    return value_->name_get();
  }

  void
  Tag::init(objects_type& args)
  {
    check_arg_count(args.size(), 0, 1);
    libport::Symbol tag_short_name;

    if (args.size() > 0)
    {
      type_check<String>(args[0]);
      tag_short_name =
	libport::Symbol(args[0]->as<String>()->value_get());
    }
    else
      tag_short_name = libport::Symbol::fresh("tag");

    value_->name_set(tag_short_name);
  }

  rTag
  Tag::new_flow_control(runner::Runner& r, objects_type& args)
  {
    args.pop_front();
    rTag res = urbi_call(r, proto, SYMBOL(new), args)->as<Tag>();
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
    check_arg_count(args.size(), 0, 1);
    rObject payload = args.empty() ? void_class : args[0];
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
      CAPTURE_GLOBAL(Event);
      rObject evt = urbi_call(r, Event, SYMBOL(new));
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

  rTag
  Tag::parent_get()
  {
    return parent_;
  }

  void
  Tag::initialize(CxxObject::Binder<Tag>& bind)
  {
    bind(SYMBOL(block), &Tag::block);
    bind(SYMBOL(enter), &Tag::enter);
    bind(SYMBOL(freeze), &Tag::freeze);
    bind(SYMBOL(getParent), &Tag::parent_get);
    bind(SYMBOL(leave), &Tag::leave);
    bind(SYMBOL(name), &Tag::name);
    bind(SYMBOL(init), &Tag::init);
    bind(SYMBOL(newFlowControl), &Tag::new_flow_control);
    bind(SYMBOL(prio), &Tag::prio);
    bind(SYMBOL(prio_set), &Tag::prio_set);
    bind(SYMBOL(stop), &Tag::stop);
    bind(SYMBOL(unblock), &Tag::unblock);
    bind(SYMBOL(unfreeze), &Tag::unfreeze);
  }

  URBI_CXX_OBJECT_REGISTER(Tag);

  rObject
  Tag::proto_make()
  {
    return new Tag();
  }

}; // namespace object
