/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <algorithm>

#include <kernel/userver.hh>
#include <object/symbols.hh>
#include <runner/interpreter.hh>
#include <urbi/object/event.hh>
#include <urbi/object/lobby.hh>
#include <urbi/sdk.hh>

namespace urbi
{
  namespace object
  {
    Event::Event(rEvent model)
      : listeners_(model->listeners_)
      , waiters_()
    {
      proto_add(model);
      slot_set(SYMBOL(active), to_urbi(false));
    }

    Event::Event(rEvent parent, rList payload)
      : listeners_(parent->listeners_)
      , waiters_()
    {
      proto_add(parent);
      slot_set(SYMBOL(active), to_urbi(true));
      slot_set(SYMBOL(payload), payload);
    }

    void
    Event::unregister(Actions a)
    {
      Listeners::iterator it = libport::find(listeners_, a);
      if (it != listeners_.end())
        listeners_.erase(it);
    }

    void
    Event::freeze(Actions a)
    {
      Listeners::iterator it = libport::find(listeners_, a);
      if (it != listeners_.end())
        it->active = false;
    }

    void
    Event::unfreeze(Actions a)
    {
      Listeners::iterator it = libport::find(listeners_, a);
      if (it != listeners_.end())
        it->active = true;
    }

    /*---------------.
    | Urbi functions |
    `---------------*/

    void
    Event::onEvent(rExecutable guard, rExecutable enter, rExecutable leave)
    {
      Actions actions(guard, enter, leave);

      runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
      foreach (object::rTag tag, r.tag_stack_get())
      {
        sched::rTag t = tag->value_get();
        using boost::bind;
        t->stop_hook_get().connect(bind(&Event::unregister, this, actions));
        t->freeze_hook_get().connect(bind(&Event::freeze, this, actions));
        t->unfreeze_hook_get().connect(bind(&Event::unfreeze, this, actions));
      }

      listeners_ << actions;
      foreach (const actives_type::value_type& active, _active)
        active.first->trigger_job(Actions(guard, enter, leave));
    }

    void
    Event::trigger_job(const Actions& actions)
    {
      if (!actions.active)
        return;
      objects_type args;
      args << this << this;
      rObject pattern = (*actions.guard)(args);
      if (pattern != void_class)
      {
        args << pattern;
        register_stop_job(stop_job_type(actions.leave, args));
        (*actions.enter)(args);
      }
    }

    void
    Event::syncEmit(const objects_type& pl)
    {
      rEvent instance = new Event(this,
                                  new List(pl));

      instance->slot_update(SYMBOL(active), to_urbi(false));
      instance->localTrigger(pl);
      instance->stop();
    }

    void
    Event::emit(const objects_type& args)
    {
      runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();

      rObject code = slot_get(SYMBOL(syncEmit));
      sched::rJob job = new runner::Interpreter
        (r.lobby_get(), r.scheduler_get(), code,
         SYMBOL(syncEmit), rObject(this), args);
      job->start_job();
    }

    void
    Event::waituntil(rObject pattern)
    {
      runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
      waiters_.push_back(std::make_pair(runner::rRunner(&r), pattern));

      // Check whether there's already a matching instance.
      foreach (const actives_type::value_type& active, _active)
        if (pattern == nil_class || pattern->call(SYMBOL(match), active.second)->as_bool())
          return;

      r.frozen_set(true);
      // Yield for the frozen state to be taken in account.
      yield();
    }

    rEvent
    Event::syncTrigger(const objects_type& pl)
    {
      rEvent instance = new Event(this, new List(pl));

      instance->localTrigger(pl);
      return instance;
    }

    rEvent
    Event::trigger(const objects_type& args)
    {
      runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
      rEvent instance = new Event(this, new List(args));

      rObject code = slot_get(SYMBOL(localTrigger));
      sched::rJob job = new runner::Interpreter
        (r.lobby_get(), r.scheduler_get(), code,
         SYMBOL(localTrigger), instance, args);
      job->start_job();
      return instance;
    }

    rEvent
    Event::source()
    {
      rObject proto = protos_get().front();
      type_check<Event>(proto);
      return proto->as<Event>();
    }

    void
    Event::localTrigger(const objects_type& pl)
    {
      rList payload = new List(pl);
      source()->_active[this] = payload;
      foreach (const Event::Actions& actions, listeners_)
        trigger_job(actions);
      waituntil_release(payload);
    }

    void
    Event::register_stop_job(const stop_job_type& stop_job)
    {
      stop_jobs_ << stop_job;
    }

    void
    Event::stop()
    {
      slot_update(SYMBOL(active), to_urbi(false));
      foreach (const stop_job_type& stop_job, stop_jobs_)
        (*stop_job.first)(stop_job.second);
      source()->_active.erase(this);
      stop_jobs_.clear();
    }

    void
    Event::waituntil_release(rObject payload)
    {
      rEvent src = source();
      foreach (const waiter_type& waiter, src->waiters_)
        if (waiter.second == nil_class || waiter.second->call(SYMBOL(match), payload)->as_bool())
          waiter.first->frozen_set(false);
    }

    bool
    Event::hasSubscribers() const
    {
      return !listeners_.empty() || !waiters_.empty();
    }

    /*-------------.
    | Urbi binding |
    `-------------*/

    URBI_CXX_OBJECT_REGISTER(Event)
    {
      bind(SYMBOL(emit), &Event::emit);
      bind(SYMBOL(hasSubscribers), &Event::hasSubscribers);
      bind(SYMBOL(localTrigger), &Event::localTrigger);
      bind(SYMBOL(onEvent), &Event::onEvent);
      bind(SYMBOL(stop), &Event::stop);
      bind(SYMBOL(syncEmit), &Event::syncEmit);
      bind(SYMBOL(syncTrigger), &Event::syncTrigger);
      bind(SYMBOL(trigger), &Event::trigger);
      bind(SYMBOL(waituntil), &Event::waituntil);
    }
  }
}

