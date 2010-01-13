/*
 * Copyright (C) 2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef OBJECT_LOCATION_HH
# define OBJECT_LOCATION_HH

# include <ast/loc.hh>

# include <urbi/object/cxx-object.hh>

namespace urbi
{
  namespace object
  {

    /// Convertion an AST location \a l into urbi class Global.Location.
    rObject Location(const ::ast::loc& l);

  } // namespace object
} // namespace urbi

#endif
