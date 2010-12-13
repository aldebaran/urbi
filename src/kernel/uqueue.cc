/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/// \file uqueue.cc
/// \brief Implementation of UQueue.

//#define ENABLE_DEBUG_TRACES
#include <libport/echo.hh>

#include <libport/cassert>
#include <libport/cstring>

#include <kernel/uqueue.hh>

namespace kernel
{
  UQueue::UQueue(super_type::size_type chunk_size)
    : super_type(chunk_size)
  {
  }
}
