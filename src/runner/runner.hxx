/**
 ** \file runner/runner.hxx
 ** \brief Inline implementation of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HXX
# define RUNNER_RUNNER_HXX

# include <object/lobby.hh>

namespace runner
{

  inline
  Runner::Runner(rLobby lobby, scheduler::Scheduler& sched,
		 const libport::Symbol& name)
    : scheduler::Job(sched, name)
    , lobby_(lobby)
    , fork_point_()
  {
  }

  inline
  Runner::Runner(const Runner& model, const libport::Symbol& name)
    : scheduler::Job(model, name)
    , lobby_(model.lobby_)
    , fork_point_()
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

  inline void
  Runner::lobby_set(rLobby lobby)
  {
    lobby_ = lobby;
  }


  inline scheduler::rTag
  Runner::fork_point_get() const
  {
    return fork_point_;
  }

  inline void
  Runner::fork_point_set(scheduler::rTag fork)
  {
    assert(fork);
    fork_point_ = fork;
  }

} // namespace runner

#endif // RUNNER_RUNNER_HXX
