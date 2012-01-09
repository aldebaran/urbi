/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
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

#include <urbi/kernel/userver.hh>

#include <urbi/object/tag.hh>
#include <urbi/object/global.hh>
#include <urbi/object/object.hh>
#include <urbi/object/string.hh>
#include <urbi/object/symbols.hh>

#include <runner/interpreter.hh>
#include <runner/runner.hh>

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

      bind_variadic(SYMBOL(newFlowControl), &Tag::new_flow_control);

#define DECLARE(Name, Function)                 \
      bind(SYMBOL_(Name), &Tag::Function)

      DECLARE(blocked,     blocked);
      DECLARE(enter,       enter);
      DECLARE(freeze,      freeze);
      DECLARE(frozen,      frozen);
      DECLARE(getParent,   parent_get);
      DECLARE(leave,       leave);
      DECLARE(name,        name);
      DECLARE(priority,    priority);
      DECLARE(scope,       scope);
      DECLARE(setPriority, priority_set);
      DECLARE(unblock,     unblock);
      DECLARE(unfreeze,    unfreeze);

#undef DECLARE
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
      runner::Runner& r = ::kernel::runner();

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
        ::kernel::runner().apply(f->value(), SYMBOL(onEnter), args);
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
        ::kernel::runner().apply(f->value(), SYMBOL(onLeave), args);
      }
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
  } // namespace object
}
