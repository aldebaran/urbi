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
    ECHO ("destroyed");
    assert (cr_stack_.empty ());
#ifndef NDEBUG
    --coro_cnt;
#endif
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
