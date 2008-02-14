/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include <sstream>

#include <libport/foreach.hh>
#include <libport/symbol.hh>

#include "kernel/uconnection.hh"

#include "ast/clone.hh"
#include "object/alien.hh"
#include "object/atom.hh"
#include "object/urbi-exception.hh"
#include "object/idelegate.hh"
#include "object/symbols.hh"
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

#define MAYBE_YIELD(Flavor)			\
  do						\
  {						\
    if (Flavor == ast::flavor_semicolon)	\
      YIELD();					\
  } while (0)

#define CATCH_FLOW_EXCEPTION(Type, Keyword, Error)              \
  catch (Type flow_exception)                                   \
  {                                                             \
    if (e.toplevel_get ())                                      \
    {                                                           \
      object::PrimitiveError error(Keyword, Error);             \
      show_error_(error, flow_exception.location_get());        \
      continue;                                                 \
    }                                                           \
    else                                                        \
      throw;                                                    \
  }

/** Catch UrbiException, execute Code, then display the error if not allready
 * done, and rethrow it. Also execute Code if no exception caught. */
#define PROPAGATE_URBI_EXCEPTION(Loc, Code)          \
  catch(object::UrbiException& ue)                   \
  {                                                  \
    Code                                             \
    current_.reset();                                \
    show_error_(ue, Loc);                            \
    throw;                                           \
  }                                                  \
  Code

  void
  Runner::show_error_ (object::UrbiException& ue, const ast::loc& l)
  {
    if (ue.location_is_set())
      return;
    ue.location_set(l);
    std::ostringstream o;
    o << "!!! " << ue.location_get () << ": " << ue.what ();
    send_message_ ("error", o.str ());
    foreach(ast::Call* c, call_stack_)
    {
      o.str("");
      o << "!!!    called from: " << c->location_get () << ": "
	<< c->name_get ();
      send_message_ ("error", o.str ());
    }
    // Reset the current value: there was an error so whatever value it has,
    // it must not be used.
    current_.reset ();
  }

  void
  Runner::send_message_ (const std::string& tag, const std::string& msg)
  {
    UConnection& c = lobby_->value_get().connection;
    c << UConnection::send (msg.c_str(), tag.c_str()) << UConnection::endl;
  }

  void
  Runner::work ()
  {
    assert (ast_);
    JECHO ("starting evaluation of AST: " << ast_, *ast_);
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
    Runner lhs (*this);
    lhs.ast_ = &e.lhs_get ();
    lhs.start_job ();

    PING ();

    JECHO ("rhs", e.rhs_get ());

    eval (e.rhs_get ());

    // Wait for lhs to terminate
    yield_until_terminated (lhs);

    PING ();

    current_ = object::void_class;
  }

  // Apply a function written in Urbi.
  object::rObject
  Runner::apply_urbi (const rObject& func,
		      const object::objects_type& args,
		      const rObject call_message)
  {
    // The called function.
    ast::Function* fn = &func.cast<object::Code> ()->value_get ();
    // Effective (evaluated) argument iterator.
    object::objects_type::const_iterator ei;
    // Formal argument iterator.
    ast::symbols_type::const_iterator fi;
    // Scope in which to evaluate the function, which may be nil
    rObject scope = func->slot_get (SYMBOL(context));

    PING ();
    // Create a new object to store the arguments. If a scope has been
    // provided, it is the first parent.
    rObject bound_args = new object::Object;
    bound_args->locals_set(true);
    if (scope != object::nil_class)
      bound_args->proto_add (scope);

    // Bind formal and effective arguments if the caller has not provided
    // a scope. If a scope has been given, it is up to the caller to set
    // it up.
    ei = args.begin();
    if (scope == object::nil_class)
    {
      bound_args->slot_set (SYMBOL(self), *ei);
      // self is also the proto of the function outer scope, so that
      // we look for non-local identifiers in the target itself.
      bound_args->proto_add (*ei);
    }

    // If this is a strict function, check the arity and bind the formal
    // arguments. Otherwise, bind the call message.

    if (fn->strict_get())
    {
      assert (!call_message);
      object::check_arg_count (fn->formals_get().size() + 1, args.size(),
			       "");

      ++ei;
      for (fi = fn->formals_get().begin();
	   fi != fn->formals_get().end() && ei != args.end();
	   ++fi, ++ei)
	bound_args->slot_set (**fi, *ei);
    }
    else
    {
      assert (call_message);
      bound_args->slot_set (SYMBOL(call), call_message);
    }

    ECHO("bound args: " << *bound_args);

    // Change the current context and call. But before, yield() so that
    // an infinite recursion gets a better chance to be detected.
    std::swap(bound_args, locals_);
    YIELD ();

    try
    {
      current_ = eval (*fn->body_get());
    }
    catch (ast::BreakException& be)
    {
      object::PrimitiveError error("break", "outside a loop");
      show_error_(error, be.location_get());
    }
    catch (ast::ReturnException& re)
    {
      current_ = re.result_get();
    }
    PROPAGATE_URBI_EXCEPTION(fn->location_get(), std::swap(bound_args, locals_);)

    return current_;
  }

  object::rObject
  Runner::eval_in_scope (rObject scope, ast::Exp& e)
  {
    std::swap (locals_, scope);
    try {
      eval (e);
    }
    PROPAGATE_URBI_EXCEPTION(e.location_get(), std::swap (locals_, scope);)
    return current_;
  }

  object::rObject
  Runner::apply (const rObject& func,
		 const object::objects_type& args,
		 const rObject call_message)
  {
    {
      // Check if any argument is void
      bool first = true;
      foreach (rObject arg, args)
      {
	if (!first && arg == object::void_class)
	  throw object::WrongArgumentType ("");
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
	current_ = apply_urbi (func, args, call_message);
	break;
      default:
	PING ();
	current_ = func;
	break;
    }

    return current_;
  }

  void
  Runner::push_evaluated_arguments (object::objects_type& args,
				    const ast::exps_type& ue_args)
  {
    ast::exps_type::const_iterator arg = ue_args.begin ();
    // Skip target
    ++arg;
    ast::exps_type::const_iterator args_end = ue_args.end ();

    for (; arg != args_end; ++arg)
    {
      eval (**arg);
      passert ("argument without a value: " << **arg, current_);
      if (current_ == object::void_class)
      {
	object::WrongArgumentType wt("");
	show_error_(wt, (*arg)->location_get());
	throw wt;
      }
      PING ();
      args.push_back (current_);
    }
  }

  object::rObject
  Runner::build_call_message (const rObject& tgt, const libport::Symbol& msg,
			      const ast::exps_type& args) const
  {
    rObject res = object::clone (object::call_class);

    // Set the sender to be the current self. self must always exist.
    res->slot_set (SYMBOL(sender),
		   locals_->slot_get (SYMBOL(self)));

    // Set the target to be the object on which the function is applied.
    res->slot_set (SYMBOL(target), tgt);

    // Set the name of the message call.
    res->slot_set (SYMBOL(message), new object::String(msg));

    // Set the args to be the unevaluated expressions, including the target.
    // We use an Alien here.
    res->slot_set (SYMBOL(args), box(const ast::exps_type&, args));

    // Store the current context in which the arguments must be evaluated.
    res->slot_set (SYMBOL(context), locals_);

    return res;
  }

  void
  Runner::operator() (ast::Call& e)
  {
    PING ();

    const rObject tgt = target(e.args_get ().front ());

    // No target?  Abort the call.  This can happen (for instance) when you
    // do a.b () and a does not exist (lookup error).
    if (!tgt)
      return;

    /*---------------------.
    | Decode the message.  |
    `---------------------*/

    rObject val;

    // We may have to run a primitive, or some code.
    try {
      // Ask the target for the handler of the message.
      val = tgt->slot_get (e.name_get ());
    }
    PROPAGATE_URBI_EXCEPTION(e.location_get(), {})

    assert(val);

    /*-------------------------.
    | Evaluate the arguments.  |
    `-------------------------*/

    // Gather the arguments, including the target.
    rObject call_message;
    object::objects_type args;
    args.push_back (tgt);

    if (val->kind_get () == object::Object::kind_code)
    {
      // Build either the evaluated argument list or the call message
      ast::Function* fn = &val.cast<object::Code> ()->value_get ();
      if (fn->strict_get())
	push_evaluated_arguments (args, e.args_get ());
      else
	call_message = build_call_message (tgt, e.name_get(), e.args_get ());
    }
    else
      push_evaluated_arguments (args, e.args_get ());

    call_stack_.push_front(&e);
    try {
      apply (val, args, call_message);
    }
    PROPAGATE_URBI_EXCEPTION(e.location_get(), call_stack_.pop_front();)

    // Because while returns 0, we can't have a call that returns 0
    // (a function that runs a while for instance).
    // passert ("no value: " << e, current_);
    ECHO (AST(e) << " result: " << *current_);
  }


  void
  Runner::operator() (ast::Function& e)
  {
    PING ();
    current_ = new object::Code (*ast::clone(e));
  }


  void
  Runner::operator() (ast::If& e)
  {
    // Evaluate the test.
    JECHO ("test", e.test_get ());
    operator() (e.test_get());

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
	show_error_(ue, (*i)->location_get());
	if (!e.toplevel_get())
	  throw;
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
  Runner::operator() (ast::Object& e)
  {
    // Make a copy of the value, otherwise each time we pass here, we
    // use the same object.  For instance
    //
    // for (var i = 0; i < 2; i++)
    //    { var a = 0; a++; cout << a }
    //
    // would display "1" and "2", since the ast::Object(0) would be used,
    // and modified via the first "a++".
    //
    // Note that using "clone" is probably not what we want if we introduce
    // a syntax a la Self to create arbitrary objects: we want a real clone
    // of the object (in the C++ sense of clone) rather than a descendant
    // (in the prototypal sense of clone).  It turns out ast::Object is used
    // only for simple object::Atom, i.e., the case where "clone" and "dup"
    // behave equally.
    current_ = object::clone(e.value_get());
    ECHO ("result: " << *current_);
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
    rObject target;
    locals = new object::Object;
    locals->locals_set(true);
    // FIXME: is this the correct order? depends on getSlot algorithm.
    locals->proto_add (locals_);
    if (e.target_get())
    {
      target = eval(*e.target_get());
      // Be safe, do not inherit from VisibilityScope but copy the slots.
      rObject vscope =
	object::object_class->slot_get(SYMBOL(VisibilityScope));
      locals->slot_set(SYMBOL(setSlot),
		       vscope->slot_get(SYMBOL(setSlot)));
      locals->slot_set(SYMBOL(updateSlot),
		       vscope->slot_get(SYMBOL(updateSlot)));
      locals->slot_set(SYMBOL(self), target);
      locals->slot_set(SYMBOL(__target), target);
      locals->proto_add(target);
    }

    std::swap(locals, locals_);
    try
    {
      super_type::operator()(e.body_get());
    }
    PROPAGATE_URBI_EXCEPTION(e.location_get(), std::swap(locals, locals_);)
  }


  void
  Runner::operator() (ast::Tag&)
  {
    // FIXME: Some code is missing here.
  }

  void
  Runner::operator() (ast::While& e)
  {
    // Evaluate the test.
    while (true)
    {
      // FIXME: YIELD if second iteration for "while;".

      JECHO ("while test", e.test_get ());
      operator() (e.test_get());
      if (!IS_TRUE(current_))
	break;

      MAYBE_YIELD(e.flavor_get());

      JECHO ("while body", e.body_get ());

      try {
	operator() (e.body_get());
      }
      catch (ast::BreakException&)
      {
	// FIXME: Fix for flavor "," and "&".
	if (e.flavor_get() == ast::flavor_semicolon ||
	    e.flavor_get() == ast::flavor_pipe)
	  break;
      };
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
    send_message_(e.tag_get(), e.text_get());
  }

  void
  Runner::operator() (ast::Foreach& e)
  {
    // Evaluate the list attribute, and check its type.
    JECHO ("foreach list", e.list_get());
    operator() (e.list_get());
    try
    {
      TYPE_CHECK(current_, object::List);
    }
    PROPAGATE_URBI_EXCEPTION(e.location_get(), {});

    JECHO("foreach body", e.body_get());

    // We need to copy the pointer on the list, otherwise the list will be
    // destroyed when children are visited and current_ is modified.
    rObject l = current_;

    // The list of runners launched for each value in the list if the flavor
    // is "&".
    std::list<Runner> runners;

    // Iterate on each value.
    foreach (rObject o, VALUE(l, object::List))
    {
      // Define a new local scope for each loop, and set the index.
      rObject locals = new object::Object;
      locals->locals_set(true);
      locals->proto_add(locals_);
      locals->slot_set(e.index_get(), o);

      // for& ... in loop.
      if (e.flavor_get() == ast::flavor_and)
      {
	// Create the new runner and launch it.
	runners.push_back(Runner(*this));
	runners.back().locals_ = locals;
	runners.back().ast_ = &e.body_get();
	runners.back().start_job();
      }
      else // for| and for;
      {
	std::swap(locals, locals_);

	MAYBE_YIELD(e.flavor_get());
	try
	{
	  operator() (e.body_get());
	}
	catch (ast::BreakException&)
	{
	  break;
	}
	// Restore previous locals_, even if an exception was thrown.
	PROPAGATE_URBI_EXCEPTION(e.location_get(), std::swap(locals, locals_);)
      }
    }

    // Wait for all runners to terminate.
    foreach(Runner& r, runners)
      yield_until_terminated(r);

    // For the moment return void.
    current_ = object::void_class;
  }

} // namespace runner
