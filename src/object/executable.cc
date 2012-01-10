/*
 * Copyright (C) 2009-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/cassert>

#include <runner/interpreter.hh>
#include <object/code.hh>
#include <urbi/object/executable.hh>

namespace urbi
{
  namespace object
  {
    rObject Executable::operator()(object::objects_type)
    {
      pabort("Should never be here");
    }

    Executable::Executable()
    {
      proto_add(Object::proto);
    }

    Executable::Executable(rExecutable model)
    {
      proto_add(model);
    }

    // Use an intermediary bouncer to make sure the Executable is
    // stored in a smart pointer, and not deleted too early.
    static void
    executable_bouncer(rExecutable e, objects_type args)
    {
      (*e)(args);
    }

    runner::Interpreter*
    Executable::make_job(rLobby lobby,
                         sched::Scheduler& sched,
                         const objects_type& args,
                         libport::Symbol name)
    {
      return new runner::Interpreter
        (lobby, sched,
         boost::bind(&executable_bouncer, this, args),
         this, name);
    }

    URBI_CXX_OBJECT_INIT(Executable)
    {}
  }
}
