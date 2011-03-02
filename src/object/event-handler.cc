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

#include <runner/urbi-job.hh>
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
      slot_set(SYMBOL(active), to_urbi(true));
    }

    EventHandler::EventHandler(rEvent source, rList payload)
      : source_(source)
      , payload_(payload)
      , detach_(false) // trigger has to be called before stop.
    {
      proto_add(proto);
      slot_set(SYMBOL(active), to_urbi(true));
    }

    URBI_CXX_OBJECT_INIT(EventHandler)
    {
#define DECLARE(Name, Cxx)            \
      bind(SYMBOL_(Name), &EventHandler::Cxx)

      DECLARE(stop,    stop);
      DECLARE(source,  source);
      DECLARE(payload, payload);
#undef DECLARE
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
        runner::UrbiJob& r = ::kernel::runner();
        sched::jobs_type children;
        // Copy container to avoid in-place modification problems.
        foreach (const stop_job_type& stop_job, stop_jobs_type(stop_jobs_))
        {
          rLobby rlobby = r.state.lobby_get();
          runner::UrbiJob* j =
            new runner::UrbiJob(rlobby,
                                r.scheduler_get(),
                                "onleave");
          j->set_action(eval::exec(stop_job.get<0>(), stop_job.get<1>()));
          sched::rJob job(j);
          job->start_job();
        }
        r.yield_until_terminated(children);
      }
      else
        foreach (const stop_job_type& stop_job, stop_jobs_type(stop_jobs_))
          (*stop_job.get<0>())(stop_job.get<1>());

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
      // Copy the callback list in case it's modified.
      std::vector<callback_type> callbacks;
      callbacks.reserve(source()->callbacks_.size());
      foreach (callback_type* cb, source()->callbacks_)
        callbacks << *cb;
      // Trigger all callbacks.
      foreach (callback_type& cb, callbacks)
        cb(payload_->value_get());
      // Copy container to avoid in-place modification problems.
      foreach (Event::rActions actions, listeners_type(source()->listeners_))
        trigger_job(actions, detach_);
    }

    void
    EventHandler::trigger_job(const rActions& actions, bool detach)
    {
      detach = detach && !actions->sync;
      runner::UrbiJob& r = ::kernel::runner();
      if (actions->frozen)
        return;
      objects_type args;
      args << this << this << payload_;
      rObject pattern = nil_class;
      if (actions->guard)
        pattern = (*actions->guard)(args);
      if (pattern != void_class)
      {
        args << pattern;
        if (actions->leave)
          register_stop_job(stop_job_type(actions->leave, args, detach));
        if (actions->enter)
        {
          if (detach)
          {
            rLobby lobby = actions->lobby;
            if (!lobby)
              lobby = r.state.lobby_get();
            sched::rJob job = actions->enter.get()->make_job
              (lobby, r.scheduler_get(), args, SYMBOL(at));
            job->start_job();
          }
          else
            (*actions->enter)(args);
        }
      }
    }

    void
    EventHandler::register_stop_job(const stop_job_type& stop_job)
    {
      stop_jobs_ << stop_job;
    }
  }
}
