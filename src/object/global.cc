/*
 * Copyright (C) 2009, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */
/**
 ** \file object/global-class.cc
 ** \brief Creation of the URBI object global.
 */

#include <object/format-info.hh>

#include <object/global.hh>
#include <object/object.hh>

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
