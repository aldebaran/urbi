/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/interpreter.hxx
 ** \brief Inline implementation of runner::Interpreter.
 */

#ifndef RUNNER_INTERPRETER_HXX
# define RUNNER_INTERPRETER_HXX

#include <libport/finally.hh>

#include <ast/ast.hh>
#include <urbi/object/tag.hh>
#include <urbi/object/symbols.hh>

namespace runner
{

  inline
  Interpreter::rObject
  Interpreter::result_get()
  {
    return result_;
  }

  inline
  libport::Symbol
  Interpreter::innermost_call_get() const
  {
    if (call_stack_.empty())
      return SYMBOL(LT_empty_GT);
    else
      return call_stack_.back().first;
  }

  inline
  const ast::Ast*
  Interpreter::innermost_node() const
  {
    return innermost_node_;
  }

  /*----------------.
  | Regular visit.  |
  `----------------*/

#define FINALLY_Ast(DefineOrUse)                                        \
  FINALLY_ ## DefineOrUse(Ast,                                          \
                          ((const ast::Ast*&, innermost_node_))         \
                          ((const ast::Ast*, previous)),                \
                          innermost_node_ = previous);

  FINALLY_Ast(DEFINE);
  inline object::rObject
  Interpreter::operator() (const ast::Ast* e)
  {
    const ast::Ast* previous = innermost_node_;
    FINALLY_Ast(USE);
    innermost_node_ = e;
    return e->eval(*this);
  }
# undef FINALLY_AST_ARGS

} // namespace runner

#endif // RUNNER_INTERPRETER_HXX
