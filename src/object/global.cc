/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
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
    {
    }

  }; // namespace object
}
