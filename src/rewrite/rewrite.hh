/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef REWRITE_REWRITE_HH
# define REWRITE_REWRITE_HH

# include <ast/fwd.hh>
# include <urbi/export.hh>

namespace rewrite
{
  ast::rAst  URBI_SDK_API rewrite(ast::rConstAst a);
  ast::rExp  URBI_SDK_API rewrite(ast::rConstExp a);
  ast::rNary URBI_SDK_API rewrite(ast::rConstNary a);

  ast::rAst  URBI_SDK_API rewrite(ast::rAst a);
  ast::rExp  URBI_SDK_API rewrite(ast::rExp a);
  ast::rNary URBI_SDK_API rewrite(ast::rNary a);

  ast::rAst  URBI_SDK_API desugar(ast::rConstAst Ast);
  ast::rAst  URBI_SDK_API rescope(ast::rConstAst Ast);
}

#endif
