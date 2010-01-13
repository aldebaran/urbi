/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/object/global.hh>
#include <urbi/object/location.hh>
#include <urbi/object/position.hh>

namespace urbi
{
  namespace object
  {

    rObject Location(const ::ast::loc& l)
    {
      CAPTURE_GLOBAL(Location);

      rObject loc =
        Location
        ->call("new",
               new object::Position(l.begin),
               new object::Position(l.end));
      return loc;
    }

  } // namespace object
} // namespace urbi
