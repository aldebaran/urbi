/**
 ** \file runner/coroutine-yield.hh
 ** \brief Definition of runner::CoroutineYield.
 */

#ifndef RUNNER_COROUTINE_YIELD_HH
# define RUNNER_COROUTINE_YIELD_HH

# include "runner/fwd.hh"

namespace runner
{

  class CoroutineException
  {
  public:
    explicit CoroutineException (const Coroutine& coro)
      : coro_ (coro)
    {
    }

    const Coroutine& coro () const
    {
      return coro_;
    }

    Coroutine& coro ()
    {
      return const_cast<Coroutine&> (coro_);
    }

  protected:
    const Coroutine& coro_;
  };

  struct CoroutineYield: public CoroutineException
  {
    explicit CoroutineYield (const Coroutine& coro)
      : CoroutineException (coro)
    {
    }
  };

  struct CoroutineAbort: public CoroutineException
  {
    explicit CoroutineAbort (const Coroutine& coro)
      : CoroutineException (coro)
    {
    }
  };

} // namespace runner

#endif // !RUNNER_COROUTINE_YIELD_HH
