/**
 ** \file runner/interpreter.hxx
 ** \brief Inline implementation of runner::Interpreter.
 */

#ifndef RUNNER_INTERPRETER_HXX
# define RUNNER_INTERPRETER_HXX

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
    assert(!call_stack_.empty());
    return call_stack_.back().first;
  }

  /*----------------.
  | Regular visit.  |
  `----------------*/

  inline object::rObject
  Interpreter::operator() (const ast::Ast* e)
  {
    libport::Finally finally(scoped_set(innermost_node_, e));
    return e->eval(*this);
  }

} // namespace runner

#endif // RUNNER_INTERPRETER_HXX
