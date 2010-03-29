/*
 * Copyright (C) 2009-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef AST_SERIALIZE_HH
# define AST_SERIALIZE_HH

# include <ast/ast.hh>
# include <urbi/export.hh>

namespace ast
{
  void URBI_SDK_API
  serialize(rConstAst ast, std::ostream& output);

  rAst URBI_SDK_API
  unserialize(std::istream& output);
}

#endif
