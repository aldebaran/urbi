/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/eval/action.hh
 ** \brief Definition of eval::ast.
 */

#ifndef EVAL_AST_HH
# define EVAL_AST_HH

# include <ast/fwd.hh>
# include <eval/action.hh>
# include <runner/fwd.hh>

namespace eval
{
  rObject ast_context(Job& job, const ast::Ast* e, rObject self);

  rObject ast(Job& job, ast::rConstAst ast);
  Action  ast(ast::rConstAst ast);

} // namespace eval

# include <eval/ast.hxx>

#endif // ! EVAL_AST_HH
