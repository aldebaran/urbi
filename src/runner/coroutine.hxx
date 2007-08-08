/**
 ** \file runner/coroutine.hxx
 ** \brief Inline implementation of runner::Coroutine.
 */

#ifndef RUNNER_COROUTINE_HXX
# define RUNNER_COROUTINE_HXX

# include "runner/coroutine.hh"
# include <iostream>

#ifndef NDEBUG
extern "C" {
  void coroutine_chk (void);
}
#endif

namespace runner
{

  inline
  Coroutine::Coroutine ()
    : cr_stack_ (),
      cr_new_call_ (false),
      cr_resumed_ (0),
      waiting_for_ (0),
      waited_by_ ()
  {
#ifndef NDEBUG
    static bool first_time = true;
    if (first_time)
      atexit (coroutine_chk);
    ++coro_cnt;
#endif
  }

  inline
  Coroutine::Coroutine (const Coroutine&)
    : cr_stack_ (),
      cr_new_call_ (false),
      cr_resumed_ (0),
      waiting_for_ (0),
      waited_by_ ()
  {
#ifndef NDEBUG
    ++coro_cnt;
#endif
  }

  template <typename T>
  T*
  Coroutine::take_first_slot ()
  {
    assert (!cr_stack_.empty ());
    struct dummy_ctx: public CoroCtx
    {
      T* data;
    };
    return reinterpret_cast<dummy_ctx*> (cr_stack_.top ().second)->data;
  }

  inline
  bool
  Coroutine::started () const
  {
    return waiting_for_ || !cr_stack_.empty ();
  }

  inline
  unsigned
  Coroutine::context_number () const
  {
    return cr_stack_.size ();
  }

  inline
  void Coroutine::finished (Coroutine&)
  {
  }

  inline
  void
  Coroutine::cr_save_ (line l, CoroCtx* ctx)
  {
    cr_stack_.push (std::make_pair (l, ctx));
  }

  inline
  Coroutine::CoroCtx*
  Coroutine::cr_restore_ ()
  {
    CoroCtx* ctx = cr_stack_.top ().second;
    cr_stack_.pop ();
    return ctx;
  }

  inline
  void
  Coroutine::cr_drop_stack_ ()
  {
    stack_type empty;
    std::swap (cr_stack_, empty);
    cr_resumed_ = 0;
  }

  inline
  Coroutine::line
  Coroutine::cr_line_ () const
  {
    return cr_stack_.top ().first;
  }

} // namespace runner

#endif // !RUNNER_COROUTINE_HXX
