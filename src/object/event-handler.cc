/*
 * Copyright (C) 2010, 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <object/symbols.hh>
#include <urbi/object/event.hh>
#include <urbi/object/event-handler.hh>

#include <runner/job.hh>
#include <eval/exec.hh>

namespace urbi
{
  namespace object
  {
    EventHandler::EventHandler(rEventHandler model)
      : source_(model->source_)
      , payload_(model->payload_)
      , detach_(false) // trigger has to be called before stop.
    {
      proto_add(model);
      slot_set_value(SYMBOL(active), to_urbi(true));
    }

    EventHandler::EventHandler(rEvent source, rList payload)
      : source_(source)
      , payload_(payload)
      , detach_(false) // trigger has to be called before stop.
    {
      proto_add(proto);
      slot_set_value(SYMBOL(active), to_urbi(true));
    }

    URBI_CXX_OBJECT_INIT(EventHandler)
    {
      BIND(payload);
      BIND(source);
      BIND(stop);
    }


    static
    void
    spawn_actions_job(rSubscription sub,
                      rExecutable action, const objects_type& args)
    {
      rLobby l = sub->lobby;
      if (!l)
        l = ::kernel::runner().state.lobby_get();
      runner::rJob job =
        Event::spawn_actions_job(l, sub->call_stack,
                                 action, sub->profile, args);
      job->start_job();
    }

    /*-------.
    | Stop.  |
    `-------*/
    void
    EventHandler::stop()
    {
      slot_update(SYMBOL(active), to_urbi(false));
      if (detach_)
      {
        runner::Job& r = ::kernel::runner();
        sched::jobs_type children;
        // Copy container to avoid in-place modification problems.
        foreach (const stop_job_type& stop_job, stop_jobs_type(stop_jobs_))
        {
          rSubscription actions = stop_job.get<0>();
          if (actions->leave_)
            spawn_actions_job(actions,
                              actions->leave_, stop_job.get<1>());
        }
        r.yield_until_terminated(children);
      }
      else
        foreach (const stop_job_type& stop_job, stop_jobs_type(stop_jobs_))
          (*stop_job.get<0>()->leave_)(stop_job.get<1>());

      stop_jobs_.clear();
      source()->active_.erase(this);
    }

    rList
    EventHandler::payload()
    {
      return payload_;
    }

    rEvent
    EventHandler::source()
    {
      return source_;
    }

    void
    EventHandler::trigger(bool detach)
    {
      detach_ = detach;
      source()->active_.insert(this);
      source()->emit_backend(payload()->value_get(), detach_, this);
    }

    void
    EventHandler::trigger_job(const rSubscription& actions, bool detach,
                              objects_type& args)
    {
      detach = detach && actions->asynchronous_get();
      if (actions->enter_)
      {
        if (detach)
          spawn_actions_job(actions, actions->enter_, args);
        else
          actions->enter(args);
      }
    }

    void
    EventHandler::register_stop_job(const stop_job_type& stop_job)
    {
      stop_jobs_ << stop_job;
    }
  }
}
