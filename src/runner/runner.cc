/**
 ** \file runner/runner.cc
 ** \brief Implementation of runner::Runner.
 */

//#define ENABLE_DEBUG_TRACES
#include "libport/compiler.hh"

#include <deque>
#include <sstream>

#include <libport/finally.hh>
#include <libport/foreach.hh>
#include <libport/symbol.hh>

#include "kernel/exception.hh"
#include "kernel/uconnection.hh"

#include "ast/clone.hh"
#include "object/alien.hh"
#include "object/atom.hh"
#include "object/global-class.hh"
#include "object/idelegate.hh"
#include "object/lazy.hh"
#include "object/object.hh"
#include "object/symbols.hh"
#include "object/urbi-exception.hh"
#include "runner/runner.hh"
#include "parser/tweast.hh"
#include "parser/uparser.hh"

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

/** Catch exceptions, execute Code, then display the error if not already
 * done, and rethrow it. Also execute Code if no exception caught. */
#define PROPAGATE_EXCEPTION(Loc, Code)			\
  catch(object::UrbiException& ue)			\
  {							\
    Code;						\
    current_.reset();					\
    propagate_error_(ue, Loc);				\
    throw;						\
  }							\
  catch(ast::FlowException& e)				\
  {							\
    Code;						\
    current_.reset();					\
    throw;						\
  }							\
  catch(scheduler::SchedulerException& e)		\
  {							\
    Code;						\
    current_.reset();					\
    throw;						\
  }							\
  catch(kernel::exception& e)				\
  {							\
    std::cerr << "Unexpected exception propagated: "	\
	      << e.what() << std::endl;			\
    Code;						\
    throw;						\
  }							\
  catch(...)						\
  {							\
    std::cerr << "Unknown exception propagated\n";	\
    Code;						\
    throw;						\
  }							\
  Code

  static void (*swap) (Runner::rObject&, Runner::rObject&) =
		 &std::swap<Runner::rObject>;

  static std::deque<const libport::Symbol*>
  decompose_tag_chain (const ast::Exp* e)
  {
    std::deque<const libport::Symbol*> res;
    while (e)
    {
      const ast::Call* c = dynamic_cast<const ast::Call*> (e);
      if (!c || c->args_get ().size () != 1)
	throw object::ImplicitTagComponentError (e->location_get ());
      res.push_front (&c->name_get ());
      e = c->args_get ().front ();
    }
    assert (!res.empty ());
    return res;
  }

  void
  Runner::show_error_ (object::UrbiException& ue)
  {
    std::ostringstream o;
    o << "!!! " << ue.location_get () << ": " << ue.what ();
    send_message_ ("error", o.str ());
    // We cannot use a const_reverse_iterator here because of a defect
    // in the C++ standard which doesn't offer a "!=" operator for
    // reverse_iterator (as returned by rend()) and const_reverse_iterator.
    // This is fixed in the draft for the next C++ standard and in
    // g++ >= 4.1.1, but we are using older g++ on some platforms.
    call_stack_type& backtrace = const_cast<call_stack_type&>(ue.backtrace_get());
    for (call_stack_type::reverse_iterator c =
	   backtrace.rbegin();
	 c != backtrace.rend();
	 ++c)
    {
      o.str("");
      o << "!!!    called from: " << (*c)->location_get () << ": "
	<< (*c)->name_get ();
      send_message_ ("error", o.str ());
    }
  }

  void
  Runner::propagate_error_ (object::UrbiException& ue, const ast::loc& l)
  {
    if (!ue.location_is_set())
      ue.location_set(l);
    if (!ue.backtrace_is_set())
      ue.backtrace_set(call_stack_);
    // Reset the current value: there was an error so whatever value it has,
    // it must not be used.
    current_.reset ();
  }

  void
  Runner::send_message_ (const std::string& tag, const std::string& msg)
  {
    UConnection& c = lobby_->value_get().connection;
    c.send (msg.c_str(), tag.c_str());
    c.endline();
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
  Runner::operator() (const ast::And& e)
  {
    // lhs will be evaluated in another Runner, while rhs will be evaluated
    // in this one. We will be the new runner parent, as we have the same
    // tags.

    JECHO ("lhs", e.lhs_get ());
    Runner* lhs = new Runner (*this, &e.lhs_get ());

    // Propagate errors between left-hand side and right-hand side runners.
    link (lhs);

    // Keep a reference on the runner so that it doesn't get destroyed
    // if it finishes first.
    scheduler::rJob lhs_ = lhs->myself_get ();

    lhs->start_job ();

    JECHO ("rhs", e.rhs_get ());
    eval (e.rhs_get ());

    // Wait for lhs to terminate
    yield_until_terminated (*lhs);

    current_ = object::void_class;
  }

  // Apply a function written in Urbi.
  object::rObject
  Runner::apply_urbi (const rObject& func,
		      const libport::Symbol& msg,
		      const object::objects_type& args,
		      rObject call_message)
  {
    // The called function.
    ast::Function& fn = func.unsafe_cast<object::Code> ()->value_get ();

    // If the function is lazy and there's no call message, forge
    // one. This happen when a lazy function is invoked with eval, for
    // instance.
    if (!fn.strict() && call_message == 0)
    {
      object::objects_type lazy_args;
      foreach (const rObject& o, args)
	lazy_args.push_back(mkLazy(*this, o));
      call_message = build_call_message(args[0], msg, lazy_args);
    }

    // Create the function's outer scope, with the first argument as
    // 'self'. The inner scope will be created when executing ()
    // on ast::Scope.
    rObject scope = object::Object::make_method_scope(args.front());
    scope->slot_set(SYMBOL(code), func);

    // If this is a strict function, check the arity and bind the formal
    // arguments. Otherwise, bind the call message.
    if (fn.strict())
    {
      ast::symbols_type& formals = *fn.formals_get();
      object::check_arg_count (formals.size() + 1, args.size(), "");
      // Effective (evaluated) argument iterator.
      // Skip "self" which has already been handled.
      object::objects_type::const_iterator ei = ++args.begin();
      foreach (libport::Symbol* s, formals)
	scope->slot_set (*s, *ei++);
    }
    else
    {
      scope->slot_set (SYMBOL(call), call_message);
    }

    ECHO("scope: " << *scope);

    // Change the current context and call. But before, check that we
    // are not exhausting the stack space, for example in an infinite
    // recursion.
    std::swap(scope, locals_);
    libport::Finally finally;
    finally << boost::bind(swap, ref(scope), ref(locals_));

    check_stack_space ();

    try
    {
      try
      {
	current_ = eval (*fn.body_get());
	run_at_exit (locals_);
      }
      catch (...)
      {
	run_at_exit (locals_);
	throw;
      }
    }
    catch (ast::BreakException& be)
    {
      object::PrimitiveError error("break", "outside a loop");
      propagate_error_(error, be.location_get());
      throw error;
    }
    catch (ast::ReturnException& re)
    {
      current_ = re.result_get();
      if (!current_)
	current_ = object::void_class;
    }
    PROPAGATE_EXCEPTION(fn.location_get(), );

    return current_;
  }

  object::rObject
  Runner::eval_in_scope (rObject scope, const ast::Exp& e)
  {
    try
    {
      std::swap (locals_, scope);
      libport::Finally finally;
      finally << boost::bind(swap, ref(scope), ref(locals_));
      eval (e);
    }
    PROPAGATE_EXCEPTION(e.location_get(),
			{
			  run_at_exit (scope);
			});
    return current_;
  }

  namespace
  {
    // Helper to determine whether a function accepts void parameters
    static inline bool
    acceptVoid(object::rObject f)
    {
      try
      {
	return IS_TRUE(f->slot_get(SYMBOL(acceptVoid)));
      }
      catch (object::LookupError&)
      {
	// acceptVoid is undefined. Refuse void parameter by default.
	return false;
      }
    }
  }

  object::rObject
  Runner::apply (const rObject& func,
		 const libport::Symbol msg,
		 object::objects_type args,
		 rObject call_message)
  {
    // If we try to call a C++ primitive with a call message, make it
    // look like a strict function call
    if (call_message &&
	(func->kind_get() != object::Object::kind_code
	 || func->value<object::Code>().strict()))
    {
      rObject urbi_args = urbi_call(*this, call_message, SYMBOL(evalArgs));
      foreach (const rObject& arg,
	       urbi_args->value<object::List>())
	args.push_back(arg);
      call_message = 0;
    }

    // Even with call message, there is at least one argument: self.
    assert (!args.empty());
    // If we use a call message, "self" is the only argument.
    assert (!call_message || args.size() == 1);

    // Check if any argument is void
    try
    {
      if (!acceptVoid(func))
      {
	bool first = true;
	foreach (rObject arg, args)
	{
	  if (!first && arg == object::void_class)
	    throw object::WrongArgumentType ("");
	  first = false;
	}
      }
    }
    catch (object::LookupError)
    {
      // Nothing. This happen only once, when invoking the first
      // setSlot that defines Code.acceptVoid to false.
    }

    switch (func->kind_get ())
    {
      case object::Object::kind_primitive:
	current_ =
	  func.unsafe_cast<object::Primitive>()->value_get()(*this, args);
	break;
      case object::Object::kind_delegate:
	current_ =
	  func.unsafe_cast<object::Delegate>()->value_get()
	  ->operator()(*this, args);
	break;
      case object::Object::kind_code:
	current_ = apply_urbi (func, msg, args, call_message);
	break;
      default:
	object::check_arg_count (1, args.size(), "");
	current_ = func;
	break;
    }

    return current_;
  }

  object::rObject
  Runner::apply (const rObject& func, const libport::Symbol msg,
		 const object::rList& args)
  {
    object::objects_type apply_args;
    foreach (rObject arg, args->value_get ())
      apply_args.push_back (arg);
    return apply (func, msg, apply_args);
  }

  void
  Runner::push_evaluated_arguments (object::objects_type& args,
				    const ast::exps_type& ue_args,
				    bool check_void)
  {
    bool tail = false;
    foreach (const ast::Exp* arg, ue_args)
    {
      // Skip target, the first argument.
      if (!tail++)
	continue;
      eval (*arg);
      // Check if any argument is void. This will be checked again in
      // Runner::apply, yet raising exception here gives better
      // location (the argument and not the whole function invocation).
      if (check_void && current_ == object::void_class)
      {
	object::WrongArgumentType e("");
	e.location_set(arg->location_get());
	throw e;
      }

      passert ("argument without a value: " << *arg, current_);
      args.push_back (current_);
    }
  }

  object::rObject
  Runner::build_call_message (const rObject& tgt, const libport::Symbol& msg,
			      const object::objects_type& args)
  {
    rObject res = object::global_class->slot_get(SYMBOL(CallMessage))->clone();

    // Set the sender to be the current self. self must always exist.
    res->slot_set (SYMBOL(sender),
		   locals_->slot_get (SYMBOL(self)));

    // Set the target to be the object on which the function is applied.
    res->slot_set (SYMBOL(target), tgt);

    // Set the name of the message call.
    res->slot_set (SYMBOL(message), object::String::fresh(msg));

    std::list<rObject> largs;
    foreach (const rObject& o, args)
    {
      largs.push_back(o);
    }
    res->slot_set (SYMBOL(args), object::List::fresh(largs));

    // Store the current context in which the arguments must be evaluated.
    res->slot_set (SYMBOL(context), object::Object::make_scope(locals_));

    return res;
  }

  object::rObject
  Runner::build_call_message (const rObject& tgt, const libport::Symbol& msg,
			      const ast::exps_type& args)
  {
    // Build the list of lazy arguments
    object::objects_type lazy_args;

    foreach (ast::Exp* e, args)
    {
      // The target can be unspecified
      if (!e)
      {
	lazy_args.push_back(object::nil_class);
	continue;
      }
      lazy_args.push_back(object::mkLazy(*this, e));
    }

    return build_call_message(tgt, msg, lazy_args);
  }

  Runner::rObject
  Runner::target (const ast::Exp* n, const libport::Symbol& name)
  {
    if (n)
      return eval (*n);
    else
    {
      object::objects_type args;
      args.push_back(object::String::fresh(name));
      try
      {
	return urbi_call(*this, locals_, SYMBOL(target), args);
      }
      catch (object::LookupError&) // No 'target' function
      {
	return locals_;
      }
    }
  }

  void
  Runner::operator() (const ast::Call& e)
  {
    rObject tgt;
    try
    {
      tgt = target(e.args_get().front(), e.name_get());
    }
    PROPAGATE_EXCEPTION(e.location_get(), {});
    assertion(tgt);


    /*---------------------.
    | Decode the message.  |
    `---------------------*/

    // The invoked slot (probably a function).
    rObject val;

    // We may have to run a primitive, or some code.
    try
    {
      // Ask the target for the handler of the message.
      val = tgt->slot_get (e.name_get ());
    }
    PROPAGATE_EXCEPTION(e.location_get(), {});
    assertion(val);


    /*-------------------------.
    | Evaluate the arguments.  |
    `-------------------------*/

    // Gather the arguments, including the target.
    object::objects_type args;
    args.push_back (tgt);

    // Build the call message for non-strict functions, otherwise the
    // evaluated argument list.
    rObject call_message;
    if (val->kind_get () == object::Object::kind_code
	&& !val.unsafe_cast<object::Code> ()->value_get ().strict())
      call_message = build_call_message (tgt, e.name_get(), e.args_get ());
    else
      push_evaluated_arguments (args, e.args_get (), !acceptVoid(val));

    try
    {
      call_stack_.push_back(&e);
      libport::Finally finally;
      finally << boost::bind(&call_stack_type::pop_back, &call_stack_);
      apply (val, e.name_get(), args, call_message);
    }
    PROPAGATE_EXCEPTION(e.location_get(), );

    // Because while returns 0, we can't have a call that returns 0
    // (a function that runs a while for instance).
    // passert ("no value: " << e, current_);
    ECHO (AST(e) << " result: " << *current_);
  }


  void
  Runner::operator() (const ast::Float& e)
  {
    current_ = object::Float::fresh(e.value_get());
  }


  void
  Runner::operator() (const ast::Foreach& e)
  {
    // Evaluate the list attribute, and check its type.
    JECHO ("foreach list", e.list_get());
    operator() (e.list_get());
    try
    {
      TYPE_CHECK(current_, object::List);
    }
    PROPAGATE_EXCEPTION(e.location_get(), {});

    JECHO("foreach body", e.body_get());

    // We need to copy the pointer on the list, otherwise the list will be
    // destroyed when children are visited and current_ is modified.
    object::List::value_type content = current_->value<object::List>();

    // The list of runners launched for each value in the list if the flavor
    // is "&".
    std::vector<scheduler::rJob> runners;
    if (e.flavor_get() == ast::flavor_and)
      runners.reserve(content.size());

    bool first_iteration = true;

    // Iterate on each value.
    foreach (rObject o, content)
    {
      // Define a new local scope for each loop, and set the index.
      rObject locals = object::Object::fresh();
      locals->locals_set(true);
      locals->proto_add(locals_);
      locals->slot_set(e.index_get(), o);

      // for& ... in loop.
      if (e.flavor_get() == ast::flavor_and)
      {
	// Create the new runner and launch it. We create a link so
	// that an error in evaluation will stop other evaluations
	// as well and propagate the exception.
	Runner* new_runner = new Runner(*this, &e.body_get());
	link(new_runner);
	runners.push_back(new_runner->myself_get());
	new_runner->locals_ = locals;
	new_runner->start_job();
      }
      else // for| and for;
      {
	std::swap(locals, locals_);
	libport::Finally finally;
	finally << boost::bind(swap, ref(locals), ref(locals_));

	if (first_iteration)
	  first_iteration = false;
	else
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
	PROPAGATE_EXCEPTION(e.location_get(), {});
      }
    }

    // Wait for all runners to terminate.
    foreach(scheduler::rJob r, runners)
      yield_until_terminated(*r);

    // For the moment return void.
    current_ = object::void_class;
  }

  object::rObject
  Runner::make_code(const ast::Function& e) const
  {
    rObject res = object::Code::fresh(*ast::clone(e));
    // Store the function declaration context. Use make_scope to add
    // an empty object above it, so as variables injected in the
    // context do not appear in the declaration scope.
    res->slot_set(SYMBOL(context), object::Object::make_scope(locals_));
    // Set capturedVars at []. By default, no variables are searched
    // in the context.
    res->slot_set(SYMBOL(capturedVars),
		       object::List::fresh(std::list<object::rObject>()));
    return res;
  }


  void
  Runner::operator() (const ast::Function& e)
  {
    current_ = make_code(e);
  }


  void
  Runner::operator() (const ast::If& e)
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
  Runner::operator() (const ast::List& e)
  {
    typedef std::list<object::rObject> objects;
    typedef std::list<ast::Exp*> exps;
    // list values
    objects values;
    exps::const_iterator i;

    // Evaluate every expression in the list
    for (i = e.value_get ().begin ();
	 i != e.value_get ().end ();
	 ++i)
    {
      operator() (**i);
      values.push_back(current_);
    }
    current_ = object::List::fresh(values);
    ECHO ("result: " << *current_);
  }


  void
  Runner::operator() (const ast::Message& e)
  {
    send_message_(e.tag_get(), e.text_get());
  }


// Forward flow exceptions up to the top-level, and handle them
// there.  Makes only sense in a Nary.
#define CATCH_FLOW_EXCEPTION(Type, Keyword, Error)              \
  catch (Type flow_exception)                                   \
  {                                                             \
    if (e.toplevel_get ())                                      \
    {                                                           \
      object::PrimitiveError error(Keyword, Error);             \
      propagate_error_(error, flow_exception.location_get());   \
      throw error;						\
      continue;                                                 \
    }                                                           \
    else                                                        \
      throw;							\
  }

  void
  Runner::operator() (const ast::Nary& e)
  {
    // List of runners for Stmt flavored by a comma.
    std::list<scheduler::rJob> runners;

    // In case we're empty.
    current_ = object::void_class;

    bool first_iteration = true;
    foreach (ast::Exp* i, e.children_get ())
    {
      // Allow some time to pass before we execute what follows.  If
      // we don't do this, the ;-operator would act almost like the
      // |-operator because it would always start to execute its RHS
      // immediately. However, we don't want to do it before the first
      // statement or if we only have one statement in the scope.
      if (first_iteration)
	first_iteration = false;
      else
	YIELD ();

      current_.reset ();
      JECHO ("child", i);

      if (dynamic_cast<ast::Stmt*>(i) &&
	  dynamic_cast<ast::Stmt*>(i)->flavor_get() == ast::flavor_comma)
      {
	// The new runners are attached to the same tags as we are
	Runner* subrunner = new Runner(*this, i);
	runners.push_back(subrunner->myself_get ());
	subrunner->start_job ();
      }
      else
      {
	// If at toplevel, stop and print errors
	try
	{
	  // Rewrite flow error if we are at toplevel
	  try
	  {
	    // Propagate potential errors
	    try
	    {
	      operator() (i);
	    }
	    PROPAGATE_EXCEPTION(e.location_get(), )
          }
	  CATCH_FLOW_EXCEPTION(ast::BreakException,
			       "break", "outside a loop")
	  CATCH_FLOW_EXCEPTION(ast::ReturnException,
			       "return", "outside a function")

	  if (e.toplevel_get () && current_.get ())
	  {
	    try
	    {
	      ECHO ("toplevel: returning a result to the connection.");
	      lobby_->value_get ().connection.new_result (current_);
	      current_.reset ();
	    }
	    catch (std::exception &ke)
	    {
	      std::cerr << "Exception when printing result: " << ke.what() << std::endl;
	    }
	    catch (...)
	    {
	      std::cerr << "Unknown exception when printing result\n";
	    }
	  }
	}
	catch (object::UrbiException& ue)
	{
	  if (e.toplevel_get())
	    show_error_(ue);
	  else
	    throw;
	}
        CATCH_FLOW_EXCEPTION(ast::BreakException,
			     "break", "outside a loop")
        CATCH_FLOW_EXCEPTION(ast::ReturnException,
                             "return", "outside a function")

      }
    }

    // FIXME: We use toplevel_get here for two different things:
    //   1. Are we at the top-level of the current runner? (primary expression)
    //   2. Are we directly below the current connection *and* the top-level
    //      of the current runner?
    // Point "1" should influence ast freeing, while point "2" should influence
    // result printing.

    // If the Nary is not the toplevel one, all subrunners must be finished when
    // the runner exits the Nary node.
    // FIXME: There is a memory leak if the Nary is a toplevel one.
    if (!e.toplevel_get ())
    {
      foreach(scheduler::rJob r, runners)
	yield_until_terminated(*r);
    }

    // FIXME: We violate the constness, but anyway this should not
    // be done here.  Not to mention the leaks, as we don't delete the
    // AST here.
    if (e.toplevel_get ())
      const_cast<ast::Nary&>(e).clear();
  }

  void
  Runner::run_at_exit (object::rObject& scope)
  {
    if (object::rObject atexit = scope->own_slot_get (SYMBOL (atexit)))
    {
      try
      {
	TYPE_CHECK (atexit, object::List);
      }
      catch (...)
      {
	// Bad type, return immediately.
	return;
      }
      object::rList atexit_funcs = atexit.unsafe_cast<object::List> ();
      object::objects_type args;
      args.push_back (scope->slot_get (SYMBOL (self)));
      rObject saved_current = current_;
      foreach (const rObject& func, atexit_funcs->value_get ())
      {
	try
	{
	  // FIXME: We have no way to find the actual message name here.
	  apply (func, SYMBOL(atExit), args);
	}
	catch (...)
	{
	  // Ignore errors in atexit blocks but execute further calls
	}
      }
      current_ = saved_current;
    }
  }

  void
  Runner::operator() (const ast::Noop&)
  {
    current_ = object::void_class;
  }

  void
  Runner::operator() (const ast::Object& e)
  {
    rObject res = object::Object::fresh();
    foreach (const ast::Slot& s, e.slots_get())
    {
      try
      {
	rObject val = eval(s.value_get());
	if (s.name_get() == SYMBOL(protos))
	{
	  // protos should always point to a list.
	  TYPE_CHECK(val, object::List);
	  res->protos_set(val.unsafe_cast<object::List>()->value_get());
	}
	else
	  res->slot_set(s.name_get(), val);
      }
      PROPAGATE_EXCEPTION(s.location_get(), {});
    }
    current_ = res;
  }


  void
  Runner::operator() (const ast::Pipe& e)
  {
    // lhs
    JECHO ("lhs", e.lhs_get ());
    operator() (e.lhs_get());

    // rhs:  start the execution immediately.
    JECHO ("rhs", e.rhs_get ());
    operator() (e.rhs_get());
  }


  void
  Runner::operator() (const ast::Scope& e)
  {
    // To each scope corresponds a "locals" object which stores the
    // local variables.  It points to the previous current scope to
    // implement lexical scoping.
    rObject locals;
    rObject target;

    if (e.target_get())
    {
      target = eval(*e.target_get());
      locals = object::Object::make_do_scope(locals_, target);
    }
    else
      locals = object::Object::make_scope(locals_);

    bool was_non_interruptible = non_interruptible_get ();
    std::swap(locals, locals_);
    libport::Finally finally;
    finally << boost::bind(swap, ref(locals), ref(locals_));
    try
    {
      try
      {
	super_type::operator()(e.body_get());
	run_at_exit (locals_);
      }
      catch (...)
      {
	run_at_exit (locals_);
	throw;
      }
    }
    PROPAGATE_EXCEPTION(e.location_get(),
			{
			  non_interruptible_set (was_non_interruptible);
			});
    if (target)
      current_ = target;
  }

  void
  Runner::operator() (const ast::Stmt& e)
  {
    JECHO ("expression", e.expression_get ());
    operator() (e.expression_get());
  }

  void
  Runner::operator() (const ast::String& e)
  {
    current_ = object::String::fresh(libport::Symbol(e.value_get()));
  }

  void
  Runner::operator() (const ast::Tag& t)
  {
    eval_tag (t.exp_get ());
  }

  void
  Runner::operator() (const ast::TaggedStmt& t)
  {
    try
    {
      push_tag (extract_tag (eval (t.tag_get ())));
      libport::Finally finally;
      finally << boost::bind(&Runner::pop_tag, this);
      // If the latest tag causes us to be frozen or blocked, let the
      // scheduler handler this properly to avoid duplicating the
      // logic.
      if (frozen () || blocked ())
	yield ();
      eval (t.exp_get ());
    }
    catch (scheduler::BlockedException& e)
    {
      // If we have been blocked, restore the tags list. We may have
      // been blocked because of another tag in our stack, but we are
      // not allowed to pop them ourselves. So check if we are still
      // blocked and go up one level in this case.
      current_.reset ();
      if (blocked ())
	throw;
      // Execution will go on as planned
      return;
    }
    PROPAGATE_EXCEPTION(t.location_get(), );
  }

  object::rObject
  Runner::eval_tag (const ast::Exp& e)
  {
    try {
      // Try to evaluate e as a normal expression.
      return eval (e);
    }
    catch (object::LookupError &ue)
    {
      // We got a lookup error. It means that we have to automatically
      // create the tag. In this case, we only accept k1 style tags,
      // i.e. chains of identifiers, excluding function calls.
      // The reason to do that is:
      //   - we do not want to mix k1 non-declared syntax with k2
      //     clean syntax for tags
      //   - we have no way to know whether the lookup error arrived
      //     in a function call or during the direct resolution of
      //     the name

      // Tag represents the top level tag
      rObject toplevel = object::tag_class;
      rObject base = toplevel;
      foreach (const libport::Symbol* element, decompose_tag_chain (&e))
      {
	// Check whether the concerned level in the chain already
	// exists.
	rObject owner = base->slot_locate (*element);
	if (owner)
	  base = owner->own_slot_get (*element);
	else
	{
	  // We have to create a new tag, which will be attached
	  // to the upper level (hierarchical tags, implicitly
	  // rooted by Tag).
	  rObject new_tag = toplevel->clone();
	  object::objects_type args;
	  args.push_back (new_tag);
	  args.push_back (object::String::fresh (*element));
	  args.push_back (base);
	  apply (toplevel->own_slot_get (SYMBOL (init)), SYMBOL(init), args);
	  base->slot_set (*element, new_tag);
	  base = new_tag;
	}
      }

      return base;
    }
  }

  void
  Runner::operator() (const ast::Throw& e)
  {
    switch (e.kind_get())
    {
      case ast::break_exception:
	throw ast::BreakException(e.location_get());

      case ast::return_exception:
	if (e.value_get())
	  operator() (*e.value_get());
	else
	  current_.reset();
	throw ast::ReturnException(e.location_get(), current_);
    }
  }


  void
  Runner::operator() (const ast::While& e)
  {
    bool first_iteration = true;
    // Evaluate the test.
    while (true)
    {
      if (first_iteration)
	first_iteration = false;
      else
	MAYBE_YIELD (e.flavor_get());
      JECHO ("while test", e.test_get ());
      operator() (e.test_get());
      if (!IS_TRUE(current_))
	break;

      JECHO ("while body", e.body_get ());

      try
      {
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
    current_ = object::void_class;
  }


} // namespace runner
