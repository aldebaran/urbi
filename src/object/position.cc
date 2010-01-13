/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <urbi/object/file.hh>
#include <urbi/object/global.hh>
#include <urbi/object/path.hh>
#include <urbi/object/position.hh>
#include <urbi/object/string.hh>

namespace urbi
{
  namespace object
  {

    rObject Position(const ::yy::position& p)
    {
      CAPTURE_GLOBAL(Position);

      rObject path = object::nil_class;
      if (p.filename)
        path = new object::String(*p.filename);
      rObject pos =
        Position
        ->call("new",
               path,
               new object::Float(p.line),
               new object::Float(p.column));
      return pos;
    }

  } // namespace object
} // namespace urbi
