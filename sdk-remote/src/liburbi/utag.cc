/*
 * Copyright (C) 2009-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/utag.hh>

namespace urbi
{
  const char* connectionTimeoutTag()
  {
    return TAG_PRIVATE_PREFIX "connection_timeout";
  }
  const char* internalPongTag()
  {
    return TAG_PRIVATE_PREFIX "internal_pong";
  }
} // namespace urbi
