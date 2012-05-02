/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <algorithm>

#include <urbi/object/symbols.hh>
#include <runner/job.hh>
#include <urbi/kernel/userver.hh>
#include <urbi/object/event-handler.hh>
#include <urbi/object/event.hh>
#include <urbi/object/lobby.hh>
#include <object/profile.hh>
#include <urbi/sdk.hh>

#include <eval/call.hh>
#include <eval/exec.hh>
#include <eval/send-message.hh>

#include <urbi/runner/raise.hh>

GD_CATEGORY(Urbi.Event);

namespace urbi
{
  namespace object
  {

    /*---------.
    | Waiter.  |
    `---------*/

    inline
    Event::Waiter::Waiter(rTag ct, runner::Job* r, rObject& p)
      : controlTag(ct), runner(r), pattern(p)
    {}

    /*--------.
    | Event.  |
    `--------*/

    Event::Event()
      : waiters_()
      , onSubscribe_(0)
      , callbacks_()
    {
      proto_add(proto);
      slot_set_value(SYMBOL(active), to_urbi(false));
    }

    Event::Event(rEvent model)
      : waiters_()
      , onSubscribe_(0)
      , callbacks_(model->callbacks_)
    {
      proto_add(model);
      slot_set_value(SYMBOL(active), to_urbi(false));
    }

    URBI_CXX_OBJECT_INIT(Event)
      : waiters_()
      , onSubscribe_(0)
      , callbacks_()
    {
      BINDG(hasSubscribers);
      BIND(onEvent, onEvent, on_event_type);
      BIND(onSubscribe, onSubscribe_);
      BIND(subscribe);
      BIND(subscribers, callbacks_);
      BIND(waituntil);
      BIND_VARIADIC(emit);
      BIND_VARIADIC(syncEmit);
      BIND_VARIADIC(syncTrigger);
      BIND_VARIADIC(trigger);
    }

    Event::~Event()
    {
      if (destructed)
        destructed();
    }

    void
    Event::subscribe(rSubscription s)
    {
      aver(s);
      GD_FINFO_TRACE("%s: New subscription %s", this, s);
      callbacks_ << s;
      if (onSubscribe_)
        onSubscribe_->syncEmit();
    }

    rSubscription
    Event::onEvent(const callback_type& cb)
    {
      aver(!cb.empty());
      rSubscription res = new Subscription(new callback_type(cb));
      res->asynchronous_ = false;
      callbacks_ << res;
      if (onSubscribe_)
        onSubscribe_->syncEmit();
      return res;
    }

    /*-----------------.
    | Urbi functions.  |
    `-----------------*/

    void
    Event::onEvent(rExecutable guard, rExecutable enter, rExecutable leave,
                   bool sync)
    {
      rSubscription sub(new Subscription(this, guard, enter, leave, sync));
      GD_FPUSH_TRACE("%s: New registration %s.", this, sub);
      runner::Job& r = ::kernel::runner();
      sub->call_stack = r.state.call_stack_get();
      const libport::Symbol& sep =
        SYMBOL(MINUS_MINUS_MINUS_MINUS_SP_event_SP_handler_SP_backtrace_COLON);
      sub->call_stack << std::make_pair(sep, boost::optional<ast::loc>());

      sub->profile = r.profile_get();
      sub->tag_stack = r.state.tag_stack_get();
      sub->lobby = r.state.lobby_get();
      foreach (object::rTag& tag, sub->tag_stack)
      {
        GD_FINFO_DEBUG("%s: Hook tag %s.", this, tag->name());
        sched::rTag t = tag->value_get();
        using boost::bind;
        sub->connections
          << t->stop_hook_get().connect(bind(&Subscription::unregister, sub))
          << t->freeze_hook_get().connect(bind(&Subscription::freeze, sub))
          << t->unfreeze_hook_get().connect(bind(&Subscription::unfreeze, sub));
      }
      subscribe(sub);

      foreach (const rEventHandler& active, active_)
      {
        // FIXME: duplication with emit_backend.
        objects_type args;
        args << this << this << active->payload();
        rObject pattern = nil_class;
        if (sub->guard)
        {
          pattern = (*sub->guard)(args);
          if (pattern == void_class)
            continue;
        }
        args << pattern;
        if (sub->leave_)
          *active << EventHandler::stop_job_type(sub, args, true);
        active->trigger_job(sub, args, true);
      }
      if (subscribed)
        subscribed();
    }

    void
    Event::waituntil(rObject pattern)
    {
      if (onSubscribe_)
        onSubscribe_->syncEmit();
      // Check whether there's already a matching instance.
      foreach (const actives_type::value_type& active, active_)
        if (pattern == nil_class
            || pattern->call(SYMBOL(match), active->payload())->as_bool())
          return;

      runner::Job& r = ::kernel::runner();
      rTag t(new Tag);
      waiters_ << Waiter(t, &r, pattern);
      libport::Finally f;
      r.state.apply_tag(t, &f);
      f << boost::bind(&Event::waituntil_remove, this, t);
      t->freeze(); // Will yield.
    }

    void
    Event::waituntil_remove(rTag what)
    {
      for (unsigned i = 0; i < waiters_.size(); ++i)
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
      rObject proto = protos_get_first();
      return from_urbi<rEvent>(proto);
    }

