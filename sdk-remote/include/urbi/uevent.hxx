/*
 * Copyright (C) 2010-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

// \file urbi/uevent.hxx
#ifndef URBI_UEVENT_HXX
# define URBI_UEVENT_HXX

# include <stdexcept>
# include <libport/cassert>
# include <urbi/uevent.hh>

namespace urbi
{
  inline
  UEvent::UEvent()
    : name("noname")
  {
  }


  inline
  void
  UEvent::__init()
  {
    ctx_->declare_event(this);
  }


  inline
  void
  UEvent::emit(UAutoValue v1,
               UAutoValue v2,
               UAutoValue v3,
               UAutoValue v4,
               UAutoValue v5,
               UAutoValue v6,
               UAutoValue v7
               )
  {
    ctx_->emit(name, v1, v2, v3, v4, v5, v6, v7);
  }
} // end namespace urbi

#endif // ! URBI_UEVENT_HXX
