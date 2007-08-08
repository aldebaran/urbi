/**
 ** \file runner/coroutine.hxx
 ** \brief Inline implementation of runner::Coroutine.
 */

#ifndef RUNNER_COROUTINE_HXX
# define RUNNER_COROUTINE_HXX

# include "runner/coroutine.hh"

namespace runner
{

  inline
  Coroutine::Coroutine ()
    : cr_stack_ (),
      cr_call_ (false)
  {
  }

  inline
  Coroutine::~Coroutine ()
  {
    assert (cr_stack_.empty ());
  }

  template <typename T>
  T*
  Coroutine::take_first_slot ()
  {
    assert (!cr_stack_.empty ());
    struct dummy_ctx
    {
      T* data;
    };
    return reinterpret_cast<dummy_ctx*> (cr_stack_.top ().second)->data;
  }

  inline
  bool
  Coroutine::started () const
  {
    return !cr_stack_.empty ();
  }

  inline
  unsigned
  Coroutine::context_number () const
  {
    return cr_stack_.size ();
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
  }

} // namespace runner

#endif // !RUNNER_COROUTINE_HXX
