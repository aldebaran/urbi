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
  Interpreter::rObject
  Interpreter::eval(ast::rConstAst e)
  {
    ECHO("Eval: " << &e << " {{{" << e << "}}}");
    operator()(e);
    ECHO("Eval: " << &e << " = " << result_);
    return result_;
  }

} // namespace runner

#endif // RUNNER_INTERPRETER_HXX
