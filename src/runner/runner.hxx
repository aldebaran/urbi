/**
 ** \file runner/runner.hxx
 ** \brief Inline implementation of runner::Runner.
 */

#ifndef RUNNER_RUNNER_HXX
# define RUNNER_RUNNER_HXX

namespace runner
{

  inline
  Runner::Runner(rLobby lobby, scheduler::Scheduler& sched)
    : scheduler::Job(sched),
      lobby_(lobby)
  {
  }

  inline
  Runner::Runner(const Runner& model)
    : scheduler::Job(model),
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

  inline const Runner::call_stack_type&
  Runner::call_stack_get() const
  {
    return call_stack_;
  }

} // namespace runner

#endif // RUNNER_RUNNER_HXX
