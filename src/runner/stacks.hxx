/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef RUNNER_STACKS_HXX
# define RUNNER_STACKS_HXX

namespace runner
{
  inline unsigned
  Stacks::local_pointer() const
  {
    return local_pointer_;
  }

  inline unsigned
  Stacks::captured_pointer() const
  {
    return captured_pointer_;
  }
}

#endif
