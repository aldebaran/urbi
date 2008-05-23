/**
 ** \file runner/interpreter.hxx
 ** \brief Inline implementation of runner::Interpreter.
 */

#ifndef RUNNER_INTERPRETER_HXX
# define RUNNER_INTERPRETER_HXX

# include "object/atom.hh"
# include "object/global-class.hh"
# include "object/alien.hh"
# include "binder/binder.hh"

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
