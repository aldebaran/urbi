/**
 ** \file runner/runner.hxx
 ** \brief Inline implementation of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HXX
# define RUNNER_RUNNER_HXX

# include "runner/runner.hh"
# include "object/primitives.hh" //connection_class

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

  inline
  Runner::rObject
  Runner::target (ast::Exp* n)
  {
    // FIXME: For the time being, if there is no target, it is the
    // Connection object which is used, sort of a Lobby for IO.
    return n ? eval (*n) : object::connection_class;
  }

} // namespace runner

# include "runner/runner.hxx"

#endif // !RUNNER_RUNNER_HXX
