/**
 ** \file runner/interpreter.hxx
 ** \brief Inline implementation of runner::Interpreter.
 */

#ifndef RUNNER_INTERPRETER_HXX
# define RUNNER_INTERPRETER_HXX

namespace runner
{

  inline
  const object::rObject&
  Interpreter::locals_get () const
  {
    return locals_;
  }

  inline
  Interpreter::rObject
  Interpreter::eval (const ast::Ast& e)
  {
    ECHO("Eval: " << &e << " {{{" << e << "}}}");
    operator()(e);
    ECHO("Eval: " << &e << " = " << current_);
    return current_;
  }

} // namespace runner

#endif // RUNNER_INTERPRETER_HXX
