/**
 ** \file runner/runner.hxx
 ** \brief Inline implementation of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HXX
# define RUNNER_RUNNER_HXX

# include <libport/compilation.hh>

# include <object/lobby.hh>
# include <object/task.hh>

namespace runner
{

  LIBPORT_SPEED_INLINE
  Runner::Runner(rLobby lobby, scheduler::Scheduler& sched,
		 const libport::Symbol& name)
    : scheduler::Job(sched, name)
    , lobby_(lobby)
  {
  }

  LIBPORT_SPEED_INLINE
  Runner::Runner(const Runner& model, const libport::Symbol& name)
    : scheduler::Job(model, name)
    , lobby_(model.lobby_)
  {
  }

  LIBPORT_SPEED_INLINE
  Runner::~Runner()
  {
  }

  LIBPORT_SPEED_INLINE
  const object::rLobby&
  Runner::lobby_get() const
  {
    return lobby_;
  }

  LIBPORT_SPEED_INLINE
  object::rLobby
  Runner::lobby_get()
  {
    return lobby_;
  }

  LIBPORT_SPEED_INLINE
  void
  Runner::lobby_set(rLobby lobby)
  {
    lobby_ = lobby;
  }

} // namespace runner

#endif // RUNNER_RUNNER_HXX
