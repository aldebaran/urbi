/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <kernel/userver.hh>

#include <urbi/object/cxx-object.hh>
#include <urbi/object/global.hh>
#include <object/symbols.hh>

#include <runner/runner.hh>

GD_CATEGORY(Urbi.CxxObject);

namespace urbi
{
  namespace object
  {
    CxxObject::CxxObject()
    {}

    void
    type_check(const rObject& o, const rObject& exp,
               boost::optional<unsigned> idx)
    {
      aver(o);
      aver(exp);
      if (!is_a(o, exp))
      {
        if (idx)
          runner::raise_argument_type_error(idx.get(), o, exp);
        else
          runner::raise_type_error(o, exp);
      }
    }

  }
}
