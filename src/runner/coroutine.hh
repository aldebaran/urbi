/**
 ** \file runner/coroutine.hh
 ** \brief Definition of runner::Coroutine.
 */

#ifndef RUNNER_COROUTINE_HH
# define RUNNER_COROUTINE_HH

# include <cassert>

# include <stack>
# include <set>
# include <utility>

# define ENABLE_DEBUG_TRACES
# include "libport/compiler.hh"

# include "runner/fwd.hh"

namespace runner
{

  class Coroutine
  {
  protected: /// \{ Constructors.
    Coroutine ();
    Coroutine (const Coroutine&);
    /// \}

  private: /// Disabled.
    Coroutine& operator= (const Coroutine&);

  public:
    /// Destructor
    virtual ~Coroutine ();

    /** Fetch the first slot of the context saved by this coroutine.
     * Undefined behavior if this coroutine did not have a slot or if you
     * pass the wrong @a T.  */
    template <typename T>
    T*
    take_first_slot ();

    /// Whether or not this coroutine started doing anything.
    bool started () const;

    /// Number of contexts stacked in this coroutine.
    unsigned context_number () const;

    /** Delete all the contexts stacked in this coroutine.
     *
     * @warning May leak memory if a raw pointer was stored in a context
     * and this pointer was the last reference to a memory zone allocated
     * with @c new or @c malloc.  */
    void reset ();

    /** Register that this coroutine must not be re-scheduled until @a coro
     * has completely finished its execution.  Once @a coro has finished,
     * this coroutine will have its method @c finished invoked with @a coro
     * in argument.
     *
     * @note Cycles are detected and lead to an @c abort (there is a cycle
     * when a coroutine @c A tries to @c wait_for @c B and @c B was already
     * waiting for @c A).  */
    void wait_for (Coroutine& coro);

  protected:
    /** Signal that @a coro has finished.  This signal is delivered for
     * coroutines waited for with @c wait_for.  You can override this
     * method.  */
    virtual void finished (Coroutine& coro);

    /***********************************************************
     * End of public interface.  Everything below is internal. *
     ***********************************************************/

  protected:
    struct CoroCtx {};

  private:
    typedef int line;
    typedef std::stack<std::pair<line, CoroCtx*> > stack_type;

  protected:
    void cr_save_ (line l, CoroCtx* ctx);
    CoroCtx* cr_restore_ ();
    void cr_drop_stack_ ();
    void cr_signal_finished_ ();

  protected:
    /// Stack of contexts.
    stack_type cr_stack_;
    /// Are we issuing a new a call?
    bool cr_new_call_;
    /// How many calls have we resumed?
    unsigned cr_resumed_;

    /// Number of coroutines this one is waiting for.
    unsigned waiting_for_;

  private:
    typedef std::set<Coroutine*> wait_set;
    /// Coroutines waiting for this one.
    wait_set waited_by_;

#ifndef NDEBUG
  public:
    static int coro_cnt;
#endif
  };

} // namespace runner

