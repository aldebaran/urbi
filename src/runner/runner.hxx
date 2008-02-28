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
  Runner::Runner (rLobby lobby, rObject locals,
		  scheduler::Scheduler& sched, ast::Ast* ast)
    : scheduler::Job (sched),
      Tag (),
      lobby_ (lobby),
      ast_ (ast),
      current_ (0),
      locals_ (locals),
      myself_ (this)
  {
    locals_->locals_set (true);
  }

  inline
  Runner::Runner(const Runner& model)
    : ast::DefaultVisitor (),
      scheduler::Job (model),
      Tag (),
      lobby_ (model.lobby_),
      ast_ (model.ast_),
      current_ (model.current_),
      locals_ (model.locals_),
      myself_ (this)
  {
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
  object::rLobby
  Runner::lobby_get ()
  {
    return lobby_;
  }

  inline
  const object::rObject&
  Runner::locals_get () const
  {
    return locals_;
  }

  inline
  const object::rObject&
  Runner::current_get () const
  {
    return current_;
  }

  inline
  scheduler::rJob
  Runner::myself_get () const
  {
    return myself_;
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
