/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <object/symbols.hh>
#include <urbi/object/event.hh>
#include <urbi/object/event_handler.hh>

namespace urbi
{
  namespace object
  {
    EventHandler::EventHandler(rEventHandler model)
      : source_(model->source_)
      , payload_(model->payload_)
    {
      proto_add(model);
      slot_set(SYMBOL(active), to_urbi(true));
    }

    EventHandler::EventHandler(rEvent source, rList payload)
      : source_(source)
      , payload_(payload)
    {
      proto_add(proto);
      slot_set(SYMBOL(active), to_urbi(true));
    }

    URBI_CXX_OBJECT_INIT(EventHandler)
    {
#define DECLARE(Name, Cxx)            \
      bind(SYMBOL(Name), &EventHandler::Cxx)

      DECLARE(stop,    stop);
      DECLARE(source,  source);
      DECLARE(payload, payload);
#undef DECLARE
    }

    void
    EventHandler::stop()
    {
      slot_update(SYMBOL(active), to_urbi(false));
      source()->stop_backend(true);
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
      source()->active_.insert(this);
      foreach (callback_type* cb, callbacks_type(source()->callbacks_))
        (*cb)(payload_->value_get());
      foreach (Event::rActions actions, listeners_type(source()->listeners_))
        trigger_job(actions, detach);
    }

    void
    EventHandler::trigger_job(const rActions& actions, bool detach)
    {
      runner::Runner& r = ::kernel::runner();
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
          source()->register_stop_job(stop_job_type(actions->leave, args));
        if (actions->enter)
        {
          if (detach)
          {
            typedef rObject(Executable::*fun_type)(objects_type);
            sched::rJob job =
              new runner::Interpreter
              (actions->lobby?actions->lobby:r.lobby_get(),
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
    }
  }
}