# define CORO_CTX_START                         \
  struct __coro_ctx: public Coroutine::CoroCtx  \
  {

# define CORO_CTX_ADD(Decl)                     \
    Decl

# define CORO_START()                                                   \
  };                                                                    \
  if (waiting_for_)                                                     \
  {                                                                     \
    assert (!cr_stack_.empty ());                                       \
    ECHO ("coroutine not ready to resume execution at line "            \
          << cr_stack_.top ().first << " (still waiting for "           \
          << waiting_for_ << " other coroutines)");                      \
    throw CoroutineYield ();                                            \
  }                                                                     \
  __coro_ctx* __ctx = 0;                                                \
  try                                                                   \
  {                                                                     \
    switch (cr_new_call_ || !started () ? 0 : cr_stack_.top ().first)   \
    {                                                                   \
    case 0:                                                             \
      cr_new_call_ =  false;                                            \
      __ctx = new __coro_ctx ();                                        \
      ECHO ("creating a new coroutine (ctx: " << __ctx << ')')

# define CORO_INIT_WITH_1SLOT_CTX(Decl)         \
  CORO_CTX_START;                               \
  CORO_CTX_ADD (Decl);                          \
  CORO_START ()

# define CORO_INIT_WITH_2SLOTS_CTX(Decl1, Decl2)        \
  CORO_CTX_START;                                       \
  CORO_CTX_ADD (Decl1);                                 \
  CORO_CTX_ADD (Decl2);                                 \
  CORO_START ()

# define CORO_INIT_WITH_3SLOTS_CTX(Decl1, Decl2, Decl3) \
  CORO_CTX_START;                                       \
  CORO_CTX_ADD (Decl1);                                 \
  CORO_CTX_ADD (Decl2);                                 \
  CORO_CTX_ADD (Decl3);                                 \
  CORO_START ()

# define CORO_INIT_WITHOUT_CTX()                \
  CORO_CTX_START;                               \
  CORO_START ()

# define CORO_CTX(What) __ctx->What

# define CORO_SAVE_BEGIN_(Coro)                                         \
    cr_save_ (__LINE__, __ctx);                                         \
    ECHO ("coroutine saved (ctx: " << __ctx << ") now "                 \
          << context_number () << " contexts in the coroutine stack")

# define CORO_SAVE_END_                                                 \
      case __LINE__:                                                    \
        __ctx = static_cast<__coro_ctx*> (cr_restore_ ());              \
        ECHO ("coroutine resumed (ctx: " << __ctx << ") now "           \
              << context_number () << " contexts in the coroutine stack")

# define CORO_YIELD_(Ret)                                               \
      do                                                                \
      {                                                                 \
        CORO_SAVE_BEGIN_ (this);                                        \
        throw Ret; /* No parens (see below) */                          \
        CORO_SAVE_END_;                                                 \
      } while (0)

/* No parentheses around throw Ret: gcc <= 3.3 would choke */

# define CORO_YIELD_VALUE(Val) CORO_YIELD_ (Val) // FIXME: Wrap Val in a known exn
# define CORO_YIELD() CORO_YIELD_ (CoroutineYield ())

# define CORO_JOIN()                            \
  do {                                          \
    if (waiting_for_)                           \
      CORO_YIELD();                             \
  } while (0)

# define CORO_CALL_(Coro, What, OnYield)                \
    cr_new_call_ = true;                                \
    ++cr_resumed_;                                      \
    if (false)                                          \
    {                                                   \
      CORO_SAVE_END_;                                   \
      ++cr_resumed_;                                    \
    }                                                   \
    try {                                               \
      What;                                             \
    }                                                   \
    catch (const CoroutineYield&)                       \
    {                                                   \
      OnYield                                           \
    }                                                   \
    ECHO ("back to coroutine ctx: " << __ctx            \
          << " with " << context_number ()              \
          << " contexts in the coroutine stack");       \
    assert (cr_resumed_);                               \
    --cr_resumed_;                                      \
    cr_new_call_ = false

# define CORO_CALL(What)                        \
  do {                                          \
    CORO_CALL_ (this, What,                     \
                CORO_SAVE_BEGIN_ (Coro);        \
                assert (cr_resumed_);           \
                --cr_resumed_;                  \
                throw;);                        \
  } while (0)

# define CORO_CALL_IN_BACKGROUND(Coro, What)    \
  do {                                          \
    Coro->cr_drop_stack_ ();                    \
    CORO_CALL_ (Coro, What, /* nothing */);     \
  } while (0)

# define CORO_CLEANUP_                                                  \
  if (!context_number () && !cr_resumed_)                               \
  {                                                                     \
    ECHO ("cleaning up");                                               \
    cr_signal_finished_ ();                                             \
    delete this;                                                        \
  }                                                                     \
  else                                                                  \
  {                                                                     \
    ECHO ("not cleaning up: " << cr_resumed_ << " calls to be resumed"); \
  }

# define CORO_RET_(Ret)                                                 \
  do {                                                                  \
    ECHO ("coroutine ret (ctx: " << __ctx << ") with "                  \
          << context_number ()-1 << " contexts in the coroutine stack"); \
    if (false)                                                          \
    {                                                                   \
      CORO_SAVE_END_;                                                   \
    }                                                                   \
    if (waiting_for_)                                                   \
    {                                                                   \
      ECHO ("cannot return at this time, still waiting for "            \
            << waiting_for_ << " other coroutines");                    \
      CORO_SAVE_BEGIN_ (this);                                          \
      throw CoroutineYield ();                                          \
    }                                                                   \
    CORO_CLEANUP_;                                                      \
    delete __ctx;                                                       \
    return Ret;                                                         \
  } while (0)

# define CORO_RETURN CORO_RET_ (;)
# define CORO_RETURN_VALUE(Val) CORO_RET_ (Val)

# define CORO_END_(Ret)                                                 \
      break;                                                            \
    default:                                                            \
      ECHO ("coroutine invalid resume (invalid line: "                  \
            << (!started () ? 0 : cr_stack_.top ().first)               \
            << ", ctx: " << __ctx << ')');                              \
      abort ();                                                         \
      CORO_SAVE_END_;                                                   \
    } /* end of switch */                                               \
  } /* end of try */                                                    \
  catch (const CoroutineYield&)                                         \
  {                                                                     \
    throw;                                                              \
  }                                                                     \
  catch (...)                                                           \
  {                                                                     \
    ECHO ("coroutine exn (ctx: " << __ctx << ") "                       \
          << context_number () << " contexts left in the coroutine stack"); \
    CORO_CLEANUP_;                                                      \
    delete __ctx;                                                       \
    throw;                                                              \
  }                                                                     \
  ECHO ("coroutine end (ctx: " << __ctx << ") "                         \
        << context_number () << " contexts left in the coroutine stack"); \
  if (waiting_for_)                                                     \
  {                                                                     \
    ECHO ("cannot return at this time, still waiting for "              \
          << waiting_for_ << " other coroutines");                      \
    CORO_SAVE_BEGIN_ (this);                                            \
    throw CoroutineYield ();                                            \
  }                                                                     \
  CORO_CLEANUP_;                                                        \
  delete __ctx;                                                         \
  return Ret

# define CORO_END  CORO_END_ (;)
# define CORO_END_VALUE(Val)  CORO_END_ (Val)

# include "runner/coroutine.hxx"

#endif // !RUNNER_COROUTINE_HH
