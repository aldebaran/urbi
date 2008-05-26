/**
 ** \file runner/runner.hxx
 ** \brief Inline implementation of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HXX
# define RUNNER_RUNNER_HXX

# include "object/atom.hh"

namespace runner
{

  inline
  Runner::Runner(rLobby lobby, scheduler::Scheduler& sched,
		 const libport::Symbol& name)
    : scheduler::Job(sched, name),
      lobby_(lobby)
  {
  }

  inline
  Runner::Runner(const Runner& model, const libport::Symbol& name)
    : scheduler::Job(model, name),
      lobby_(model.lobby_)
  {
  }

  inline
  Runner::~Runner()
  {
  }

  inline const object::rLobby&
  Runner::lobby_get() const
  {
    return lobby_;
  }

  inline object::rLobby
  Runner::lobby_get()
  {
    return lobby_;
  }

} // namespace runner

#endif // RUNNER_RUNNER_HXX
