/**
 ** \file runner/runner.hxx
 ** \brief Inline implementation of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HXX
# define RUNNER_RUNNER_HXX

# include "runner/runner.hh"

namespace runner
{

  inline
  Runner::Runner ()
  {}

  inline
  Runner::~Runner ()
  {}

  inline
  Runner::rObject
  Runner::eval (const ast::Ast& e)
  {
    e.accept (*this);
    return current_;
  }

  inline
  Runner::rObject
  Runner::result ()
  {
    return current_;
  }

} // namespace runner

# include "runner/runner.hxx"

#endif // !RUNNER_RUNNER_HXX
