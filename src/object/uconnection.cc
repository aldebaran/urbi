/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

# include <object/symbols.hh>
# include <object/uconnection.hh>
# include <object/urbi-exception.hh>
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
# define DECLARE(a) bind(SYMBOL(a), &UConnection::a)
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
      libport::utime_t now = libport::utime();
      if (enabled && now - libport::seconds_to_utime(lastCall) >
          libport::seconds_to_utime(minInterval) && !processing)
      {
        if (asynchronous)
        {
          processing = true;
          sched::rJob job = new runner::Interpreter
          ( r.lobby_get(),
           r.scheduler_get(),
           boost::bind(&UConnection::doCall, this, (runner::Runner*)0, self, t),
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
      // If target is not inputport, write to uvar
      if (!target->slot_has(SYMBOL(inputPort)))
        target->as<UVar>()->update_(self->as<UVar>()->getter(true));
      else // Else bypass write and call notifies.
      callNotify(r?*r:kernel::urbiserver->getCurrentRunner(),
                   target->as<UVar>(), SYMBOL(change), self);
      libport::utime_t end = libport::utime();
      lastCall = (double)end / 1.0e6;
      double ct = (double)(end-now) / 1.0e6;
      minCallTime = (callCount? std::min(ct, minCallTime):ct);
      maxCallTime = std::max(ct, maxCallTime);
      callCount++;
      totalCallTime += ct;
      processing = false;
    }
  }
}

