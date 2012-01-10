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
 ** \file eval/ast.hxx
 ** \brief Definition of eval::ast.
 */

#ifndef EVAL_AST_HXX
# define EVAL_AST_HXX

# include <libport/bind.hh>
# include <libport/finally.hh>

# include <runner/state.hh>
# include <runner/job.hh>

namespace eval
{

#define FINALLY_Context(DefineOrUse)                    \
    FINALLY_ ## DefineOrUse                             \
    (Context,                                           \
      ((Job&, job))                                     \
      ((const runner::State::var_context_type&, ctx)),  \
     job.state.pop_context(ctx);                        \
    )

  FINALLY_Context(DEFINE);

  rObject ast_context(Job& job, const ast::Ast* e, rObject self)
  {
    typedef runner::State::var_context_type var_context_type;
    var_context_type ctx = job.state.push_context(self);
    FINALLY_Context(USE);
    return ast(job, e);
  }


#define FINALLY_Ast(DefineOrUse)                                        \
  FINALLY_ ## DefineOrUse(Ast,                                          \
                          ((Job&, job))                                 \
                          ((const ast::Ast*, previous)),                \
                          job.state.innermost_node_set(previous));

  FINALLY_Ast(DEFINE);

  // !!! GD_* macros are commented because this consume stack space in speed
  // mode, even if messages are not printed.

  rObject ast(Job& job, ast::rConstAst n)
  {
    // GD_CATEGORY(Urbi.Eval.Ast);

    // GD_FPUSH_TRACE("Ast %s (%s)",
    //                n->node_type(),
    //                string_cast(n->location_get()));
    const ast::Ast* previous = job.state.innermost_node_get();
    FINALLY_Ast(USE);
    job.state.innermost_node_set(n.get());

    // let the AST node bounce on the ast_impl::eval functions
    return n->eval(job);
  }

  Action ast(ast::rConstAst n)
  {
    return boost::bind(&ast, _1, n);
  }

} // namespace eval


#endif // ! EVAL_AST_HXX
