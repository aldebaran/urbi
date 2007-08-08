/**
 ** \file runner/coroutine.hh
 ** \brief Definition of runner::Coroutine.
 */

#ifndef RUNNER_COROUTINE_HH
# define RUNNER_COROUTINE_HH

# include <cassert>

# include <stack>
# include <utility>

# define ENABLE_DEBUG_TRACES
# include "libport/compiler.hh"

# include "runner/fwd.hh"

namespace runner
{

  class Coroutine
  {
  protected:
    Coroutine ();
    ~Coroutine ();

    template <typename T>
    T*
    take_first_slot ();

    bool started () const;

    unsigned context_number () const;

  private:
    typedef int line;
    typedef void* opaque_ctx_type;
    typedef std::stack<std::pair<line, opaque_ctx_type> > stack_type;

  protected:
    stack_type cr_stack_;
    bool cr_call_;
  };

} // namespace runner

# define CORO_CTX_START                         \
  struct __coro_ctx                             \
  {

# define CORO_CTX_ADD(Decl)                     \
    Decl

# define CORO_START()                                                   \
  };                                                                    \
  __coro_ctx* __ctx = 0;                                                \
  try                                                                   \
  {                                                                     \
    switch (cr_call_ || !started () ? 0 : cr_stack_.top ().first)       \
    {                                                                   \
    case 0:                                                             \
      cr_call_ =  false;                                                \
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

# define CORO_SAVE_BEGIN_                                               \
  cr_stack_.push (std::make_pair (__LINE__,                             \
                                  reinterpret_cast<void*> (__ctx)));    \
    ECHO ("coroutine saved (ctx: " << __ctx << ") now "                 \
          << context_number () << " contexts in the coroutine stack")

# define CORO_SAVE_END_                                                 \
      case __LINE__:                                                    \
        __ctx = reinterpret_cast<__coro_ctx*> (cr_stack_.top ().second); \
        cr_stack_.pop ();                                               \
        ECHO ("coroutine resumed (ctx: " << __ctx << ") now "           \
              << context_number () << " contexts in the coroutine stack")


# define CORO_YIELD_(Ret)                                               \
      do                                                                \
      {                                                                 \
        CORO_SAVE_BEGIN_;                                               \
        throw Ret; /* No parens (see below) */                          \
        CORO_SAVE_END_;                                                 \
      } while (0)

/* No parentheses around throw Ret: gcc <= 3.3 would choke */

# define CORO_YIELD_VALUE(Val) CORO_YIELD_ (Val) // FIXME: Wrap Val in a known exn
# define CORO_YIELD() CORO_YIELD_ (CoroutineYield ())

# define CORO_CALL(What)                        \
  do {                                          \
    cr_call_ = true;                            \
    if (false)                                  \
    {                                           \
      CORO_SAVE_END_;                           \
    }                                           \
    try {                                       \
      What;                                     \
    }                                           \
    catch (const CoroutineYield&)               \
    {                                           \
      CORO_SAVE_BEGIN_;                         \
      throw;                                    \
    }                                           \
    ECHO ("back to coroutine ctx: " << __ctx    \
          << " with " << context_number ()          \
          << " contexts in the coroutine stack");   \
    cr_call_ = false;                           \
  } while (0)

# define CORO_CALL_IN_BACKGROUND(What)          \
  do {                                          \
    try {                                       \
      cr_call_ = true;                          \
      What;                                     \
      cr_call_ = false;                         \
    }                                           \
    catch (const CoroutineYield&)               \
    {                                           \
    }                                           \
  } while (0)

# define CORO_RET_(Ret)                                         \
  do {                                                          \
    ECHO ("coroutine ret (ctx: " << __ctx << ") now "           \
          << context_number ()-1 << " contexts in the coroutine stack"); \
    delete __ctx;                                               \
    cr_stack_.pop ();                                           \
    return Ret;                                                 \
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
    delete __ctx;                                                       \
    throw;                                                              \
  }                                                                     \
  ECHO ("coroutine end (ctx: " << __ctx << ") "                         \
        << context_number () << " contexts left in the coroutine stack"); \
  delete __ctx;                                                         \
  return Ret

# define CORO_END  CORO_END_ (;)
# define CORO_END_VALUE(Val)  CORO_END_ (Val)

# include "runner/coroutine.hxx"

#endif // !RUNNER_COROUTINE_HH
