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
 ** \file runner/eval/ast.hxx
 ** \brief Definition of eval::ast.
 */

#ifndef EVAL_AST_HXX
# define EVAL_AST_HXX

# include <boost/scoped_ptr.hpp>

# include <libport/bind.hh>
# include <libport/compilation.hh>
# include <libport/finally.hh>
# include <libport/foreach.hh>
# include <libport/format.hh>

# include <sched/exception.hh>

# include <kernel/uconnection.hh>
# include <kernel/userver.hh>

# include <ast/all.hh>

# include <object/symbols.hh>
# include <object/system.hh>

# include <urbi/object/code.hh>
# include <urbi/object/event.hh>
# include <urbi/object/event-handler.hh>
# include <urbi/object/global.hh>
# include <urbi/object/list.hh>
# include <urbi/object/dictionary.hh>
# include <urbi/object/object.hh>
# include <urbi/object/tag.hh>
# include <urbi/object/slot.hh>
# include <urbi/object/string.hh>

# include <urbi/runner/raise.hh>

# include <runner/state.hh>
# include <runner/job.hh>

# include <eval/ast.hh>
# include <eval/call.hh>
# include <eval/raise.hh>
# include <eval/send-message.hh>


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

  LIBPORT_SPEED_INLINE
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

  LIBPORT_SPEED_INLINE
  rObject ast(Job& job, ast::rConstAst n)
  {
    // GD_CATEGORY(Eval.Ast);

    // GD_FPUSH_TRACE("Ast %s (%s)",
    //                n->node_type(),
    //                string_cast(n->location_get()));
    const ast::Ast* previous = job.state.innermost_node_get();
    FINALLY_Ast(USE);
    job.state.innermost_node_set(n.get());

    // let the AST node bounce on the ast_impl::eval functions
    return n->eval(job);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  Action ast(ast::rConstAst n)
  {
    return boost::bind(&ast, _1, n);
  }

} // namespace eval


#endif // ! EVAL_AST_HXX
