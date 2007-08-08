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
#define YIELD(Ast)                              \
  do                                            \
  {                                             \
    ECHO (ME << " yielding on ast = " << &Ast); \
    yield ();                                   \
    CORO_YIELD ();                              \
  } while (0)

  void
  Runner::work ()
  {
    ast::Ast* ast = Coroutine::take_first_slot<ast::Ast> ();
    assert (ast);
    ECHO (ME << " restarting evaluation of " << ast);
    // Restart evaluation where it was stopped.
    operator() (*ast);
  }

  void
  Runner::emit_result (rObject result)
  {
    context_->value_get ().new_result (result);
  }

  void
  Runner::operator() (const ast::AssignExp& e)
  {
    CORO_INIT_WITH_1SLOT_CTX (const ast::Ast* ast);
    CORO_CTX (ast = &e);
    YIELD (e);

    {
      PING ();
      assert (e.lhs_get().args_get().size () == 1);
      rObject tgt = target(e.lhs_get().args_get().front());
      libport::Symbol s = e.lhs_get().name_get();
      tgt->slot_set (s, eval(e.rhs_get()));
    }

    CORO_END;
    assert (e.lhs_get().args_get().size () == 1);
    rObject tgt = target(e.lhs_get().args_get().front());
    libport::Symbol s = e.lhs_get().name_get();
    tgt->slot_set (s, eval(e.rhs_get()));
  }

  void
  Runner::operator() (const ast::AndExp& e)
  {
    CORO_INIT_WITH_1SLOT_CTX (const ast::Ast* ast);
    CORO_CTX (ast = &e);

    /// FIXME BROKEN
    ECHO ("[LHS] this = " << ME);
    CORO_CALL_IN_BACKGROUND (operator() (e.lhs_get()));
    PING ();
    ECHO ("[RHS] this = " << ME);
    CORO_CALL (operator() (e.rhs_get()));

    CORO_END;
  }

  void
  Runner::operator() (const ast::CallExp& e)
  {
    CORO_INIT_WITH_1SLOT_CTX (const ast::Ast* ast);
    CORO_CTX (ast = &e);
    YIELD (e);

    {
      PING ();
      // Iterate over arguments, with a special case for the target.
      ast::exps_type::const_iterator
        i = e.args_get().begin(),
        i_end = e.args_get().end();
      rObject tgt = target(*i);

      // Ask the target for the handler of the message.
      rObject val = tgt->lookup (e.name_get ());

      // Gather the arguments, including the target.
      object::objects_type args;
      args.push_back (tgt);
      for (++i; i != i_end; ++i)
        args.push_back (eval (**i));

      // We may have to run a primitive, or some code.
      switch (val->kind_get ())
      {
        case object::Object::kind_primitive:
          current_ = val.cast<object::Primitive>()->value_get()(args);
          break;
        case object::Object::kind_code:
          CORO_CALL (current_ = eval (val.cast<object::Code>()->value_get()));
          break;
        default:
          current_ = val;
          break;
      }
    }

    CORO_END;
  }

  void
  Runner::operator() (const ast::FloatExp& e)
  {
    CORO_INIT_WITH_1SLOT_CTX (const ast::Ast* ast);
    CORO_CTX (ast = &e);
    YIELD (e);

    current_ = new object::Float (e.value_get());
    ECHO ("result: " << *current_);

    CORO_END;
  }


  void
  Runner::operator() (const ast::ListExp& e)
  {
    CORO_INIT_WITH_1SLOT_CTX (const ast::Ast* ast);
    CORO_CTX (ast = &e);
    YIELD (e);

    {
      PING ();
      // list values
      std::list<object::rObject> values;
      // Evaluate every expression in the list
      // FIXME: parallelized?
      BOOST_FOREACH (const ast::Exp* v, e.value_get())
      {
        operator() (*v);
        values.push_back(current_);
      }
      current_ = new object::List (values);
      ECHO ("result: " << *current_);
    }

    CORO_END;
  }


  void
  Runner::operator() (const ast::Function& e)
  {
    CORO_INIT_WITH_1SLOT_CTX (const ast::Ast* ast);
    CORO_CTX (ast = &e);
    YIELD (e);

    PING ();
    // FIXME: Arguments.
    current_ = new object::Code (*e.body_get());

    CORO_END;
  }


  void
  Runner::operator() (const ast::NegOpExp& e)
  {
    CORO_INIT_WITH_1SLOT_CTX (const ast::Ast* ast);
    CORO_CTX (ast = &e);

    {
      rObject val = eval (e.operand_get ());
      assert (val->kind_get() == object::Object::kind_float);
      current_ =
        new object::Float (-1 * val.cast<object::Float>()->value_get ());
    }

    CORO_END;
  }


  void
  Runner::operator() (const ast::SemicolonExp& e)
  {
    CORO_INIT_WITH_1SLOT_CTX (const ast::Ast* ast);
    CORO_CTX (ast = &e);

    ECHO ("[LHS] this = " << ME);
    CORO_CALL (operator() (e.lhs_get()));
    ECHO ("sending result of lhs");
    if (current_.get ())
      emit_result (current_);
    current_.reset ();
    assert (current_.get () == 0);
    ECHO ("[RHS] this = " << ME);
    CORO_CALL (operator() (e.rhs_get()));
    ECHO ("sending result of rhs");
    if (current_.get ())
      emit_result (current_);

    CORO_END;
  }


  void
  Runner::operator() (const ast::StringExp& e)
  {
    CORO_INIT_WITH_1SLOT_CTX (const ast::Ast* ast);
    CORO_CTX (ast = &e);
    YIELD (e);

    PING ();
    current_ = new object::String(e.value_get());
    ECHO ("result: " << *current_);

    CORO_END;
  }

} // namespace runner
