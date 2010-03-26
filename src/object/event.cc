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
    Event::Event()
      : listeners_()
      , waiters_()
    {
      proto_add(proto);
      slot_set(SYMBOL(active), to_urbi(false));
    }

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
      , callbacks_(parent->callbacks_)
    {
      proto_add(parent);
      slot_set(SYMBOL(active), to_urbi(true));
      slot_set(SYMBOL(payload), payload);
    }

    void
    Event::unregister(rActions a)
    {
      Listeners::iterator it = libport::find(listeners_, a);
      if (it != listeners_.end())
        listeners_.erase(it);
      foreach(boost::signals::connection& c, a->connections)
        c.disconnect();
      a->connections.clear();
    }

    void
    Event::freeze(rActions a)
    {
      a->active = false;
    }

    void
    Event::unfreeze(rActions a)
    {
      a->active = true;
    }

    /*---------------.
    | Urbi functions |
    `---------------*/

    void
    Event::onEvent(rExecutable guard, rExecutable enter, rExecutable leave)
    {
      rActions actions(new Actions(guard, enter, leave));
      runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
      actions->tag_stack = r.tag_stack_get();
      foreach (object::rTag tag, actions->tag_stack)
      {
        sched::rTag t = tag->value_get();
        using boost::bind;
        actions->connections.push_back(
          t->stop_hook_get().connect(bind(&Event::unregister, this, actions)));
        actions->connections.push_back(
          t->freeze_hook_get().connect(bind(&Event::freeze, this, actions)));
        actions->connections.push_back(
          t->unfreeze_hook_get().connect(bind(&Event::unfreeze, this, actions)));
      }

      listeners_ << actions;
      foreach (const actives_type::value_type& active, _active)
        active.first->trigger_job(actions, true);
      if (slot_has(SYMBOL(onSubscribe)))
        slot_get(SYMBOL(onSubscribe))->call(SYMBOL(syncEmit));
    }

    void
    Event::onEvent(const callback_type& cb)
    {
      callbacks_ << cb;
    }

    void
    Event::trigger_job(const rActions& actions, bool detach)
    {
      runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
      if (!actions->active)
        return;
      objects_type args;
      args << this << this;
      rObject pattern = (*actions->guard)(args);
      if (pattern != void_class)
      {
        args << pattern;
        register_stop_job(stop_job_type(actions->leave, args));
        if (detach)
        {
          sched::rJob job = new runner::Interpreter
            (r.lobby_get(), r.scheduler_get(),
             boost::bind(static_cast<rObject(Executable::*)(objects_type)>(&Executable::operator()), actions->enter.get(), args),
             this, SYMBOL(at));
          job->start_job();
        }
        else
          (*actions->enter)(args);
      }
    }

    void
    Event::emit_backend(const objects_type& pl, bool detach)
    {
      rEvent instance = new Event(this, new List(pl));

      instance->slot_update(SYMBOL(active), to_urbi(false));
      instance->localTrigger(pl, detach);
      instance->stop_backend(detach);
    }

    void
    Event::syncEmit(const objects_type& pl)
    {
      emit_backend(pl, false);
    }

    void
    Event::emit(const objects_type& args)
    {
      emit_backend(args, true);
    }

    void
    Event::emit()
    {
      emit(objects_type());
    }

    void
    Event::waituntil(rObject pattern)
    {
      runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
      waiters_.push_back(std::make_pair(runner::rRunner(&r), pattern));

      if (slot_has(SYMBOL(onSubscribe)))
        slot_get(SYMBOL(onSubscribe))->call(SYMBOL(syncEmit));
      // Check whether there's already a matching instance.
      foreach (const actives_type::value_type& active, _active)
        if (pattern == nil_class || pattern->call(SYMBOL(match), active.second)->as_bool())
          return;

      r.frozen_set(true);
      // Yield for the frozen state to be taken in account.
      yield();
    }

    rEvent
    Event::trigger_backend(const objects_type& pl, bool detach)
    {
      rEvent instance = new Event(this, new List(pl));

      instance->localTrigger(pl, detach);
      return instance;
    }

    rEvent
    Event::syncTrigger(const objects_type& pl)
    {
      return trigger_backend(pl, false);
    }

    rEvent
    Event::trigger(const objects_type& pl)
    {
      return trigger_backend(pl, true);
    }

    rEvent
    Event::source()
    {
      rObject proto = protos_get().front();
      type_check<Event>(proto);
      return proto->as<Event>();
    }

    void
    Event::localTrigger(const objects_type& pl, bool detach)
    {
      rList payload = new List(pl);
      source()->_active[this] = payload;
      waituntil_release(payload);
      foreach (const callback_type& cb, callbacks_)
        cb(pl);
      foreach (Event::rActions actions, listeners_)
        trigger_job(actions, detach);
    }

    void
    Event::register_stop_job(const stop_job_type& stop_job)
    {
      stop_jobs_ << stop_job;
    }

    void
    Event::stop()
    {
      stop_backend(false);
    }

    void
    Event::stop_backend(bool detach)
    {
      runner::Runner& r = ::kernel::urbiserver->getCurrentRunner();
      slot_update(SYMBOL(active), to_urbi(false));
      if (detach)
      {
        sched::jobs_type children;
        foreach (const stop_job_type& stop_job, stop_jobs_)
        {
          sched::rJob job = new runner::Interpreter
            (r.lobby_get(), r.scheduler_get(),
             boost::bind(static_cast<rObject(Executable::*)(objects_type)>(&Executable::operator()), stop_job.first.get(), stop_job.second),
             this, SYMBOL(onleave));
          job->start_job();
        }
        r.yield_until_terminated(children);
      }
      else
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
          waiter.first.unchecked_cast<runner::Runner>()->frozen_set(false);
    }

    bool
    Event::hasSubscribers() const
    {
      return !listeners_.empty() || !waiters_.empty();
    }

    Event::Actions::~Actions()
    {
      foreach(boost::signals::connection& c, connections)
        c.disconnect();
    }

    /*-------------.
    | Urbi binding |
    `-------------*/

    URBI_CXX_OBJECT_REGISTER(Event)
    {
      bind(SYMBOL(emit), static_cast<void (Event::*)(const objects_type&)>(&Event::emit));
      bind(SYMBOL(hasSubscribers), &Event::hasSubscribers);
      bind(SYMBOL(localTrigger), &Event::localTrigger);
      bind(SYMBOL(onEvent), static_cast<void (Event::*)(rExecutable guard, rExecutable enter, rExecutable leave)>(&Event::onEvent));
      bind(SYMBOL(stop), &Event::stop);
      bind(SYMBOL(syncEmit), &Event::syncEmit);
      bind(SYMBOL(syncTrigger), &Event::syncTrigger);
      bind(SYMBOL(trigger), &Event::trigger);
      bind(SYMBOL(waituntil), &Event::waituntil);
    }
  }
}

