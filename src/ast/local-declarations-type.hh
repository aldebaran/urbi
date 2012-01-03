/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef AST_DECLARATIONS_TYPE_HH
# define AST_DECLARATIONS_TYPE_HH

# include <iosfwd>
# include <vector>

# include <ast/local-declaration.hh>

namespace ast
{

  /// List of declarations.
  typedef std::vector<rLocalDeclaration> local_declarations_type;

  /// Separated by commas.
  std::ostream&
  operator<<(std::ostream& o, const ast::local_declarations_type& ss);

}

#endif // ! AST_DECLARATIONS_TYPE_HH
