/*
 * Copyright (C) 2009, 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef URBI_SDK_HH
# define URBI_SDK_HH

# include <libport/utime.hh>

# include <urbi/object/cxx-object.hh>
# include <urbi/object/slot.hh>
# include <urbi/object/cxx-conversions.hh>
# include <urbi/object/object.hh>

namespace urbi
{
  /*-------------.
  | Import types |
  `-------------*/

  using object::Object;
  using object::CxxObject;
  using object::rObject;
  using object::Slot;

  URBI_SDK_API void yield();
  URBI_SDK_API void yield_until(libport::utime_t t);
  URBI_SDK_API void yield_for(libport::utime_t t);
  URBI_SDK_API void yield_for_fd(int fd);

  /// Wait for data on \a fd, yielding until data is available.
  /// \return the available data, "" iff EOF.
  URBI_SDK_API std::string yield_for_read(int fd);

  // FIXME: function isn't ideal
  inline rObject global();
}

# include <urbi/sdk.hxx>

#endif
