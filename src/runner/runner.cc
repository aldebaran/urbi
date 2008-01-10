/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include <sstream>
#include <boost/foreach.hpp>
#include <libport/symbol.hh>

#include "kernel/uconnection.hh"

#include "ast/clone.hh"
#include "object/atom.hh"
#include "object/urbi-exception.hh"
#include "object/idelegate.hh"
#include "runner/runner.hh"

namespace runner
{

/// Address of \a Runner seen as a \c Job (Runner has multiple inheritance).
#define JOB(Runner) static_cast<Job*> (Runner)

/// Address of \c this seen as a \c Job (Runner has multiple inheritance).
#define ME JOB (this)

#define AST(Ast) "{{{" << Ast << "}}}"

/// Job echo.
#define JECHO(Title, Ast)					\
  ECHO ("job " << ME << ", " Title ": " << AST(Ast))


/** Call this macro at the very beginning of the evaluation of an AST
 * node.  Nodes that should yield are that doing something useful.  For
 * instance in "1;2" we have 3 nodes: two values in a Semicolon.  The
 * Semicolon does not do anything useful, it simply recurses in its
 * left hand side.  However, the "1" does something useful: it calculates
 * a value (even though calculating the constant "1" is somewhat trivial),
 * so it should most probably start with a YIELD.
 *
 * The Ast is expected to be named "e".  */
#define YIELD()							\
  do                                                            \
  {                                                             \
    ECHO ("job " << ME << " yielding on AST: "                  \
	  << &e << ' ' << AST(e));				\
    CORO_YIELD ();                                              \
  } while (0)

  void
  Runner::raise_error_ (const object::UrbiException& ue)
  {
    UConnection& c = lobby_.cast<object::Lobby>()->value_get().connection;
    std::ostringstream o;
    o << "!!! " << ue.location_get () << ": " << ue.what () << std::ends;
    c << UConnection::send (o.str ().c_str (), "error")
      << UConnection::endl;
    // Reset the current value: there was an error so whatever value it has,
    // it must not be used.
    current_.reset ();
  }

  void
  Runner::work ()
  {
    assert (ast_);

    if (!started_)
    {
      ECHO ("job " << ME << " starting evaluation of AST: " << ast_
	    << ' ' << AST(*ast_));
      started_ = true;
    }
    else
      ECHO ("job " << ME << " restarting evaluation of AST: " << ast_
	    << ' ' << AST(*ast_)
	    << ' ' << context_count () << " contexts in the coroutine stack");
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
    Runner& r = dynamic_cast<Runner&> (coro);
    ECHO (ME << " join with " << JOB (&r));
    /*
     * Alright so for now, we only join other coroutines for the &-operator.
     * So what this line does is that it fetches the result of that
     * coroutine (which is about to delete itself).  Admittedly, this line
     * do better be in the operator (And) but it's been moved here
     * temporarily due to a small bug in the coroutine implementation: Once
     * we'll be back to the operator (And) the coroutine which has finished
     * may have deleted itself and we could be accessing its current_
     * attribute whereas the object is no longer valid.  So this line will
     * move to operator (And) once the coroutine properly handle its
     * lifetime.
     */
    current_ = r.current_;
  }



  /*---------------------.
  | Regular operator().  |
  `---------------------*/

