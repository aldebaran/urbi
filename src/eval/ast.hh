/*
 * Copyright (C) 2011-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file eval/ast.hh
 ** \brief Definition of eval::ast.
 */

#ifndef EVAL_AST_HH
# define EVAL_AST_HH

# include <ast/fwd.hh>
# include <eval/action.hh>
# include <urbi/runner/fwd.hh>

#ifdef SHELL_EXCEPTION_WORKAROUND
# define INLINE_AST_FUNCS
#else
# define INLINE_AST_FUNCS ATTRIBUTE_ALWAYS_INLINE
# endif
namespace eval
{
  INLINE_AST_FUNCS
  rObject ast_context(Job& job, const ast::Ast* e, rObject self);

  INLINE_AST_FUNCS
  rObject ast(Job& job, ast::rConstAst ast);

  INLINE_AST_FUNCS
  Action  ast(ast::rConstAst ast);

} // namespace eval
#ifndef SHELL_EXCEPTION_WORKAROUND
# include <eval/ast.hxx>
#endif

#endif // ! EVAL_AST_HH
