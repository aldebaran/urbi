/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/tag.cc
 ** \brief Creation of the Urbi object tag.
 */

#include <kernel/userver.hh>

#include <urbi/object/tag.hh>
#include <urbi/object/global.hh>
#include <urbi/object/object.hh>
#include <urbi/object/string.hh>
#include <object/symbols.hh>

#include <runner/interpreter.hh>
#include <runner/runner.hh>

#include <sched/tag.hh>

namespace urbi
{
  namespace object
  {
    Tag::Tag()
      : value_(new sched::Tag(libport::Symbol::make_empty()))
    {
      proto_add(proto ? rObject(proto) : Object::proto);
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

    Tag::value_type&
    Tag::value_get()
    {
      return value_;
    }

    const Tag::value_type&
    Tag::value_get() const
    {
      return value_;
    }

    void
    Tag::block(const objects_type& args)
    {
      check_arg_count(args.size(), 0, 1);
      const rObject& payload = args.empty() ? void_class : args.front();
      value_->block(::kernel::scheduler(), payload);
    }

    void
    Tag::freeze()
    {
      runner::Runner& r = ::kernel::runner();

      value_->freeze();
      if (r.frozen())
        r.yield();
    }

    void
    Tag::init(const objects_type& args)
    {
      check_arg_count(args.size(), 0, 1);
      if (args.empty())
        name_set(libport::Symbol::fresh("tag"));
      else
      {
        type_check<String>(args[0]);
        name_set(libport::Symbol(args[0]->as<String>()->value_get()));
      }
    }

    rTag
    Tag::new_flow_control(const objects_type& args)
    {
      // FIXME: new is now called on self instead of on proto.
      rTag res = args[0]->call_with_this(SYMBOL(new), args)->as<Tag>();
      res->value_get()->flow_control_set();
      return res;
    }

    Tag::priority_type
    Tag::priority_set(priority_type prio)
    {
      return value_->prio_set(::kernel::scheduler(), prio);
    }

    void
    Tag::stop(const rObject& payload)
    {
      value_->stop(::kernel::scheduler(), payload);
    }

    void
    Tag::stop(const objects_type& args)
    {
      check_arg_count(args.size(), 0, 1);
      stop(args.empty() ? void_class : args.front());
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
      if (owner->local_slot_get(field))
        return owner->slot_get(field);

      CAPTURE_GLOBAL(Event);
      rObject evt = Event->call(SYMBOL(new));
      owner->slot_set(field, evt);
      return evt;
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
      if (local_slot_get(SYMBOL(enterEvent)))
        slot_get(SYMBOL(enterEvent))->call(SYMBOL(syncEmit));
    }

    void
    Tag::triggerLeave()
    {
      if (local_slot_get(SYMBOL(leaveEvent)))
        slot_get(SYMBOL(leaveEvent))->call(SYMBOL(syncEmit));
    }

    rTag
    Tag::parent_get()
    {
      return parent_;
    }

    static inline
    runner::Runner&
    runner()
    {
      return ::kernel::runner();
    }

    static inline
    runner::Interpreter&
    interpreter()
    {
      return dynamic_cast<runner::Interpreter&>(runner());
    }

    rTag
    Tag::scope()
    {
      return new Tag(interpreter().scope_tag());
    }

    bool
    Tag::frozen() const
    {
      return value_->frozen();
    }

    bool
    Tag::blocked() const
    {
      return value_->blocked();
    }

    void
    Tag::initialize(CxxObject::Binder<Tag>& bind)
    {
      typedef void (Tag::*stop_type)(const objects_type& args);
      bind(SYMBOL(stop), static_cast<stop_type> (&Tag::stop));
#define DECLARE(Name, Function)                 \
      bind(SYMBOL(Name), &Tag::Function)

      DECLARE(block, block);
      DECLARE(blocked, blocked);
      DECLARE(enter, enter);
      DECLARE(freeze, freeze);
      DECLARE(frozen, frozen);
      DECLARE(getParent, parent_get);
      DECLARE(init, init);
      DECLARE(leave, leave);
      DECLARE(name, name);
      DECLARE(newFlowControl, new_flow_control);
      DECLARE(priority, priority);
      DECLARE(scope, scope);
      DECLARE(setPriority, priority_set);
      DECLARE(unblock, unblock);
      DECLARE(unfreeze, unfreeze);
#undef DECLARE
    }

    URBI_CXX_OBJECT_REGISTER(Tag)
      : value_(new sched::Tag(libport::Symbol::make_empty()))
    {}

  } // namespace object
}
