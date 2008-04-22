/**
 ** \file runner/runner.hxx
 ** \brief Inline implementation of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HXX
# define RUNNER_RUNNER_HXX

# include "object/atom.hh"
# include "object/global-class.hh"
# include "object/alien.hh"

# include "runner/runner.hh"

namespace runner
{

  inline
  Runner::Runner (rLobby lobby, rObject locals,
		  scheduler::Scheduler& sched, const ast::Ast* ast)
    : Runner::super_type (),
      scheduler::Job (sched),
      lobby_ (lobby),
      ast_ (ast),
      current_ (0),
      locals_ (locals)
  {
    if (!locals_)
      locals_ = object::Object::make_do_scope(object::global_class, lobby);
    // If the lobby has a slot connectionTag, push it
    rObject connection_tag = lobby_->slot_locate(SYMBOL(connectionTag));
    if (connection_tag)
      push_tag(extract_tag(connection_tag->slot_get(SYMBOL(connectionTag))));
  }

  inline
  Runner::Runner(const Runner& model, const ast::Ast* ast)
    : Runner::super_type (),
      scheduler::Job (model),
      lobby_ (model.lobby_),
      ast_ (ast),
      current_ (0),
      locals_ (model.locals_)
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
  Runner::rObject
  Runner::eval (const ast::Ast& e)
  {
    ECHO("Eval: " << &e << " {{{" << e << "}}}");
    e.accept (*this);
    ECHO("Eval: " << &e << " = " << current_);
    return current_;
  }

} // namespace runner

#endif // !RUNNER_RUNNER_HXX
