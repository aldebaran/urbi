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
  Runner::Runner (rLobby lobby, rObject locals, Scheduler& sched, ast::Ast* ast)
    : Coroutine (sched),
      lobby_ (lobby),
      ast_ (ast),
      started_ (false),
      current_ (0),
      locals_ (locals)
  {
    locals_->locals_set (true);
  }

  inline
  Runner::~Runner ()
  {
  }

  inline
  const object::rLobby&
  Runner::lobby_get () const
  {
    return lobby_;
  }

  inline
  Runner::rObject
  Runner::eval (ast::Ast& e)
  {
    ECHO("Eval: " << &e << " {{{" << e << "}}}");
    e.accept (*this);
    ECHO("Eval: " << &e << " = " << current_);
    return current_;
  }

  inline
  Runner::rObject
  Runner::target (ast::Exp* n)
  {
    // If there is no target, look in the local variables.
    if (n)
      return eval (*n);
    else
      return locals_;
  }

} // namespace runner

#endif // !RUNNER_RUNNER_HXX
