/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */
//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include <boost/foreach.hpp>

#include "kernel/uconnection.hh"
#include "object/atom.hh"
#include "runner/runner.hh"

namespace runner
{

/// Address of \a Runner seen as a \c Job (Runner has multiple inheritance).
#define JOB(Runner) static_cast<Job*> (Runner)

/// Address of \c this seen as a \c Job (Runner has multiple inheritance).
#define ME JOB (this)

/** Call this macro at the very beginning of the evaluation of an AST
 * node.  Nodes that should yield are that doing something useful.  For
 * instance in "1;2" we have 3 nodes: two values in a SemicolonExp.  The
 * SemicolonExp does not do anything useful, it simply recurses in its
 * left hand side.  However, the "1" does something useful: it calculates
 * a value (even though calculating the constant "1" is somewhat trivial),
 * so it should most probably start with a YIELD. */
#define YIELD(Ast)                                              \
  do                                                            \
  {                                                             \
    ECHO ("job " << ME << " yielding on AST: "                  \
	  << &Ast << " {{{" << Ast << "}}}");                   \
    CORO_YIELD ();                                              \
  } while (0)

  void
  Runner::work ()
  {
    assert (ast_);

    if (!started_)
    {
      ECHO ("job " << ME << " starting evaluation of AST: " << ast_
	    << " {{{" << *ast_ << "}}}");
      started_ = true;
    }
    else
      ECHO ("job " << ME << " restarting evaluation of AST: " << ast_
	    << " {{{" << *ast_ << "}}} "
	    << context_count () << " contexts in the coroutine stack");
    operator() (*ast_);
  }

  void
  Runner::stop ()
  {
    Coroutine::reset ();
  }

  void
  Runner::finished (Coroutine& coro)
  {
    ECHO (ME << " join with " << JOB (static_cast<Runner*> (&coro)));
    Runner& r = dynamic_cast<Runner&> (coro);
    emit_result (r.current_);
  }

  void
  Runner::emit_result (rObject result)
  {
    if (result.get ())
      context_->value_get ().connection.new_result (result);
  }

  void
  Runner::operator() (ast::AssignExp& e)
  {
    CORO_WITH_1SLOT_CTX (rObject, tgt);
    YIELD (e);

    PING ();
    assert (e.lhs_get ().args_get ().size () == 1);
    CORO_CALL (tgt = target (e.lhs_get ().args_get ().front ()));
    CORO_CALL (eval (e.rhs_get ()));
    {
      libport::Symbol s = e.lhs_get ().name_get ();
      tgt->slot_set (s, current_);
    }

    CORO_END;
  }

  void
  Runner::operator() (ast::AndExp& e)
  {
    CORO_WITHOUT_CTX ();

    {
      ECHO ("job " << ME << ", lhs: {{{" << e.lhs_get () << "}}}");
      Runner* lhs = new Runner (*this);
      lhs->ast_ = &e.lhs_get ();
      CORO_CALL_IN_BACKGROUND (lhs, lhs->eval (e.lhs_get ()));

      PING ();
      ECHO ("job " << ME << ", rhs: {{{" << e.rhs_get () << "}}}");
      Runner* rhs = new Runner (*this);
      rhs->ast_ = &e.rhs_get ();
      CORO_CALL_IN_BACKGROUND (rhs, rhs->eval (e.rhs_get ()));

      wait_for (*lhs);
      wait_for (*rhs);
    }
    CORO_JOIN ();
    PING ();

    CORO_END;
  }

  void
  Runner::operator() (ast::CallExp& e)
  {
    CORO_CTX_VARS ((6, (
      (bool, call_code),
      (rObject, val),
      // Iterate over arguments, with a special case for the target.
      (ast::exps_type::const_iterator, i),
      (ast::exps_type::const_iterator, i_end),
      // Gather the arguments, including the target.
      (object::objects_type, args),
      (rObject, tgt)
    )));

    PING ();
    i = e.args_get ().begin ();
    i_end = e.args_get ().end ();
    CORO_CALL (tgt = target(*i));

    // Ask the target for the handler of the message.
    val = tgt->lookup (e.name_get ());

    args.push_back (tgt);
    PING ();
    for (++i; i != i_end; ++i)
    {
      CORO_CALL (eval (**i));
      PING ();
      args.push_back (current_);
    }

    // We may have to run a primitive, or some code.
    call_code = false;
    switch (val->kind_get ())
    {
      case object::Object::kind_primitive:
	PING ();
	current_ = val.cast<object::Primitive>()->value_get()(context_, args);
	break;
      case object::Object::kind_code:
	PING ();
	call_code = true;
	break;
      default:
	PING ();
	current_ = val;
	break;
    }
    if (call_code)
    {
      PING ();
      CORO_CALL (current_ = eval (val.cast<object::Code> ()->value_get ()));
    }

    CORO_END;
  }

