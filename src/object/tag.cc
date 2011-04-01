/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
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

#include <runner/job.hh>
#include <eval/call.hh>

#include <sched/tag.hh>

namespace urbi
{
  namespace object
  {
    Tag::Tag()
      : value_(new sched::Tag(""))
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

    URBI_CXX_OBJECT_INIT(Tag)
      : value_(new sched::Tag(""))
    {
#define DECLARE(Name, Cast)                                             \
      bind(SYMBOL_(Name), static_cast<void (Tag::*)(Cast)>(&Tag::Name))

      DECLARE(init,                 );
      DECLARE(init,  const std::string&);
      DECLARE(stop,                 );
      DECLARE(stop,  rObject        );
      DECLARE(block,                );
      DECLARE(block, rObject        );

#undef DECLARE

      BIND_VARIADIC(newFlowControl, new_flow_control);

      BIND(blocked);
      BIND(enter);
      BIND(freeze);
      BIND(frozen);
      BIND(getParent,   parent_get);
      BIND(leave);
      BIND(name);
      BIND(priority);
      BIND(scope);
      BIND(setPriority, priority_set);
      BIND(unblock);
      BIND(unfreeze);
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
    Tag::block()
    {
      block(void_class);
    }

    void
    Tag::block(rObject payload)
    {
      value_->block(::kernel::scheduler(), payload);
      // changed();
    }

    void
    Tag::freeze()
    {
      runner::Job& r = ::kernel::runner();

      value_->freeze();
      // changed();
      if (r.frozen())
        r.yield();
    }

    void
    Tag::init()
    {
      init(libport::Symbol::fresh_string("tag"));
    }

    void
    Tag::init(const std::string& name)
    {
      name_set(name);
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
    Tag::stop()
    {
      stop(void_class);
    }

    void
    Tag::stop(rObject payload)
    {
      value_->stop(::kernel::scheduler(), payload);
      // changed();
    }

    void
    Tag::unblock()
    {
      value_->unblock();
      // changed();
    }

    void
    Tag::unfreeze()
    {
      value_->unfreeze();
      // changed();
    }

    static inline rObject
    tag_event(Tag* owner, libport::Symbol field)
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
      if (rSlot f = local_slot_get(SYMBOL(onEnter)))
      {
        objects_type args;
        args << this;
        eval::call_apply(::kernel::runner(),
                         f->value(), SYMBOL(onEnter), args);
      }
      if (local_slot_get(SYMBOL(enterEvent)))
        slot_get(SYMBOL(enterEvent))->call(SYMBOL(syncEmit));
    }

    void
    Tag::triggerLeave()
    {
      if (rSlot f = local_slot_get(SYMBOL(onLeave)))
      {
        objects_type args;
        args << this;
        eval::call_apply(::kernel::runner(),
                         f->value(), SYMBOL(onLeave), args);
      }
      if (local_slot_get(SYMBOL(leaveEvent)))
        slot_get(SYMBOL(leaveEvent))->call(SYMBOL(syncEmit));
    }

    rTag
    Tag::parent_get()
    {
      return parent_;
    }

    rTag
    Tag::scope()
    {
      runner::Job& r = ::kernel::runner();
      return new Tag(r.state.scope_tag(r.scheduler_get()));
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
  } // namespace object
}
