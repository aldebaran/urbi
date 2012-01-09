/*
 * Copyright (C) 2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_LOBBY_HXX
# define OBJECT_LOBBY_HXX

# include <urbi/object/symbols.hh>
# include <urbi/object/tag.hh>

namespace urbi
{
  namespace object
  {

    inline
    rTag
    Lobby::tag_get() const
    {
      return slot_get(SYMBOL(connectionTag))->as<object::Tag>();
    }

  } // namespace object
} // namespace urbi

#endif
