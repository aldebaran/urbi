/*
 * Copyright (C) 2007-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file ast/flavor.hh
 ** \brief Declaration of ast::flavor_type.
 */

#ifndef AST_FLAVOR_HH
# define AST_FLAVOR_HH

# include <ostream>

namespace ast
{
  /// Whether a command is to be run in background.
  enum flavor_type
  {
    flavor_none,
    flavor_and,
    flavor_comma,
    flavor_pipe,
    flavor_semicolon,
  };

  /// Report \a e on \a o.
  std::ostream& operator<<(std::ostream& o, flavor_type e);

} // namespace ast

# include <ast/flavor.hxx>

#endif // !AST_FLAVOR_HH
