/*
 * Copyright (C) 2009-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef AST_DOT_PRINT_HH
# define AST_DOT_PRINT_HH

# include <ostream>

# include <ast/fwd.hh>
# include <urbi/export.hh>

namespace ast
{
  /// Dump an AST as a Dot graph.
  /// \param ast     the AST to dump.
  /// \param stream  where to dump.
  /// \param title   the name of the graph.
  void URBI_SDK_API
  dot_print(rConstAst ast, std::ostream& stream, const std::string& title = "");
}

#endif
