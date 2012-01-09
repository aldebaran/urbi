/*
 * Copyright (C) 2010-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

# include <urbi/object/event.hh>
# include <urbi/object/symbols.hh>
# include <object/uconnection.hh>
# include <urbi/object/urbi-exception.hh>
# include <object/uvalue.hh>
# include <object/uvar.hh>

# include <runner/interpreter.hh>

# include <urbi/object/lobby.hh>

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
# define DECLARE(Name)                          \
      bind(SYMBOL_(Name), &UConnection::Name)
      DECLARE(source);
      DECLARE(target);
      DECLARE(enabled);
      DECLARE(minInterval);
      DECLARE(lastCall);
      DECLARE(callCount);
      DECLARE(totalCallTime);
      DECLARE(minCallTime);
      DECLARE(maxCallTime);
      DECLARE(asynchronous);
      init_();
# undef DECLARE
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

    bool UConnection::call(runner::Runner& r, rObject self)
    {
      rObject t = UVar::fromName(target);
      if (!t)
        return false;
      if (enabled
          && !processing
          && (libport::utime() - libport::seconds_to_utime(lastCall) >
              libport::seconds_to_utime(minInterval)))
      {
        if (asynchronous)
        {
          processing = true;
          sched::rJob job = new runner::Interpreter
            (r.lobby_get(),
             r.scheduler_get(),
             boost::bind(&UConnection::doCall,
                         this, (runner::Runner*)0, self, t),
             rUConnection(this),
             SYMBOL(doCall));
          job->start_job();
        }
        else
          doCall(&r, self, t);
      }
      return true;
    }

    void UConnection::doCall(runner::Runner* r, rObject self, rObject target)
    {
      processing = true;
      libport::utime_t now = libport::utime();
      if (target->slot_has(SYMBOL(inputPort)))
      {
        // If target is InputPut, bypass write and call notifies.
        callNotify(r ? *r : ::kernel::runner(),
                   target->as<UVar>(), target->as<UVar>()->change_, self);
        rList l =  target->call(SYMBOL(changeConnections))->as<List>();
        if (l)
          callConnections(r ? *r : ::kernel::runner(), self, l);
      }
      else
        // If target is not, write to uvar.
        target->as<UVar>()->update_(self->as<UVar>()->getter(true));
      libport::utime_t end = libport::utime();
      lastCall = (double)end / 1.0e6;
      double ct = (double)(end-now) / 1.0e6;
      minCallTime = callCount ? std::min(ct, minCallTime) : ct;
      maxCallTime = std::max(ct, maxCallTime);
      callCount++;
      totalCallTime += ct;
      processing = false;
    }
  }
}
