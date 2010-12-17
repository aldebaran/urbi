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
#include <urbi/object/event-handler.hh>
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

    URBI_CXX_OBJECT_INIT(Event)
    {
      typedef void (Event::*emit_type)(const objects_type& args);
      bind_variadic<void, Event>(SYMBOL(emit),
                                 static_cast<emit_type>(&Event::emit));
      bind(SYMBOL(hasSubscribers), &Event::hasSubscribers);
      typedef
      void (Event::*on_event_type)
      (rExecutable guard, rExecutable enter, rExecutable leave);
      bind(SYMBOL(onEvent), static_cast<on_event_type>(&Event::onEvent));
      bind_variadic<void, Event>(SYMBOL(syncEmit), &Event::syncEmit);
      bind_variadic<rEventHandler, Event>(SYMBOL(syncTrigger), &Event::syncTrigger);
      bind_variadic<rEventHandler, Event>(SYMBOL(trigger), &Event::trigger);

#define DECLARE(Name, Cxx)            \
      bind(SYMBOL(Name), &Event::Cxx)

      DECLARE(waituntil, waituntil);

#undef DECLARE
    }

    Event::~Event()
    {
      destructed_();
    }

    /*------------.
    | Callbacks.  |
    `------------*/

    Event::signal_type&
    Event::destructed_get()
    {
      return destructed_;
    }

    Event::signal_type&
    Event::subscribed_get()
    {
      return subscribed_;
    }

    Event::signal_type&
    Event::unsubscribed_get()
    {
      return unsubscribed_;
    }

    void
    Event::unregister(Actions* a)
    {
      // The erase might make us loose the last counted ref on a.
      rActions ra(a);
      listeners_type::iterator it = libport::find(listeners_, a);
      if (it != listeners_.end())
        listeners_.erase(it);
      foreach(boost::signals::connection& c, a->connections)
        c.disconnect();
      a->connections.clear();
      if (listeners_.empty())
        unsubscribed_();
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

    /*-----------------.
    | Urbi functions.  |
    `-----------------*/

    void
    Event::onEvent(rExecutable guard, rExecutable enter, rExecutable leave)
    {
      rActions actions(new Actions(guard, enter, leave));
      runner::Runner& r = ::kernel::runner();
      actions->tag_stack = r.tag_stack_get();
      actions->lobby = r.lobby_get();
      foreach (object::rTag& tag, actions->tag_stack)
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
      foreach (const actives_type::value_type& active, active_)
        active->trigger_job(actions, true);
      if (slot_has(SYMBOL(onSubscribe)))
        slot_get(SYMBOL(onSubscribe))->call(SYMBOL(syncEmit));
      subscribed_();
    }

    void
    Event::waituntil(rObject pattern)
    {
      if (slot_has(SYMBOL(onSubscribe)))
        slot_get(SYMBOL(onSubscribe))->call(SYMBOL(syncEmit));
      // Check whether there's already a matching instance.
      foreach (const actives_type::value_type& active, active_)
        if (pattern == nil_class
            || pattern->call(SYMBOL(match), active->payload())->as_bool())
          return;

      runner::Runner& r = ::kernel::runner();
      rTag t(new Tag);
      waiters_ << Waiter(t, &r, pattern);
      libport::Finally f;
      r.apply_tag(t, &f);
      f << boost::bind(&Event::waituntil_remove, this, t);
      t->freeze(); // Will yield.
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
    Event::source()
    {
      rObject proto = protos_get().front();
      return from_urbi<rEvent>(proto);
    }

    /*-------.
    | Emit.  |
    `-------*/

    // Use an intermediary bouncer to make sure the Executable is
    // stored in a smart pointer, and not deleted too early.
    static void
    executable_bouncer(rExecutable e, objects_type args)
    {
      (*e)(args);
    }

    sched::rJob
    Event::spawn_actions_job(rLobby lobby, rExecutable e,
                             const objects_type& args)
    {
      typedef rObject(Executable::*fun_type)(objects_type);
      runner::Runner& r = ::kernel::runner();
      return new runner::Interpreter
        (lobby ? lobby : r.lobby_get(),
         r.scheduler_get(),
         boost::bind(&executable_bouncer, e, args),
         this, SYMBOL(at));
    }

    void
    Event::emit_backend(const objects_type& pl, bool detach)
    {
      sched::rJob enter, leave;
      rList payload = new List(pl);
      slot_update(SYMBOL(active), to_urbi(false));
      waituntil_release(payload);
      foreach (callback_type* cb, callbacks_type(callbacks_))
        (*cb)(pl);
      // Copy container to avoid in-place modification problems.
      foreach (const Event::rActions& actions, listeners_type(listeners_))
      {
        if (actions->frozen)
          return;
        objects_type args;
        args << this << this << payload;
        rObject pattern = nil_class;
        if (actions->guard)
          pattern = (*actions->guard)(args);
        if (pattern != void_class)
        {
          args << pattern;

          if (detach)
          {
            if (actions->enter)
              enter = spawn_actions_job(actions->lobby, actions->enter, args);
            if (actions->leave)
              leave = spawn_actions_job(actions->lobby, actions->leave, args);

            // Start jobs simultaneously.
            if (actions->enter)
              enter->start_job();
            if (actions->leave)
              leave->start_job();
          }
          else
          {
            if (actions->enter)
              (*actions->enter)(args);
            if (actions->leave)
              (*actions->leave)(args);
          }
        }
      }

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

    /*----------.
    | Trigger.  |
    `----------*/

    rEventHandler
    Event::trigger_backend(const objects_type& pl, bool detach)
    {
      rList payload = new List(pl);
      rEventHandler handler = new EventHandler(this, payload);
      waituntil_release(payload);
      handler->trigger(detach);
      return handler;
    }

    rEventHandler
    Event::syncTrigger(const objects_type& pl)
    {
      return trigger_backend(pl, false);
    }

    rEventHandler
    Event::trigger(const objects_type& pl)
    {
      return trigger_backend(pl, true);
    }

    void
    Event::waituntil_release(rObject payload)
    {
      // This iteration needs to remove some elements as it goes.
      for (unsigned i = 0; i < waiters_.size(); )
      {
	Waiter& waiter = waiters_[i];
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
	  waiters_[i] = waiters_[waiters_.size()-1];
	  waiters_.pop_back();
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

    Event::Subscription::Subscription(rEvent event, callback_type* cb)
      : event_(event)
      , cb_(cb)
    {}

    void
    Event::Subscription::stop()
    {
      Event::callbacks_type::iterator it = libport::find(event_->callbacks_, cb_);
      assert(it != event_->callbacks_.end());
      delete *it;
      event_->callbacks_.erase(it);
    }
  }
}
