/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

# include <urbi/object/event.hh>
# include <object/symbols.hh>
# include <object/uconnection.hh>
# include <object/urbi-exception.hh>
# include <object/uvalue.hh>
# include <urbi/object/slot.hh>

# include <runner/job.hh>

# include <urbi/object/lobby.hh>

GD_CATEGORY(Urbi.UConnection);
namespace urbi
{
  namespace object
  {
    UConnection::UConnection()
    {
      proto_add(new UConnection);
      init_();
    }

    UConnection::UConnection(libport::intrusive_ptr<UConnection> model)
    {
      proto_add(model);
      init_();
    }

    URBI_CXX_OBJECT_INIT(UConnection)
    {
      BIND(asynchronous);
      BIND(callCount);
      BIND(enabled);
      BIND(lastCall);
      BIND(maxCallTime);
      BIND(minCallTime);
      BIND(minInterval);
      BIND(source);
      BIND(target);
      BIND(totalCallTime);
      init_();
    }

    void UConnection::init_()
    {
      enabled = true;
      minInterval = 0;
      lastCall = (double)libport::utime() / 1.0e6;
      totalCallTime = 0;
      callCount = 0;
      maxCallTime = 0;
      minCallTime = 0;
      asynchronous = false;
      processing = false;
    }

    bool UConnection::call(runner::Job& r, rObject self)
    {
      // FIXME: stay connected in the hope of reconnecting one day?
      rSlot t = target->as<Slot>();
      if (!t || t == Slot::proto)
        return false;
      if (t->call(SYMBOL(dead)) == true_class)
      {
        //FIXME: try to keep informations for possible future reconnection
        GD_INFO_TRACE("Target is dead");
        target = Slot::proto;
        return false;
      }
      GD_FINFO_DUMP("%s -> %s call, processing: %s enabled: %s",
                    source, target, processing, enabled);
      if (enabled
          && !processing
          && (libport::utime() - libport::seconds_to_utime(lastCall) >
              libport::seconds_to_utime(minInterval)))
      {
        if (asynchronous)
        {
          processing = true;
          // build an Action out of a member function
          sched::rJob job =
            r.spawn_child(
              boost::bind(&UConnection::doCall, this, _1, self, t),
              sched::configuration.minimum_stack_size)
            ->name_set("doCall");
          job->start_job();
        }
        else
          doCall(r, self, t);
      }
      return true;
    }

    rObject UConnection::doCall(runner::Job&,
                                rObject self,
                                rObject target)
    {
      processing = true;
      libport::utime_t now = libport::utime();
      // FIXME: fake sender
      rObject val = self->as<Slot>()->value(nil_class, true);
      // Do not call uobject_set here as it would reset Slot name
      // causing the update and break remote loopback detection.
      target->as<Slot>()->set(val, nil_class, libport::utime());
      libport::utime_t end = libport::utime();
      lastCall = (double)end / 1.0e6;
      double ct = (double)(end-now) / 1.0e6;
      minCallTime = callCount ? std::min(ct, minCallTime) : ct;
      maxCallTime = std::max(ct, maxCallTime);
      callCount++;
      totalCallTime += ct;
      processing = false;
      return object::void_class;
    }
  }
}
