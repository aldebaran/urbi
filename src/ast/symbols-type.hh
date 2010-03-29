/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef AST_SYMBOLS_TYPE_HH
# define AST_SYMBOLS_TYPE_HH

# include <iosfwd>
# include <vector>

# include <libport/fwd.hh>

namespace ast
{
  /// Function formal arguments.
  typedef std::vector<libport::Symbol> symbols_type;
}

// We need argument dependent name lookup for symbols_type,
// which is actually a class of std: it is std::vector<...>.
namespace std
{
  /// Separated by commas.
  std::ostream&
  operator<<(std::ostream& o, const ast::symbols_type& ss);
}

#endif // ! AST_SYMBOLS_TYPE_HH
