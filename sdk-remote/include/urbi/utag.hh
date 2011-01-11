/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_UTAG_HH
# define URBI_UTAG_HH

/// A prefix for tags with which internal messages are exchanged.
/// Callbacks with tag_wildcard do not see these messages.  A macro so
/// that we can easily concatenate strings at compile time.
# define TAG_PRIVATE_PREFIX "__gostai_private__"

// FIXME: The need to make these tags public is not clear.
namespace urbi
{
  extern const char* connectionTimeoutTag;
  extern const char* internalPongTag;

} // namespace urbi

#endif // URBI_UTAG_HH