  void
  Runner::operator() (ast::And& e)
  {
    CORO_WITHOUT_CTX ();

    {
      JECHO ("lhs", e.lhs_get ());
      Runner* lhs = new Runner (*this);
      lhs->ast_ = &e.lhs_get ();
      CORO_CALL_IN_BACKGROUND (lhs, lhs->eval (e.lhs_get ()));

      PING ();
      JECHO ("rhs", e.rhs_get ());
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
  Runner::operator() (ast::Call& e)
  {
    CORO_CTX_VARS
      ((11, (
	  // Whether or not we must issue a real URBI function call
	  (bool, call_code),
	  // Whether or not something went wrong
	  (bool, has_error),
	  (rObject, val),
	  // Iteration over un-evaluated effective arguments.
	  (ast::exps_type::const_iterator, i),
	  (ast::exps_type::const_iterator, i_end),
	  // Gather the arguments, including the target.
	  (object::objects_type, args),
	  (rObject, tgt),
	  // Formal argument iterator.
	  (ast::symbols_type::const_iterator, fi),
	  // Effective (evaluated) argument iterator.
	  (object::objects_type::const_iterator, ei),
	  // Object to bind the arguments.
	  (rObject, bound_args),
	  // The called function.
	  (ast::Function*, fn)
	  )));

    /*-------------------------.
    | Evaluate the arguments.  |
    `-------------------------*/
    PING ();

    // Iterate over arguments, with a special case for the target.
    i = e.args_get ().begin ();
    i_end = e.args_get ().end ();

    CORO_CALL (tgt = target(*i));
    // No target?  Abort the call.  This can happen (for instance) when you
    // do a.b () and a does not exist (lookup error).
    if (!tgt)
      CORO_RETURN;

    args.push_back (tgt);
    PING ();
    for (++i; i != i_end; ++i)
    {
      CORO_CALL (eval (**i));
      passert ("argument without a value: " << **i, current_);
      PING ();
      args.push_back (current_);
    }

    /*---------------------.
    | Decode the message.  |
    `---------------------*/
    // We may have to run a primitive, or some code.
    // We cannot use CORO_* in a switch.
    call_code = false;
    has_error = false;
    try {
      // Ask the target for the handler of the message.
      val = tgt->slot_get (e.name_get ());
    }
    catch (object::UrbiException& ue)
    {
      ue.location_set (e.location_get ());
      raise_error_ (ue);
      current_.reset ();
      has_error = true;
    }
    if (has_error)
      CORO_RETURN;
    // FIXME: Do we need to issue an error message here?
    if (!val)
      CORO_RETURN;

    switch (val->kind_get ())
    {
      case object::Object::kind_primitive:
	PING ();
	try {
	  current_ = val.cast<object::Primitive>()->value_get()(lobby_, args);
	}
	catch (object::UrbiException& ue)
	{
	  ue.location_set (e.location_get ());
	  throw;
	}
	break;
      case object::Object::kind_delegate:
	PING();
	try {
	  current_ = val.cast<object::Delegate>()->value_get()
	    ->operator()(lobby_, args);
	}
	catch (object::UrbiException& ue)
	{
	  ue.location_set (e.location_get ());
	  throw;
	}
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

    /*---------------------------.
    | Calling an Urbi function.  |
    `---------------------------*/
    if (call_code)
    {
      PING ();
      // Create a new object to store the arguments.
      bound_args = new object::Object;
      bound_args->locals_set(true);

      // Fetch the called function.
      fn = &val.cast<object::Code> ()->value_get ();

      // Check the arity.
      try {
	object::check_arg_count (fn->formals_get().size(), args.size() - 1,
				 __PRETTY_FUNCTION__);
      }
      catch (object::UrbiException& ue)
      {
	ue.location_set (e.location_get ());
	throw;
      }

      // Bind formal and effective arguments.
      // The target is "self".
      ei = args.begin();
      bound_args->slot_set (libport::Symbol("self"), *ei);
      // self is also the parent of the function outer scope, so that
      // we look for non-local identifiers in the target itself.
      bound_args->parent_add (*ei);

      // Now bind the non-target arguments.
      ++ei;
      for (fi = fn->formals_get().begin();
	   fi != fn->formals_get().end() && ei != args.end();
	   ++fi, ++ei)
	bound_args->slot_set (**fi, *ei);
      ECHO("bound args: " << *bound_args);
      // Change the current context and call.
      std::swap(bound_args, locals_);

      CORO_CALL_CATCH (current_ = eval (*fn->body_get());,
	catch (ast::BreakException& be)
	{
	  object::PrimitiveError error("break", "called outside a loop");
	  error.location_set(be.location_get());
	  raise_error_(error);
	}
	catch (ast::ReturnException& re)
	{
	  current_ = re.result_get();
	});
      std::swap(bound_args, locals_);
    }

    // Because while returns 0, we can't have a call that returns 0
    // (a function that runs a while for instance).
    // passert ("no value: " << e, current_);
    ECHO (AST(e) << " result: " << *current_);
    CORO_END;
  }


  void
  Runner::operator() (ast::Float& e)
  {
    CORO_WITHOUT_CTX ();
    YIELD ();

    current_ = new object::Float (e.value_get());
    ECHO ("result: " << *current_);

    CORO_END;
  }


  void
  Runner::operator() (ast::Function& e)
  {
    CORO_WITHOUT_CTX ();
    YIELD ();

    PING ();
    current_ = new object::Code (*ast::clone(e));

    CORO_END;
  }


  void
  Runner::operator() (ast::If& e)
  {
    CORO_WITHOUT_CTX ();

    // Evaluate the test.
    JECHO ("test", e.test_get ());
    CORO_CALL (operator() (e.test_get()));

    YIELD();

    if (IS_TRUE(current_))
    {
      JECHO ("then", e.thenclause_get ());
      CORO_CALL (operator() (e.thenclause_get()));
    }
    else
    {
      JECHO ("else", e.elseclause_get ());
      CORO_CALL (operator() (e.elseclause_get()));
    }

    CORO_END;
  }


  void
  Runner::operator() (ast::List& e)
  {
    typedef std::list<object::rObject> objects;
    typedef std::list<ast::Exp*> exps;
		      // list values
    CORO_WITH_2SLOTS_CTX (objects, values,
			  exps::const_iterator, i);
    YIELD ();

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
  Runner::operator() (ast::Nary& e)
  {
    // FIXME: other flavor support.
    CORO_CTX_VARS
      ((1, (
	  (ast::exec_exps_type::iterator, i)
	  )));

    for (i = e.children_get().begin(); i != e.children_get().end(); ++i)
    {
      current_.reset ();
      JECHO ("child", *i);
      CORO_CALL_CATCH (operator() (*i->first);,
	catch (object::UrbiException& ue)
	{
	  raise_error_ (ue);
	  continue;
	});

      if (e.toplevel_get () && current_.get ())
      {
	ECHO ("toplevel: returning a result to the connection.");
	lobby_->value_get ().connection.new_result (current_);
	current_.reset ();
      }

      /* Allow some time to pass before we execute what follows.  If
	 we don't do this, the ;-operator would act almost like the
	 |-operator because it would always start to execute its RHS
	 immediately.  */
      YIELD ();
    }

    // FIXME: I am very afraid that because of the YIELD above, some
    // command are added right before this clear().  Hence the assert.
    assert (i == e.children_get().end());
    if (e.toplevel_get ())
      e.clear();

    CORO_END;
  }


  void
  Runner::operator() (ast::Noop&)
  {
    CORO_WITHOUT_CTX ();
    current_.reset ();
    CORO_END;
  }


  void
  Runner::operator() (ast::Pipe& e)
  {
    CORO_WITHOUT_CTX ();

    // lhs
    JECHO ("lhs", e.lhs_get ());
    CORO_CALL (operator() (e.lhs_get()));

    // rhs:  start the execution immediately.
    JECHO ("rhs", e.rhs_get ());
    CORO_CALL (operator() (e.rhs_get()));

    CORO_END;
  }


  void
  Runner::operator() (ast::Scope& e)
  {
    // To each scope corresponds a "locals" object which stores the
    // local variables.  It points to the previous current scope to
    // implement lexical scoping.
    CORO_WITH_1SLOT_CTX (rObject, locals);
    if (e.target_get())
    {
      CORO_CALL (locals = eval(*e.target_get()));
      // FIXME: Set the parents to locals_? Set self?
    }
    else
    {
      locals = new object::Object;
      locals->locals_set(true).parent_add (locals_);
    }

    std::swap(locals, locals_);
    CORO_CALL (super_type::operator()(e.body_get()));
    std::swap(locals, locals_);

    CORO_END;
  }


  void
  Runner::operator() (ast::String& e)
  {
    CORO_WITHOUT_CTX ();
    YIELD ();

    PING ();
    current_ = new object::String(e.value_get());
    ECHO ("result: " << *current_);

    CORO_END;
  }

  void
  Runner::operator() (ast::Tag&)
  {
    CORO_WITHOUT_CTX ();
    // FIXME: Some code is missing here.
    CORO_END;
  }

  void
  Runner::operator() (ast::While& e)
  {
    CORO_CTX_VARS
      ((1, (
	  (bool, broken)
	  )));

    // Evaluate the test.
    while (true)
    {
      // FIXME: YIELD if second iteration for "while;".

      JECHO ("while test", e.test_get ());
      CORO_CALL (operator() (e.test_get()));
      if (!IS_TRUE(current_))
	break;

      if (e.flavor_get() == ast::flavor_semicolon)
	YIELD();

      JECHO ("while body", e.body_get ());

      broken = false;
      CORO_CALL_CATCH (operator() (e.body_get());,
	catch (ast::BreakException&)
	{
	  // FIXME: Fix for flavor "," and "&".
	  if (e.flavor_get() == ast::flavor_semicolon ||
	      e.flavor_get() == ast::flavor_pipe)
	  broken = true;
	});
      if (broken)
	break;
    }
    // As far as I know, `while' doesn't return a value in URBI.
    current_.reset ();

    CORO_END;
  }

  void
  Runner::operator() (ast::Throw& e)
  {
    CORO_WITHOUT_CTX ();

    if (e.kind_get() == ast::break_exception)
      throw ast::BreakException(e.location_get());
    else if (e.kind_get() == ast::return_exception)
    {
      if (e.value_get())
	CORO_CALL (operator() (*e.value_get()));
      else
	current_.reset();
      throw ast::ReturnException(e.location_get(), current_);
    }

    CORO_END;
  }

} // namespace runner