    /*-------.
    | Emit.  |
    `-------*/

    runner::rJob
    Event::action_job(rLobby lobby, const call_stack_type& stack,
                      rExecutable e,
                      rProfile profile, const objects_type& args)
    {
      runner::Job& r = ::kernel::runner();
      runner::Job* res =
        e->make_job(lobby, r.scheduler_get(), args, SYMBOL(event));
      // Append the back-trace of the event handler (the "at") below
      // that of the emission back trace.
      res->state
        .call_stack_get()
        .insert(res->state.call_stack_get().begin(),
                stack.begin(), stack.end());
      if (profile)
        res->profile_start(profile, SYMBOL(event), e.get());
      return res;
    }

    void
    Event::emit_backend(const objects_type& pl, bool detach, EventHandler* h)
    {
      GD_FPUSH_TRACE("%s: Emit, %s subscribers.", this, callbacks_.size());
      rList payload = new List(pl);
      if (!h)
        slot_update(SYMBOL(active), to_urbi(false));
      waituntil_release(payload);
      bool empty = callbacks_.empty();
      callbacks_type cbcopy;
      // Copy active subscriptions, cleanup list
      for (unsigned i = 0; i < callbacks_.size(); ++i)
      {
        rSubscription& s = callbacks_[i];
        if (s->disconnected_get())
        {
          callbacks_[i] = callbacks_[callbacks_.size()-1];
          callbacks_.pop_back();
          --i;
        }
        else
          cbcopy << s;
      }
      // List was empty and we didn't notice.
      if (!empty && callbacks_.empty())
      {
        GD_INFO_TRACE("No more subscribers, calling unsubscribed");
        if (unsubscribed)
          unsubscribed();
      }
      GD_FINFO_TRACE("%s subscribers after cleanup", callbacks_.size());
      for (unsigned i = 0; i < cbcopy.size(); ++i)
      {
        rSubscription s = cbcopy[i];
        aver(s);
        libport::utime_t now = kernel::server().getTime();
        GD_FPUSH_TRACE
          ("Considering %s, mi %s(%s), lc %s(%s), now %s, enabl %s, async %s",
           s,
           s->minInterval_, libport::seconds_to_utime(s->minInterval_),
           s->lastCall_, libport::seconds_to_utime(s->lastCall_),
           now,
           s->enabled_,
           s->asynchronous_);
        if (s->disconnected_get())
        {
          GD_INFO_TRACE("Subscriber is disconnected");
          // For removal we just swap with the last for better performances.
          cbcopy[i] = cbcopy[cbcopy.size()-1];
          cbcopy.pop_back();
            --i;
        }
        else if (s->enabled_
                 && now - libport::seconds_to_utime(s->lastCall_) >=
                 libport::seconds_to_utime(s->minInterval_)
                 && (!s->maxParallelEvents_
                     || s->maxParallelEvents_ > s->processing_))
        {
          // FIXME: CRAP if we honor the event emit sync/at sync rule,
          // no way to catch changed asynchronously
          bool async =
            (s->event_ && (detach && s->asynchronous_get()))
            || (!s->event_ && (detach || s->asynchronous_get()));
          GD_FINFO_TRACE("Subscriber is live for notification"
                         " (cb: %s e: %s), async: %s",
                         s->cb_, s->event_, async);
          if (s->frozen)
          {
            GD_FINFO_TRACE("%s: Skip frozen registration %s.", this, s);
            continue;
          }
          // FIXME: duplication with onEvent.
          objects_type args;
          args << this << this << payload;
          rObject pattern = nil_class;
          if (s->guard)
          {
            pattern = (*s->guard)(args);
            if (pattern == void_class)
            {
              GD_FINFO_TRACE("%s: Skip pattern mismatch %s.", this, s);
              continue;
            }
          }
          args << pattern;
          if (h && s->leave_)
            *h << EventHandler::stop_job_type(s, args, detach);
          if (async)
          {
            // If we create a job, it can die before executing a single line
            // of code.
            // To avoid any race condition, we just create the job without
            // touching any stat or holding any lock.
            eval::Action a =
              eval::exec(boost::bind(&Subscription::run_sync,
                                     s, this, pl, h, detach, false, args),
                         this);
            runner::rJob j =
              new runner::Job(s->lobby, kernel::runner().scheduler_get());
            j->set_action(a);
            j->state.tag_stack_set(s->tag_stack);
            GD_FINFO_DUMP("Subscriber will run in job %s", j);
            j->start_job();
          }
          else
            s->run_sync(this, pl, h, detach, true, args);
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
          foreach (const rTag& t, waiter.runner->state.tag_stack_get_all())
            if (t != waiter.controlTag && t->frozen())
            {
              frozen = true;
              ++i;
              break;
            }
          if (!frozen) // Do not trigger a frozen at.
          {
            waiter.controlTag->unfreeze();
            // Yes this is also valid for the last element.
            waiters_[i] = waiters_[waiters_.size()-1];
            waiters_.pop_back();
          }
	}
	else
	  ++i;
      }
    }

    bool
    Event::hasSubscribers() const
    {
      return !waiters_.empty() || !callbacks_.empty();
    }

  }
}
