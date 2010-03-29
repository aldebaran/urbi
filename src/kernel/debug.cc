/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <iostream>
#include <libport/indent.hh>

#include <kernel/debug.hh>

namespace
{

  static int init_cerr_indent ()
  {
    std::cerr << libport::resetindent;
    return 42;
  }
  static int init = init_cerr_indent();

}
