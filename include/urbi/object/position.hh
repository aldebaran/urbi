/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_POSITION_HH
# define OBJECT_POSITION_HH

# include <parser/position.hh>

# include <urbi/object/fwd.hh>

namespace urbi
{
  namespace object
  {

    /// Convertion a parser position \a p into urbi class Global.Position.
    rObject Position(const ::yy::position& p);

  } // namespace object
} // namespace urbi

#endif
