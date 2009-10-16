/*
 * Copyright (C) 2009, Gostai S.A.S.
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

# include <object/cxx-conversions.hh>
# include <object/cxx-object.hh>
# include <object/object.hh>
# include <object/slot.hh>

namespace urbi
{
  /*-------------.
  | Import types |
  `-------------*/

  using object::Object;
  using object::CxxObject;
  using object::rObject;
  using object::Slot;

  void yield();
  void yield_until(libport::utime_t t);
  void yield_for(libport::utime_t t);
  void yield_for_fd(int fd);

  /// Wait for data on \a fd, yielding until data is available.
  /// \return the available data, "" iff EOF.
  std::string yield_for_read(int fd);

  // FIXME: function isn't ideal
  inline
  rObject global();
}

# include <urbi/sdk.hxx>

#endif
