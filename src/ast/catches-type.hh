/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef AST_CATCHES_TYPE_HH
# define AST_CATCHES_TYPE_HH

# include <iosfwd>
# include <vector>

# include <ast/catch.hh>
# include <ast/fwd.hh>

namespace ast
{
  /// List of catches.
  typedef std::vector<rCatch> catches_type;

  std::ostream& operator<<(std::ostream& o, const catches_type& c);
}

#endif // ! AST_CATCHES_TYPE_HH
