/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/object/symbols.hh>
#include <urbi/object/event.hh>
#include <urbi/object/event-handler.hh>
#include <object/profile.hh>

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
      : source_(0)
      , payload_(0)
      , detach_(false)
    {
      BIND(payload);
      BIND(source);
      BIND(stop);
    }


    /*-------.
    | Stop.  |
    `-------*/

    void
    EventHandler::stop()
    {
      slot_update(SYMBOL(active), to_urbi(false));
      // Copy container to avoid in-place modification problems.
      foreach (const stop_job_type& stop_job, stop_jobs_type(stop_jobs_))
        stop_job.subscription->leave(stop_job.args, detach_);
      stop_jobs_.clear();
      source()->active_.erase(this);
    }

    void
    EventHandler::trigger(bool detach)
    {
      detach_ = detach;
      source()->active_.insert(this);
      source()->emit_backend(payload()->value_get(), detach_, this);
    }

    void
    EventHandler::trigger_job(const rSubscription& sub,
                              const objects_type& args,
                              bool detach)
    {
      sub->enter(args, detach && sub->asynchronous_get());
    }

  }
}
