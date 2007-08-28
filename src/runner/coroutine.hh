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

# include <boost/preprocessor/array.hpp>
# include <boost/preprocessor/tuple.hpp>
# include <boost/preprocessor/repeat.hpp>

# define ENABLE_DEBUG_TRACES
# include "libport/compiler.hh"

# include "runner/fwd.hh"
# include "runner/job.hh"
# include "runner/coroutine-yield.hh"

namespace runner
{

  class Coroutine: public Job
  {
  protected: /// \{ Constructors.
    explicit Coroutine (Scheduler& scheduler);
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
    unsigned context_count () const;

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
    /** @internal
     * Interface for coroutine contexts.  Each coroutine declares a local
     * @c struct where their variables can be stored.  This local @c struct
     * inherits this interface.  */
    struct CoroCtx {};

    /// Used to store values of @c __LINE__.
    typedef int line;

  private:
    /// Type to hold a call stack.
    typedef std::stack<std::pair<line, CoroCtx*> > stack_type;

  protected:
    /** @internal
     * Save the context @a ctx at line @a l.  @a l is the value of @c
     * __LINE__ at the call site.  */
    void cr_save_ (line l, CoroCtx* ctx);

    /** @internal
     * Pop and return a context out of the context stack.
     * @return the topmost context of the context stack.  */
    CoroCtx* cr_restore_ ();

    /** @internal
     * Drop all the contexts of the internal context stack of the coroutine.
     * Useful when a coroutine has been cloned and the execution is
     * considered to start from the current point.
     */
    void cr_drop_stack_ ();

    /** @internal
     * When this coroutine is finished, signal it to other coroutines
     * waiting for this one by calling their @c finished method with @c this
     * as argument.  */
    void cr_signal_finished_ ();

    /** @internal
     * @return the line where execution stopped in the topmost stack-frame.
     */
    line cr_line_ () const;

    /** @internal Number of coroutines this one is waiting for.  */
    unsigned cr_waiting_for_ () const;

    /** @internal Number of coroutines waiting for this one.  */
    unsigned cr_waited_by_ () const;

  protected:
    /// Stack of contexts.  Associates lines to contexts.
    stack_type cr_stack_;
    /// Are we issuing a new a call?
    bool cr_new_call_;
    /// Has this coroutine finished its execution?
    bool cr_finished_;
    /// How many calls have we resumed?
    unsigned cr_resumed_;

  private:
    /// Number of coroutines this one is waiting for.
    unsigned waiting_for_;
    typedef std::set<Coroutine*> wait_set;
    /// Coroutines waiting for this one.
    wait_set waited_by_;

#ifndef NDEBUG
  public:
    /// Track the number of coroutines alive.
    static int coro_cnt;
#endif
  };

} // namespace runner

/*********************
 * Coroutine macros. *
 *********************/

/*
 * Macros ending with an underscore (`_') are considered internal
 * implementation details and should not be used.
 *
 * The `public' macros have some restrictions:
 *   - Do NOT call more than one CORO_* macro per line.
 *   - The following macros must NOT be invoked from within a
 *     switch-case statement:
 *       CORO_YIELD, CORO_YIELD_VALUE, CORO_CALL, CORO_CALL_IN_BACKGROUND
 */

/// @internal Helper of @c CORO_CTX_VARS_ used to declare a variable
# define CORO_DECL_(Z, N, Array)                                \
  BOOST_PP_TUPLE_ELEM(2, 0, BOOST_PP_ARRAY_ELEM(N, Array))	\
  BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_ARRAY_ELEM(N, Array));

/** @internal Helper of @c CORO_CTX_VARS_ used to initialize a reference to
 * a member of the context.  */
# define CORO_USE_(Z, N, Array)						\
  BOOST_PP_TUPLE_ELEM(2, 0, BOOST_PP_ARRAY_ELEM(N, Array))&		\
  BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_ARRAY_ELEM(N, Array))		\
  = ctx__->BOOST_PP_TUPLE_ELEM(2, 1, BOOST_PP_ARRAY_ELEM(N, Array));

