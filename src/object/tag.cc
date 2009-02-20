/**
 ** \file object/tag-class.cc
 ** \brief Creation of the URBI object tag.
 */

#include <kernel/userver.hh>

#include <object/tag.hh>
#include <object/global.hh>
#include <object/object.hh>
#include <object/string.hh>
#include <object/symbols.hh>

#include <runner/runner.hh>

#include <sched/tag.hh>

namespace object
{
  Tag::Tag()
    : value_(new sched::Tag(libport::Symbol::make_empty()))
  {
    proto_add(proto ? proto : object_class);
  }

  Tag::Tag(const value_type& value)
    : value_(value)
  {
    proto_add(proto);
  }

  Tag::Tag(rTag model)
    : value_(new sched::Tag(model->value_->name_get()))
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
  Tag::block(const objects_type& args)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    check_arg_count(args.size(), 0, 1);
    const rObject& payload = args.empty() ? void_class : args.front();
    value_->block(r.scheduler_get(), payload);
  }

  void
  Tag::freeze()
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

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
  Tag::init(const objects_type& args)
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
  Tag::new_flow_control(const objects_type& args)
  {
    // FIXME: new is now called on self instead of on proto.
    rTag res = args[0]->call(SYMBOL(new), args)->as<Tag>();
    res->value_get()->flow_control_set();
    return res;
  }

  sched::prio_type
  Tag::prio()
  {
    return value_->prio_get();
  }

  sched::prio_type
  Tag::prio_set(sched::prio_type prio)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    return value_->prio_set(r.scheduler_get(), prio);
  }

  void
  Tag::stop(const objects_type& args)
  {
    runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
    check_arg_count(args.size(), 0, 1);
    const rObject& payload = args.empty() ? void_class : args.front();
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
  tag_event(Tag* owner, const libport::Symbol& field)
  {
    if (!owner->slot_has(field))
    {
      CAPTURE_GLOBAL(Event);
      rObject evt = Event->call(SYMBOL(new));
      owner->slot_set(field, evt);
      return evt;
    }
    return owner->slot_get(field);
  }

  rObject
  Tag::enter()
  {
    return tag_event(this, SYMBOL(enterEvent));
  }

  rObject
  Tag::leave()
  {
    return tag_event(this, SYMBOL(leaveEvent));
  }

  void
  Tag::triggerEnter()
  {
    if (slot_has(SYMBOL(enterEvent)))
      slot_get(SYMBOL(enterEvent))->call(SYMBOL(syncEmit));
  }

  void
  Tag::triggerLeave()
  {
    if (slot_has(SYMBOL(leaveEvent)))
      slot_get(SYMBOL(leaveEvent))->call(SYMBOL(syncEmit));
  }

  rTag
  Tag::parent_get()
  {
    return parent_;
  }

  bool
  Tag::frozen()
  {
    return value_->frozen();
  }

  bool
  Tag::blocked()
  {
    return value_->blocked();
  }

  void
  Tag::initialize(CxxObject::Binder<Tag>& bind)
  {
    bind(SYMBOL(block), &Tag::block);
    bind(SYMBOL(blocked), &Tag::blocked);
    bind(SYMBOL(enter), &Tag::enter);
    bind(SYMBOL(freeze), &Tag::freeze);
    bind(SYMBOL(frozen), &Tag::frozen);
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
