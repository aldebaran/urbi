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
#define JOB(Runner) static_cast<scheduler::Job*> (Runner)

/// Address of \c this seen as a \c Job (Runner has multiple inheritance).
#define ME JOB (this)

#define AST(Ast) "{{{" << Ast << "}}}"

/// Job echo.
#define JECHO(Title, Ast)                                       \
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
#define YIELD()                                 \
  do                                            \
  {                                             \
    ECHO ("job " << ME << " yielding on AST: "  \
          << &e << ' ' << AST(e));              \
    yield ();                                   \
  } while (0)

#define CATCH_FLOW_EXCEPTION(Type, Keyword, Error)              \
  catch (Type flow_exception)                                   \
  {                                                             \
    if (e.toplevel_get ())                                      \
    {                                                           \
      object::PrimitiveError error(Keyword, Error);             \
      error.location_set(flow_exception.location_get());        \
      raise_error_(error);                                      \
      continue;                                                 \
    }                                                           \
    else                                                        \
      throw;                                                    \
  }

  void
  Runner::raise_error_ (const object::UrbiException& ue)
  {
    std::ostringstream o;
    o << "!!! " << ue.location_get () << ": " << ue.what () << std::ends;
    send_message_ (o.str (), "error");
    BOOST_FOREACH(ast::loc& l, callStack)
      send_message_ (std::string("!!!    called from: ") +
			       boost::lexical_cast<std::string>(l), "error");
    // Reset the current value: there was an error so whatever value it has,
    // it must not be used.
    current_.reset ();
  }

  void Runner::send_message_ (const std::string& text, const std::string& tag)
  {
    UConnection& c = lobby_.cast<object::Lobby>()->value_get().connection;
    c << UConnection::send (text.c_str(), tag.c_str()) << UConnection::endl;
  }

  void
  Runner::work ()
  {
    assert (ast_);

    ECHO ("job " << ME << " starting evaluation of AST: " << ast_
          << ' ' << AST(*ast_));
    operator() (*ast_);
  }


  /*---------------------.
  | Regular operator().  |
  `---------------------*/

  void
  Runner::operator() (ast::And& e)
  {
    // lhs will be evaluated in another Runner, while rhs will be evaluated
    // in this one.

    JECHO ("lhs", e.lhs_get ());
    Runner* lhs = new Runner (*this);
    lhs->ast_ = &e.lhs_get ();
    scheduler_get().add_job (lhs);

    PING ();

    JECHO ("rhs", e.rhs_get ());

    eval (e.rhs_get ());

    // Wait for lhs to terminate
    yield_until_terminated (*lhs);
    delete lhs;

    PING ();

    current_ = object::void_class;
  }

  // Apply a function written in Urbi
  object::rObject
  Runner::apply_urbi (rObject scope, const rObject& func,
		      const object::objects_type& args)
  {
    // The called function.
    ast::Function* fn;
    // Effective (evaluated) argument iterator.
    object::objects_type::const_iterator ei;
    // Formal argument iterator.
    ast::symbols_type::const_iterator fi;

    PING ();
    // Create a new object to store the arguments unless a scope has been
    // given for this purpose.
    rObject bound_args;
    if (scope)
      bound_args = scope;
    else
      bound_args = new object::Object;
    bound_args->locals_set(true);

    // Fetch the called function.
    fn = &func.cast<object::Code> ()->value_get ();

    // Check the arity.
    object::check_arg_count (fn->formals_get().size(), args.size() - 1,
			     __PRETTY_FUNCTION__);

    // Bind formal and effective arguments.
    // The target is "self".
    ei = args.begin();
    bound_args->slot_set (libport::Symbol("self"), *ei);
    // self is also the proto of the function outer scope, so that
    // we look for non-local identifiers in the target itself.
    bound_args->proto_add (*ei);

    // Now bind the non-target arguments.
    ++ei;
    for (fi = fn->formals_get().begin();
	 fi != fn->formals_get().end() && ei != args.end();
	 ++fi, ++ei)
      bound_args->slot_set (**fi, *ei);
    ECHO("bound args: " << *bound_args);
    // Change the current context and call.
    std::swap(bound_args, locals_);

    try {
      current_ = eval (*fn->body_get());
    }
    catch (ast::BreakException& be)
      {
        object::PrimitiveError error("break", "outside a loop");
        error.location_set(be.location_get());
        raise_error_(error);
      }
    catch (ast::ReturnException& re)
      {
        current_ = re.result_get();
      };
    std::swap(bound_args, locals_);

    return current_;
  }

  object::rObject
  Runner::apply (rObject scope, const rObject& func,
                 const object::objects_type& args)
  {
    {
      // Check if any argument is void
      bool first = true;
      BOOST_FOREACH(rObject arg, args)
      {
	if (!first && arg == object::void_class)
	  throw object::WrongArgumentType (__PRETTY_FUNCTION__);
	first = false;
      }
    }

    switch (func->kind_get ())
    {
      case object::Object::kind_primitive:
        PING ();
        current_ = func.cast<object::Primitive>()->value_get()(*this, args);
	break;
      case object::Object::kind_delegate:
        PING();
        current_ = func.cast<object::Delegate>()->value_get()
          ->operator()(*this, args);
	break;
      case object::Object::kind_code:
        PING ();
	current_ = apply_urbi (scope, func, args);
	break;
      default:
        PING ();
        current_ = func;
	break;
    }

    return current_;
  }

  void
  Runner::operator() (ast::Call& e)
  {
    // Whether or not something went wrong
    bool has_error;
    rObject val;
    // Iteration over un-evaluated effective arguments.
    ast::exps_type::const_iterator i;
    ast::exps_type::const_iterator i_end;
    // Gather the arguments, including the target.
    object::objects_type args;
    rObject tgt;

    /*-------------------------.
    | Evaluate the arguments.  |
    `-------------------------*/
    PING ();

    // Iterate over arguments, with a special case for the target.
    i = e.args_get ().begin ();
    i_end = e.args_get ().end ();

    tgt = target(*i);
    // No target?  Abort the call.  This can happen (for instance) when you
    // do a.b () and a does not exist (lookup error).
    if (!tgt)
      return;

    args.push_back (tgt);
    PING ();
    for (++i; i != i_end; ++i)
    {
      eval (**i);
      passert ("argument without a value: " << **i, current_);
      if (current_ == object::void_class)
      {
	object::WrongArgumentType wt(__PRETTY_FUNCTION__);
	wt.location_set((*i)->location_get());
	throw wt;
      }
      PING ();
      args.push_back (current_);
    }

    /*---------------------.
    | Decode the message.  |
    `---------------------*/
    // We may have to run a primitive, or some code.
    // We cannot use CORO_* in a switch.
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
      return;
    // FIXME: Do we need to issue an error message here?
    if (!val)
      return;
    callStack.push_front(e.location_get());
    try {
      apply (0, val, args);
    }
    catch (object::UrbiException& ue)
    {
      ue.location_set (e.location_get ());
      throw;
    };
    callStack.pop_front();

    // Because while returns 0, we can't have a call that returns 0
    // (a function that runs a while for instance).
    // passert ("no value: " << e, current_);
    ECHO (AST(e) << " result: " << *current_);
  }


  void
  Runner::operator() (ast::Float& e)
  {
    YIELD ();

    current_ = new object::Float (e.value_get());
    ECHO ("result: " << *current_);
  }


  void
  Runner::operator() (ast::Function& e)
  {
    YIELD ();

    PING ();
    current_ = new object::Code (*ast::clone(e));
  }


  void
  Runner::operator() (ast::If& e)
  {
    // Evaluate the test.
    JECHO ("test", e.test_get ());
    operator() (e.test_get());

    YIELD();

    if (IS_TRUE(current_))
    {
      JECHO ("then", e.thenclause_get ());
      operator() (e.thenclause_get());
    }
    else
    {
      JECHO ("else", e.elseclause_get ());
      operator() (e.elseclause_get());
    }
  }


  void
  Runner::operator() (ast::List& e)
  {
    typedef std::list<object::rObject> objects;
    typedef std::list<ast::Exp*> exps;
    // list values
    objects values;
    exps::const_iterator i;
    YIELD ();

    PING ();
    // Evaluate every expression in the list
    // FIXME: parallelized?
    for (i = e.value_get ().begin ();
         i != e.value_get ().end ();
         ++i)
    {
      operator() (**i);
      values.push_back(current_);
    }
    current_ = new object::List (values);
    ECHO ("result: " << *current_);
  }


  void
  Runner::operator() (ast::Nary& e)
  {
    // FIXME: other flavor support.
    ast::exec_exps_type::iterator i;
    current_ = object::void_class;
    for (i = e.children_get().begin(); i != e.children_get().end(); ++i)
    {
      current_.reset ();
      JECHO ("child", *i);
      try {
        operator() (*i);
      }
      catch (object::UrbiException& ue)
      {
        raise_error_ (ue);
        continue;
      }
      CATCH_FLOW_EXCEPTION(ast::BreakException, "break", "outside a loop")
        CATCH_FLOW_EXCEPTION(ast::ReturnException, "return",
                             "outside a function");


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
  }


  void
  Runner::operator() (ast::Noop&)
  {
    current_.reset ();
  }


  void
  Runner::operator() (ast::Pipe& e)
  {
    // lhs
    JECHO ("lhs", e.lhs_get ());
    operator() (e.lhs_get());

    // rhs:  start the execution immediately.
    JECHO ("rhs", e.rhs_get ());
    operator() (e.rhs_get());
  }


  void
  Runner::operator() (ast::Scope& e)
  {
    // To each scope corresponds a "locals" object which stores the
    // local variables.  It points to the previous current scope to
    // implement lexical scoping.
    rObject locals;
    if (e.target_get())
    {
      locals = eval(*e.target_get());
      // FIXME: Set the protos to locals_? Set self?
    }
    else
    {
      locals = new object::Object;
      locals->locals_set(true).proto_add (locals_);
    }

    std::swap(locals, locals_);
    super_type::operator()(e.body_get());
    std::swap(locals, locals_);
  }


  void
  Runner::operator() (ast::String& e)
  {
    YIELD ();

    PING ();
    current_ = new object::String(e.value_get());
    ECHO ("result: " << *current_);
  }

  void
  Runner::operator() (ast::Tag&)
  {
    // FIXME: Some code is missing here.
  }

  void
  Runner::operator() (ast::While& e)
  {
    bool broken;

    // Evaluate the test.
    while (true)
    {
      // FIXME: YIELD if second iteration for "while;".

      JECHO ("while test", e.test_get ());
      operator() (e.test_get());
      if (!IS_TRUE(current_))
        break;

      if (e.flavor_get() == ast::flavor_semicolon)
        YIELD();

      JECHO ("while body", e.body_get ());

      broken = false;
      try {
        operator() (e.body_get());
      }
      catch (ast::BreakException&)
      {
        // FIXME: Fix for flavor "," and "&".
        if (e.flavor_get() == ast::flavor_semicolon ||
            e.flavor_get() == ast::flavor_pipe)
          broken = true;
      };
      if (broken)
        break;
    }
    // As far as I know, `while' doesn't return a value in URBI.
    current_.reset ();
  }

  void
  Runner::operator() (ast::Throw& e)
  {
    if (e.kind_get() == ast::break_exception)
      throw ast::BreakException(e.location_get());
    else if (e.kind_get() == ast::return_exception)
    {
      if (e.value_get())
        operator() (*e.value_get());
      else
        current_.reset();
      throw ast::ReturnException(e.location_get(), current_);
    }
  }

  void
  Runner::operator() (ast::Stmt& e)
  {
    JECHO ("expression", e.expression_get ());
    operator() (e.expression_get());
  }

  void
  Runner::operator() (ast::Message& e)
  {
    send_message_(e.text_get(), e.tag_get());
  }

} // namespace runner
