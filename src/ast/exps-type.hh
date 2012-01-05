/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef AST_EXPS_TYPE_HH
# define AST_EXPS_TYPE_HH

# include <iosfwd>
# include <vector>

# include <ast/fwd.hh>

namespace ast
{

  /// List of expressions, for List, Nary, Call etc.
  typedef std::vector<rExp> exps_type;

  /// Separated by end of lines.
  std::ostream& operator<<(std::ostream& o, const ast::exps_type& ss);

}

#endif // ! AST_EXPS_TYPE_HH
