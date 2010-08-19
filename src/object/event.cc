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
      , callbacks_()
      , callbacks_instance_()
    {
      foreach (callback_type* cb, parent->callbacks_)
        callbacks_instance_ << *cb;
      proto_add(parent);
      slot_set(SYMBOL(active), to_urbi(true));
      slot_set(SYMBOL(payload), payload);
    }

    void
    Event::unregister(Actions* a)
    {
      // The erase might make us loose the last counted ref on a.
      rActions ra(a);
      Listeners::iterator it = libport::find(listeners_, a);
      if (it != listeners_.end())
        listeners_.erase(it);
      foreach(boost::signals::connection& c, a->connections)
        c.disconnect();
      a->connections.clear();
    }

    void
    Event::freeze(Actions* a)
    {
      a->frozen++;
    }

    void
    Event::unfreeze(Actions* a)
    {
      aver(a->frozen);
      a->frozen--;
    }

    /*-----------------.
    | Urbi functions.  |
    `-----------------*/

    typedef
    void (Event::*on_event_type)
    (rExecutable guard, rExecutable enter, rExecutable leave);

    void
    Event::onEvent(rExecutable guard, rExecutable enter, rExecutable leave)
    {
      rActions actions(new Actions(guard, enter, leave));
      runner::Runner& r = ::kernel::runner();
      actions->tag_stack = r.tag_stack_get();
      foreach (object::rTag tag, actions->tag_stack)
      {
        sched::rTag t = tag->value_get();
        using boost::bind;
        actions->connections
          << t->stop_hook_get().connect(
            bind(&Event::unregister, this, actions.get()))
          << t->freeze_hook_get().connect(
            bind(&Event::freeze, this, actions.get()))
          << t->unfreeze_hook_get().connect(
            bind(&Event::unfreeze, this, actions.get()));
      }

      listeners_ << actions;
      foreach (const actives_type::value_type& active, _active)
        active.first->trigger_job(actions, true);
      if (slot_has(SYMBOL(onSubscribe)))
        slot_get(SYMBOL(onSubscribe))->call(SYMBOL(syncEmit));
    }

    Event::Subscription
    Event::onEvent(const callback_type& cb)
    {
      aver(!cb.empty());
      callback_type* res = new callback_type(cb);
      callbacks_ << res;
      if (slot_has(SYMBOL(onSubscribe)))
        slot_get(SYMBOL(onSubscribe))->call(SYMBOL(syncEmit));
      return Subscription(this, res);
    }

    void
    Event::trigger_job(const rActions& actions, bool detach)
    {
      runner::Runner& r = ::kernel::runner();
      if (actions->frozen)
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
          typedef rObject(Executable::*fun_type)(objects_type);
          sched::rJob job =
            new runner::Interpreter
            (r.lobby_get(),
             r.scheduler_get(),
             boost::bind(static_cast<fun_type>(&Executable::operator()),
                         actions->enter.get(), args),
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

    typedef void (Event::*emit_type)(const objects_type& args);
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
      if (slot_has(SYMBOL(onSubscribe)))
        slot_get(SYMBOL(onSubscribe))->call(SYMBOL(syncEmit));
      // Check whether there's already a matching instance.
      foreach (const actives_type::value_type& active, _active)
        if (pattern == nil_class
            || pattern->call(SYMBOL(match), active.second)->as_bool())
          return;

      runner::Runner& r = ::kernel::runner();
      rTag t(new Tag);
      waiters_ << Waiter(t, &r, pattern);
      libport::Finally f;
      r.apply_tag(t, &f);
      f << boost::bind(&Event::waituntil_remove, this, t);
      t->freeze();// Will yield
    }

    void
    Event::waituntil_remove(rTag what)
    {
      for(unsigned i=0; i < waiters_.size(); ++i)
        if (waiters_[i].controlTag == what)
        {
          waiters_[i] = waiters_[waiters_.size()-1];
          waiters_.pop_back();
          return;
        }
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

      foreach (const callback_type& cb, callbacks_instance_)
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
      stop_backend(true);
    }

    // Use an intermediary bouncer to make sure the Executable is
    // stored in a smart pointer, and not deleted too early.
    static void
    executable_bouncer(rExecutable e, objects_type args)
    {
      (*e)(args);
    }

    void
    Event::stop_backend(bool detach)
    {
      runner::Runner& r = ::kernel::runner();
      slot_update(SYMBOL(active), to_urbi(false));
      if (detach)
      {
        sched::jobs_type children;
        foreach (const stop_job_type& stop_job, stop_jobs_)
        {
          typedef rObject(Executable::*fun_type)(objects_type);
          sched::rJob job = new runner::Interpreter
            (r.lobby_get(), r.scheduler_get(),
             boost::bind(&executable_bouncer,
                         stop_job.first, stop_job.second),
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
      // This iteration needs to remove some elements as it goes.
      for (unsigned i = 0; i < src->waiters_.size(); )
      {
	Waiter& waiter = src->waiters_[i];
	if (waiter.pattern == nil_class
            || waiter.pattern->call(SYMBOL(match), payload)->as_bool())
	{
          // Check if any tag is frozen beside the first one.
          bool frozen = false;
          foreach (const rTag& t, waiter.runner->tag_stack_get_all())
          {
            if (t != waiter.controlTag && t->frozen())
            {
              frozen = true;
              break;
            }
          }
          if (frozen) // Do not trigger a frozen at.
          {
            ++i;
            continue;
          }
	  waiter.controlTag->unfreeze();
	  // Yes this is also valid for the last element.
	  src->waiters_[i] = src->waiters_[src->waiters_.size()-1];
	  src->waiters_.pop_back();
	}
	else
	  ++i;
      }
    }

    bool
    Event::hasSubscribers() const
    {
      return !listeners_.empty() || !waiters_.empty() || !callbacks_.empty();
    }

    Event::Actions::~Actions()
    {
      foreach(boost::signals::connection& c, connections)
        c.disconnect();
    }

    /*---------------.
    | Subscription.  |
    `---------------*/

    Event::Subscription::Subscription(rEvent event, const callback_type* cb)
      : event_(event)
      , cb_(cb)
    {}

    void
    Event::Subscription::stop()
    {
      for (Event::callbacks_type::iterator it = event_->callbacks_.begin();
           it != event_->callbacks_.end();
           ++it)
        if (*it == cb_)
        {
          delete *it;
          event_->callbacks_.erase(it);
          return;
        }
    }

    /*---------------.
    | Urbi binding.  |
    `---------------*/

    URBI_CXX_OBJECT_REGISTER(Event)
    {
      bind(SYMBOL(emit), static_cast<emit_type>(&Event::emit));
      bind(SYMBOL(hasSubscribers), &Event::hasSubscribers);
      bind(SYMBOL(localTrigger), &Event::localTrigger);
      bind(SYMBOL(onEvent), static_cast<on_event_type>(&Event::onEvent));
      bind(SYMBOL(stop), &Event::stop);
      bind(SYMBOL(syncEmit), &Event::syncEmit);
      bind(SYMBOL(syncTrigger), &Event::syncTrigger);
      bind(SYMBOL(trigger), &Event::trigger);
      bind(SYMBOL(waituntil), &Event::waituntil);
    }
  }
}
