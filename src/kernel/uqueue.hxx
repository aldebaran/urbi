/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef KERNEL_UQUEUE_HXX
# define KERNEL_UQUEUE_HXX

namespace kernel
{
  inline void
  UQueue::push(const std::string& s)
  {
    push(s.c_str(), s.size());
  }
}

#endif // !KERNEL_UQUEUE_HXX
