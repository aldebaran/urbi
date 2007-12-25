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
  Runner::Runner (rLobby lobby, Scheduler& sched, ast::Ast* ast)
    : Coroutine (sched),
      lobby_ (lobby),
      ast_ (ast),
      started_ (false),
      current_ (0),
      locals_ (new object::Object)
  {
    locals_->locals_set (true);
    // If the lookup in the local variable failed, try in the the
    // Connection object, sort of a Lobby for Io.
    locals_->parent_add(lobby_);
    // Provide direct access to the Lobby.
    locals_->slot_set(libport::Symbol("lobby"), lobby_);
  }

  inline
  Runner::~Runner ()
  {
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
