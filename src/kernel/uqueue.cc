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
#include <parser/prescan.hh>

namespace kernel
{
  UQueue::UQueue(super_type::size_type chunk_size)
    : super_type(chunk_size)
    , preparse_hint(0)
  {
  }

  std::string
  UQueue::pop_command()
  {
    // The buffer is null-terminated when used in full, so we can use it
    // without computing its length.
    LIBPORT_DEBUG("Size: " << size());
    if (size() < preparse_hint)
      return std::string();
    LIBPORT_DEBUG("buf: {{{" << std::string(peek()) << "}}}");
    long len = parser::prescan(peek(), size());
    LIBPORT_DEBUG("Len: " << len);
    if (len <= 0)
    {
      preparse_hint = len * -1;
      // Avoid multiple buffer resize.
      reserve(preparse_hint);
      return std::string();
    }
    preparse_hint = 0;
    const std::string res(pop(len), len);
    LIBPORT_DEBUG("Res: " << res);
    return res;
  }
}