  void
  Runner::operator() (ast::FloatExp& e)
  {
    CORO_WITHOUT_CTX ();
    YIELD (e);

    current_ = new object::Float (e.value_get());
    ECHO ("result: " << *current_);

    CORO_END;
  }


  void
  Runner::operator() (ast::ListExp& e)
  {
    typedef std::list<object::rObject> objects;
    typedef std::list<ast::Exp*> exps;
		      // list values
    CORO_WITH_2SLOTS_CTX (objects, values,
			  exps::const_iterator, i);
    YIELD (e);

    PING ();
    // Evaluate every expression in the list
    // FIXME: parallelized?
    for (i = e.value_get ().begin ();
	 i != e.value_get ().end ();
	 ++i)
    {
      CORO_CALL (operator() (**i));
      values.push_back(current_);
    }
    current_ = new object::List (values);
    ECHO ("result: " << *current_);

    CORO_END;
  }


  void
  Runner::operator() (ast::Function& e)
  {
    CORO_WITHOUT_CTX ();
    YIELD (e);

    PING ();
    // FIXME: Arguments.
    current_ = new object::Code (*e.body_get());

    CORO_END;
  }


  void
  Runner::operator() (ast::NegOpExp& e)
  {
    CORO_WITHOUT_CTX ();

    CORO_CALL (eval (e.operand_get ()));
    {
      assert (current_->kind_get() == object::Object::kind_float);
      current_ =
	new object::Float (-1 * current_.cast<object::Float>()->value_get ());
    }

    CORO_END;
  }


  void
  Runner::operator() (ast::Noop&)
  {
    CORO_WITHOUT_CTX ();
    CORO_END;
  }


  void
  Runner::operator() (ast::PipeExp& e)
  {
    CORO_WITHOUT_CTX ();

    // lhs
    ECHO ("job " << ME << ", lhs: {{{" << e.lhs_get () << "}}}");
    CORO_CALL (operator() (e.lhs_get()));
    ECHO ("sending result of lhs");
    emit_result (current_);

    current_.reset ();
    assert (current_.get () == 0);

    // rhs:  start the execution immediately.
    ECHO ("job " << ME << ", rhs: {{{" << e.rhs_get () << "}}}");
    CORO_CALL (operator() (e.rhs_get()));
    ECHO ("sending result of rhs");
    emit_result (current_);

    /* We already returned the result of both our lhs and our rhs so let's
     * reset the current value to make sure nobody will return it again.  If
     * this ever becomes a problem (because we might want `a|b' to return
     * the value of `b') we can leave the value of the rhs in current_
     * instead of returning it.  I haven't do so ATM because the way it's
     * done now enforces the URBI semantics.  */
    current_.reset ();
    assert (current_.get () == 0);

    CORO_END;
  }


  void
  Runner::operator() (ast::SemicolonExp& e)
  {
    CORO_WITHOUT_CTX ();

    // lhs
    ECHO ("job " << ME << ", lhs: {{{" << e.lhs_get () << "}}}");
    CORO_CALL (operator() (e.lhs_get()));
    ECHO ("sending result of lhs");
    emit_result (current_);

    current_.reset ();
    assert (current_.get () == 0);

    /* Allow some time to pass before we execute RHS.  If we don't do this,
     * the ;-operator would act almost like the |-operator because it would
     * always start to execute its RHS immediately.  */
    YIELD (e);
    // rhs
    ECHO ("job " << ME << ", rhs: {{{" << e.rhs_get () << "}}}");
    CORO_CALL (operator() (e.rhs_get()));
    ECHO ("sending result of rhs");
    emit_result (current_);

    CORO_END;
  }


  void
  Runner::operator() (ast::StringExp& e)
  {
    CORO_WITHOUT_CTX ();
    YIELD (e);

    PING ();
    current_ = new object::String(e.value_get());
    ECHO ("result: " << *current_);

    CORO_END;
  }

} // namespace runner
