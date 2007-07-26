/**
 ** \file runner/runner.hxx
 ** \brief Inline implementation of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HXX
# define RUNNER_RUNNER_HXX

# include "object/atom.hh"

# include "runner/runner.hh"

namespace runner
{

  inline
  Runner::Runner (rContext ctx)
    : context_ (ctx),
      current_ (0)
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
    if (n)
      return eval (*n);
    else
      return context_;
  }

} // namespace runner

#endif // !RUNNER_RUNNER_HXX
