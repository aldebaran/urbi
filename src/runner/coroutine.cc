#include <boost/foreach.hpp>

# define ENABLE_DEBUG_TRACES
# include "libport/compiler.hh"

# include "runner/coroutine.hh"

#ifndef NDEBUG
extern "C"
{
  void coroutine_chk (void)
  {
    if (runner::Coroutine::coro_cnt)
      ECHO ("runner::Coroutine::coro_cnt = " << runner::Coroutine::coro_cnt);
    assert (runner::Coroutine::coro_cnt == 0);
  }
}

int runner::Coroutine::coro_cnt = 0;
#endif

namespace runner
{

  Coroutine::~Coroutine ()
  {
    if (!waited_by_.empty ())
    {
      ECHO ("warning: coroutine destroyed but " << waited_by_.size ()
            << " other coroutines still waiting.");
      // This is not a problem as long as coroutines signaled don't try to
      // use this coroutine which has been partially destroyed.
      cr_signal_finished_ ();
    }
    ECHO ("destroyed");
    assert (cr_stack_.empty ());
#ifndef NDEBUG
    --coro_cnt;
#endif
  }

  void
  Coroutine::cr_signal_finished_ ()
  {
    cr_finished_ = true;
    BOOST_FOREACH (Coroutine* coro, waited_by_)
    {
      assert (coro);
      assert (coro->waiting_for_);
      --coro->waiting_for_;
      coro->finished (*this);
    }
    wait_set empty;
    waited_by_.swap (empty);
  }

  void
  Coroutine::reset ()
  {
    cr_drop_stack_ ();
  }

  void
  Coroutine::wait_for (Coroutine& coro)
  {
    if (waited_by_.find (&coro) != waited_by_.end ())
    {
      ECHO ("wait cycle detected: "
            << &coro << " was already waiting for " << this
            << " when " << this << " attempted to wait for " << &coro);
      assert (!"wait cycle detected");
      abort ();
    }
    std::pair<wait_set::iterator, bool> res = coro.waited_by_.insert (this);
    if (res.second)
      ++waiting_for_;
  }

} // namespace runner
