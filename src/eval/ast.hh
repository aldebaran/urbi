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

# include <libport/compilation.hh>
# include <libport/finally.hh>

# include <ast/all.hh>
# include <ast/print.hh>

# include <runner/state.hh>
# include <runner/job.hh>

# include <eval/action.hh>

namespace eval
{

  namespace ast_impl
  {
# define VISIT(Macro, Data, Node)                       \
    rObject eval(Job& this_, const ast::Node* n);

    AST_FOR_EACH_NODE(VISIT);
#undef VISIT
  }

  rObject ast_context(Job& job, const ast::Ast* e, rObject self);

  rObject ast(Job& job, ast::rConstAst ast);
  Action  ast(ast::rConstAst ast);

} // namespace eval

# if defined LIBPORT_COMPILATION_MODE_SPEED
#  include <eval/ast.hxx>
# endif

#endif // ! EVAL_AST_HH
