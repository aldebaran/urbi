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

  /*----------------.
  | Regular visit.  |
  `----------------*/

  inline object::rObject
  Interpreter::operator() (const ast::Ast* e)
  {
    /// Catch exceptions, set the location and backtrace if not
    /// already done, and rethrow it.
    try
    {
      return e->eval(*this);
    }
    catch (object::Exception& x)
    {
      propagate_error_(x, e->location_get());
      throw;
    }
  }

} // namespace runner

#endif // RUNNER_INTERPRETER_HXX
