/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file object/global.cc
 ** \brief Creation of the Urbi object Global.
 */

#include <libport/finally.hh>

#include <object/format-info.hh>

#include <urbi/object/global.hh>
#include <urbi/object/object.hh>

namespace urbi
{
  namespace object
  {
    rObject global_class;

    /*--------------------.
    | Global primitives.  |
    `--------------------*/

    void
    global_class_initialize()
    {}

    rObject
    capture(libport::Symbol name, const rObject& from)
    {
      GD_CATEGORY(Urbi);
      static bool capturing = false;
      if (capturing)
        GD_FABORT("CAPTURE_ from CAPTURE_: %s.", name);

      LIBPORT_SCOPE_SET(capturing, true);
      return from->slot_get(name);
    }
  }; // namespace object
}
