/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <runner/shell.hh>

namespace kernel
{
  inline
  runner::Shell&
  shell()
  {
    return static_cast<runner::Shell&>(runner());
  }

} // namespace kernel
