/**
 ** \file runner/coroutine-yield.hh
 ** \brief Definition of runner::CoroutineYield.
 */

#ifndef RUNNER_COROUTINE_YIELD_HH
# define RUNNER_COROUTINE_YIELD_HH

# include "runner/fwd.hh"

namespace runner
{

  class CoroutineYield
  {
  public:
    explicit CoroutineYield (Coroutine& coro)
      : coro_ (coro)
    {
    }

    const Coroutine& coro () const
    {
      return coro_;
    }

    Coroutine& coro ()
    {
      return coro_;
    }

  protected:
    Coroutine& coro_;
  };

} // namespace runner

#endif // !RUNNER_COROUTINE_YIELD_HH