/** @internal Define a context.
 * @param Vars a Boost CPP Array of tuples (type, name).
 * @param NewCtx is an expression (rvalue) to initialize the @c ctx__
 * pointer.  */
# define CORO_CTX_VARS_(Vars, NewCtx)                                   \
  struct coro_ctx__: public Coroutine::CoroCtx                          \
  {                                                                     \
    BOOST_PP_REPEAT(                                                    \
      BOOST_PP_ARRAY_SIZE(Vars),                                        \
      CORO_DECL_,                                                       \
      Vars                                                              \
    )                                                                   \
  };                                                                    \
  line cr_line__ = cr_new_call_ || !started () ? 0 : cr_line_ ();        \
  CORO_CHECK_WAITING_ ("coroutine not ready to resume execution at line " \
		       << cr_line__ << " (still waiting for "           \
		       << cr_waiting_for_ () << " other coroutines)");  \
  coro_ctx__* ctx__ = cr_line__ == 0                                    \
		    ? NewCtx                                            \
		    : static_cast<coro_ctx__*> (cr_restore_ ());        \
  /* Inject the values of the context in the current scope */           \
  BOOST_PP_REPEAT(                                                      \
    BOOST_PP_ARRAY_SIZE(Vars),                                          \
    CORO_USE_,                                                          \
    Vars)                                                               \
  try                                                                   \
  {                                                                     \
    switch (cr_line__)                                                  \
    {                                                                   \
    case 0:                                                             \
      cr_new_call_ =  false;                                            \
      ECHO ("creating a new coroutine (ctx: " << ctx__ << ')')

/** Initialize the context of a coroutine.
 * @param Vars a Boost CPP Array of tuples (type, name).
 * Example: @code
 * CORO_CTX_VARS ((1, ((int, i))));
 * @endcode
 * Example: @code
 * CORO_CTX_VARS ((2, ((int, i), (float, f))));
 * @endcode
 * Example: @code
 * CORO_CTX_VARS ((3, (
 *   (MyObject, obj),
 *   (int*, p),
 *   (float, f),
 * )));
 * @endcode
 */
# define CORO_CTX_VARS(Vars) CORO_CTX_VARS_ (Vars, new coro_ctx__ ())

/// Initialize a coroutine without a context.
# define CORO_WITHOUT_CTX() CORO_CTX_VARS_ ((0, ()), 0)

/** @internal Suspend the execution if this coroutine is waiting for other
 * coroutines to finish.
 * @param Msg a debugging message.
 * @param Code something to execute just before yielding.  */
# define CORO_CHK_WAITING_(Msg, Code)                           \
  if (cr_waiting_for_ ())                                       \
  {                                                             \
    assert (!cr_stack_.empty ());                               \
    ECHO (Msg);                                                 \
    Code                                                        \
    scheduler_get ().add_job (this);                            \
    throw CoroutineYield (*this);                               \
  }

/** @internal Suspend the execution if this coroutine is waiting for other
 * coroutines to finish.  */
# define CORO_CHECK_WAITING_(Msg)               \
  CORO_CHK_WAITING_ (Msg, /*nothing*/)

/** @internal Suspend the execution if this coroutine is waiting for other
 * coroutines to finish.  If the execution is suspended, @c CORO_SAVE_BEGIN_
 * is invoked before yielding.  */
# define CORO_CHECK_WAITING_AND_SAVE_()                                 \
  CORO_CHK_WAITING_ ("cannot return at this time, still waiting for "   \
		     << cr_waiting_for_ () << " other coroutines",      \
		     CORO_SAVE_BEGIN_;)

/** Shorthand to initialize a context with a single variable.  */
# define CORO_WITH_1SLOT_CTX(Type, Name)        \
  CORO_CTX_VARS ((1, ((Type, Name))))

/** Shorthand to initialize a context with two variables.  */
# define CORO_WITH_2SLOTS_CTX(Type1, Name1, Type2, Name2)       \
  CORO_CTX_VARS ((2, ((Type1, Name1), (Type2, Name2))))

/** Shorthand to initialize a context with 3 variables.  */
# define CORO_WITH_3SLOTS_CTX(Type1, Name1, Type2, Name2, Type3, Name3) \
  CORO_CTX_VARS ((3, (                                                  \
    (Type1, Name1),                                                     \
    (Type2, Name2),                                                     \
    (Type3, Name3),                                                     \
  )))

/** @internal
 * Start to save the context.  Must be invoked on the same line as
 * @c CORO_SAVE_END_.  The current line and context are pushed on
 * the context stack. */
# define CORO_SAVE_BEGIN_                                               \
    cr_save_ (__LINE__, ctx__);                                         \
    ECHO ("coroutine saved (ctx: " << ctx__ << ") now "                 \
	  << context_count () << " contexts in the coroutine stack")

/** @internal
 * Finish to save the context.  Must be invoked on the same line as
 * @c CORO_SAVE_BEGIN_.  Add a @c case to resume the execution at
 * @c __LINE__ and fetch the context when execution is resumed at
 * @c __LINE__.
 * @warning Must not be invoked form within a @c switch case.
 * @warning Must not be immediately reachable.  Invoke this macro after
 * a @c throw or an @c abort.  If you can't, place it in a
 * @code if (false) { ... } @endcode.
 */
# define CORO_SAVE_END_                                                 \
      case __LINE__:                                                    \
	ECHO ("coroutine resumed (ctx: " << ctx__ << ") now "           \
	      << context_count () << " contexts in the coroutine stack")

/// @internal Yield the value @a Ret.
# define CORO_YIELD_(Ret)                                               \
      do                                                                \
      {                                                                 \
	CORO_SAVE_BEGIN_;						\
	scheduler_get ().add_job (this);                                \
	/* Not "throw Ret()": gcc <= 3.3 chokes. */			\
	throw Ret;							\
	CORO_SAVE_END_;                                                 \
      } while (0)


/// Yield and return an intermediate value @c Val.
# define CORO_YIELD_VALUE(Val) CORO_YIELD_ (Val) // FIXME: Wrap Val in a known exn

/// Yield without returning an intermediate value.
# define CORO_YIELD() CORO_YIELD_ (CoroutineYield (*this))

/// Suspend the execution of the coroutine until all the coroutines it's
/// waiting for have finished their execution.
# define CORO_JOIN()                            \
  do {                                          \
    if (cr_waiting_for_ ())                     \
      CORO_YIELD();                             \
  } while (0)

/** @internal
 * Call another coroutine with the C++ statement @a What and execute the C++
 * statement @a OnYield if the statement @a What did a Yield.
 * The C++ statement @a Before is executed before invoking @a What.
 */
# define CORO_CALL_(Before, What, OnYield)              \
    cr_new_call_ = true;                                \
    ++cr_resumed_;                                      \
    Before                                              \
    try {                                               \
      What;                                             \
    }                                                   \
    catch (const CoroutineYield&)                       \
    {                                                   \
      OnYield                                           \
    }                                                   \
    ECHO ("back to coroutine ctx: " << ctx__            \
	  << " with " << context_count ()		\
	  << " contexts in the coroutine stack");       \
    assert (cr_resumed_);                               \
    --cr_resumed_;                                      \
    cr_new_call_ = false

/**
 * Call another coroutine with the C++ statement @a What.  You must use this
 * if you call another coroutine or if you invoke yourself recursively.  It
 * does not harm if @a What doesn't actually call a coroutine.  If the
 * coroutine invoked by @a What yields (or does something similar), the
 * execution of this coroutine is suspended.  Later when this coroutine runs
 * again, the execution of @a What will be resumed where it stopped.  All
 * side effects and operations done by @a What will be repeated each time
 * the execution of @a What is resumed.  Thus you should avoid to compute
 * anything in @a What as well as doing side effects.  When the execution of
 * @a What is resumed, only values stored in the context are valid.
 * Example: @code CORO_CALL (member_ = other_coro ()); @endcode
 * Example: @code CORO_CALL (other_coro ()); @endcode
 */
# define CORO_CALL(What)                        \
  do {                                          \
    CORO_CALL_ (if (false)                      \
		{                               \
		  CORO_SAVE_END_;               \
		  ++cr_resumed_;                \
		},                              \
		What,                           \
		CORO_SAVE_BEGIN_;               \
		assert (cr_resumed_);           \
		--cr_resumed_;                  \
		throw;);                        \
  } while (0)

/** Same thing as @c CORO_CALL but you need to specify the target
 * @c Coroutine pointer in @a Coro and if @a What yields.
 */
# define CORO_CALL_IN_BACKGROUND(Coro, What)            \
  do {                                                  \
    Coro->cr_drop_stack_ ();                            \
    CORO_CALL_ (/* nothing */, What, /* nothing */);    \
  } while (0)

/// @internal Maybe perform some cleanup.  Checks if this coroutine finished
/// its execution and if it did, signal other coroutines waiting for this
/// one and automatically @c delete self.
# define CORO_CLEANUP_                                  \
  if (cr_finished_)                                     \
  {                                                     \
    ECHO ("destroying myself");                         \
    cr_signal_finished_ ();                             \
    delete this;                                        \
  }                                                     \
  else if (!context_count () && !cr_resumed_)          \
  {                                                     \
    ECHO ("finished: signaling " << cr_waited_by_ ()    \
	  << " other coroutines");                      \
    cr_signal_finished_ ();                             \
    scheduler_get ().add_job (this);                    \
    CORO_SAVE_BEGIN_;                                   \
    throw CoroutineYield (*this);                       \
  }

/// @internal Return the value @a Ret and terminates this coroutine.
# define CORO_RET_(Ret)                                                 \
  do {                                                                  \
    ECHO ("coroutine ret (ctx: " << ctx__ << ") with "                  \
	  << context_count ()-1 << " contexts in the coroutine stack"); \
    if (false)                                                          \
    {                                                                   \
      CORO_SAVE_END_;                                                   \
    }                                                                   \
    CORO_CHECK_WAITING_AND_SAVE_ ();                                    \
    CORO_CLEANUP_;                                                      \
    delete ctx__;                                                       \
    return Ret;                                                         \
  } while (0)

/// Terminate this coroutine without returning a value.
# define CORO_RETURN CORO_RET_ (;)
/// Terminate this coroutine and return the value @c Val.
/// @param Val is guaranteed to execute only once.
# define CORO_RETURN_VALUE(Val) CORO_RET_ (Val)

/// @internal Epilogue of the coroutine.  Terminate the coroutine with the
/// return value @a Ret.
# define CORO_END_(Ret)                                                 \
      break;                                                            \
    default:                                                            \
      ECHO ("coroutine invalid resume (invalid line: "                  \
	    << cr_line__ << ", ctx: " << ctx__ << ')');                 \
      abort ();                                                         \
      CORO_SAVE_END_; /* Save for the waiting_for case below.  */       \
    } /* end of switch */                                               \
  } /* end of try */                                                    \
  catch (const CoroutineYield&)                                         \
  {                                                                     \
    throw;                                                              \
  }                                                                     \
  catch (...)                                                           \
  {                                                                     \
    ECHO ("coroutine exn (ctx: " << ctx__ << ") "                       \
	  << context_count () << " contexts left in the coroutine stack"); \
    try {                                                               \
      CORO_CLEANUP_;                                                    \
    }                                                                   \
    catch (...)                                                         \
    {                                                                   \
    }                                                                   \
    delete ctx__;                                                       \
    throw;                                                              \
  }                                                                     \
  ECHO ("coroutine end (ctx: " << ctx__ << ") "                         \
	<< context_count () << " contexts left in the coroutine stack"); \
  CORO_CHECK_WAITING_AND_SAVE_ ();                                      \
  CORO_CLEANUP_;                                                        \
  delete ctx__;                                                         \
  return Ret

/// Epilogue of a coroutine that returns @c void.
# define CORO_END  CORO_END_ (;)
/// Epilogue of a coroutine that returns @a Val.
# define CORO_END_VALUE(Val)  CORO_END_ (Val)

# include "runner/coroutine.hxx"

#endif // !RUNNER_COROUTINE_HH
