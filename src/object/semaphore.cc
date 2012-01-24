/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/kernel/userver.hh>
#include <object/code.hh>
#include <urbi/object/float.hh>
#include <object/semaphore.hh>
#include <urbi/object/symbols.hh>
#include <runner/runner.hh>
#include <sched/fwd.hh>
#include <libport/debug.hh>

GD_CATEGORY(Urbi.Semaphore);

namespace urbi
{
  namespace object
  {
    Semaphore::Semaphore()
    {
      proto_add(proto ? rObject(proto) : Object::proto);
    }

    Semaphore::Semaphore(rSemaphore model)
      : value_(model->value_)
    {
      proto_add(proto);
    }

    Semaphore::Semaphore(const value_type& value)
      : value_(value)
    {
      proto_add(proto);
    }

    URBI_CXX_OBJECT_INIT(Semaphore)
    {
#define DECLARE(Name, Cxx)                \
      bind(SYMBOL_(Name), &Semaphore::Cxx)

      DECLARE(new,             _new);
      DECLARE(criticalSection, criticalSection);
      DECLARE(acquire,         acquire);
      DECLARE(release,         release);

#undef DECLARE
    }

    rSemaphore
    Semaphore::_new(rObject, rFloat c)
    {
      return new Semaphore(make_pair(c->to_unsigned_type(),
                                     std::list<runner::rRunner>()));
    }

    void
    Semaphore::acquire()
    {
      runner::Runner& r = ::kernel::runner();

      GD_FINFO_TRACE("%p: Acquire", &r);
      if (--value_.first < 0)
      {
        std::list<runner::rRunner>::iterator i =
          value_.second.insert(value_.second.end(), &r);
        try
        {
          // Wait until the job is unfrozen by the release.
          r.frozen_set(true);
          GD_FINFO_TRACE("%p: Waiting", &r);
          r.yield();
          GD_FINFO_TRACE("%p: Waking-up", &r);
        }
        catch (...)
        {
          GD_FINFO_TRACE("%p: Caught Exception", &r);
          // If the current process had a release token and has caught an
          // exception too, then forward the release token.
          if (r.frozen_get())
          {
            release_and_forward(false);
            value_.second.erase(i);
            r.frozen_set(false);
          }
          else
            release_and_forward(true);
          throw;
        }
      }
    }

    void
    Semaphore::release()
    {
      release_and_forward(true);
    }

    void
    Semaphore::release_and_forward(bool forward)
    {
      runner::Runner& r = ::kernel::runner();
      GD_FINFO_TRACE("%p: Release", &r);

      if (++value_.first <= 0 && forward)
      {
        GD_FINFO_TRACE("%p: Wake-up %p", &r, value_.second.front().get());
        value_.second.front()->frozen_set(false);
        value_.second.pop_front();
      }
    }

    rObject
    Semaphore::criticalSection(rCode f)
    {
      objects_type args;
      args.push_back(this);

      acquire();
      libport::Finally finally(boost::bind(&Semaphore::release, this));
      return (*f)(args);
    }
  } // namespace object
}
