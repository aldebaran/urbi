/**
 ** \file runner/interpreter.cc
 ** \brief Implementation of runner::Interpreter.
 */

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include <deque>

#include <boost/range/iterator_range.hpp>

#include <libport/finally.hh>
#include <libport/foreach.hh>
#include <libport/symbol.hh>

#include "kernel/exception.hh"
#include "kernel/uconnection.hh"

#include "ast/new-clone.hh"
#include "ast/print.hh"

#include "object/atom.hh"
#include "object/global-class.hh"
#include "object/idelegate.hh"
#include "object/lazy.hh"
#include "object/object.hh"
#include "object/symbols.hh"
#include "object/tag-class.hh"
#include "object/urbi-exception.hh"
#include "object/flow-exception.hh"

#include "runner/interpreter.hh"
#include "parser/uparser.hh"

namespace runner
{
  using boost::bind;
  using libport::Finally;

/// Address of \a Interpreter seen as a \c Job (Interpreter has multiple inheritance).
#define JOB(Interpreter) static_cast<scheduler::Job*> (Interpreter)

/// Address of \c this seen as a \c Job (Interpreter has multiple inheritance).
#define ME JOB (this)

#define AST(Ast)                                \
  (Ast).location_get ()                         \
  << libport::incendl                           \
  << "{{{"                                      \
  << libport::incendl                           \
  << Ast                                        \
  << libport::decendl                           \
  << "}}}"                                      \
  << libport::decindent

/// Job echo.
#define JECHO(Title, Content)                           \
  ECHO ("job " << ME << ", " Title ": " << Content)

/// Job & astecho.
#define JAECHO(Title, Ast)                      \
  JECHO (Title, AST(Ast))

/* Yield and trace. */
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

/// Catch exceptions, execute Code, then display the error if not
/// already done, and rethrow it. Also execute Code if no exception
/// caught.
///
/// Since this is a macro, we are likely to capture identifiers.  In
/// particular, don't use "e" to bind the name of the caught
/// exceptions, since that's the name used by the formal argument of
/// the visit methods.
#define PROPAGATE_EXCEPTION(Node)			\
  catch (object::UrbiException& propagate_exception)    \
  {							\
    current_.reset();					\
    propagate_error_(propagate_exception,               \
                     (Node).location_get());            \
    throw;						\
  }							\
  catch (object::FlowException&)                        \
  {							\
    current_.reset();					\
    throw;						\
  }							\
  catch (scheduler::SchedulerException&)                \
  {							\
    current_.reset();					\
    throw;						\
  }							\
  catch (kernel::exception& propagate_exception)        \
  {							\
    std::cerr << "Unexpected exception propagated: "	\
	      << propagate_exception.what()             \
              << std::endl;                             \
    throw;						\
  }							\
  catch (std::exception& propagate_exception)		\
  {							\
    std::cerr << "Unexpected exception propagated: "	\
	      << propagate_exception.what()             \
              << std::endl;                             \
    throw;						\
  }							\
  catch (...)						\
  {							\
    std::cerr << "Unknown exception propagated\n";	\
    throw;						\
  }

  // Helper to generate a function that swaps two rObjects.
  inline
  boost::function0<void>
  swap(Interpreter::rObject& lhs, Interpreter::rObject& rhs)
  {
    // Strangely, this indirection is needed
    void (*f) (Interpreter::rObject&, Interpreter::rObject&) =
      std::swap<Interpreter::rObject>;
    return boost::bind(f, boost::ref(lhs), boost::ref(rhs));
  }

  // This function takes an expression and attempts to decompose it
  // into a list of identifiers, and a potential expression that
  // represents the owner of the new tag.
  typedef std::deque<const ast::Call*> tag_chain_type;
  static
  std::pair<const ast::Exp*, tag_chain_type>
  decompose_tag_chain (const ast::Exp* e)
  {
    tag_chain_type res;
    while (!e->implicit())
    {
      const ast::Call* c = dynamic_cast<const ast::Call*>(e);
      if (!c)
	throw object::ImplicitTagComponentError(e->location_get());
      if (c->args_get().size() > 1)
        return std::make_pair(c, res);
      res.push_front (c);
      e = &c->args_get().front();
    }
    assert (!res.empty ());
    return std::make_pair((const ast::Exp*)0, res);
  }


  Interpreter::Interpreter (rLobby lobby,
			    rObject locals,
			    scheduler::Scheduler& sched,
			    const ast::Ast* ast,
			    bool free_ast_after_use,
			    const libport::Symbol& name)
    : Interpreter::super_type(),
      Runner(lobby, sched, name),
      ast_(ast),
      free_ast_after_use_(free_ast_after_use),
      code_(0),
      current_(0),
      locals_(locals)
  {
    init();
  }

  Interpreter::Interpreter(const Interpreter& model, rObject code,
			   const libport::Symbol& name)
    : Interpreter::super_type(),
      Runner(model, name),
      ast_(0),
      free_ast_after_use_(false),
      code_(code),
      current_(0),
      locals_(model.locals_)
  {
    init();
  }

  Interpreter::Interpreter(const Interpreter& model,
			   const ast::Ast* ast,
			   bool free_ast_after_use,
			   const libport::Symbol& name)
    : Interpreter::super_type (),
      Runner(model, name),
      ast_(ast),
      free_ast_after_use_(free_ast_after_use),
      code_(0),
      current_(0),
      locals_(model.locals_)
  {
  }

  Interpreter::~Interpreter ()
  {
  }

  void
  Interpreter::init()
  {
    if (!locals_)
      locals_ = object::Object::make_method_scope(lobby_);
    // If the lobby has a slot connectionTag, push it
    rObject connection_tag = lobby_->slot_locate(SYMBOL(connectionTag));
    if (connection_tag)
      push_tag(extract_tag(connection_tag->slot_get(SYMBOL(connectionTag))));
  }

  void
  Interpreter::show_error_ (const object::UrbiException& ue)
  {
    std::ostringstream o;
    o << "!!! " << ue.location_get () << ": " << ue.what ();
    send_message_ ("error", o.str ());
    show_backtrace(ue.backtrace_get(), "error");
  }

  void
  Interpreter::propagate_error_ (object::UrbiException& ue, const ast::loc& l)
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
  Interpreter::work ()
  {
    assert (ast_ || code_);
    JAECHO ("starting evaluation of AST: ", *ast_);
    if (ast_)
    {
      Finally finally;
      if (free_ast_after_use_)
	finally << bind(&operator delete, const_cast<ast::Ast*>(ast_));
      operator()(*ast_);
    }
    else
    {
      object::objects_type args;
      args.push_back(locals_);
      apply(code_, SYMBOL(task), args);
    }
  }

  /*----------------.
  | Regular visit.  |
  `----------------*/

  void
  Interpreter::visit (const ast::And& e)
  {
    // lhs will be evaluated in another Interpreter, while rhs will be evaluated
    // in this one. We will be the new runner parent, as we have the same
    // tags.

    JAECHO ("lhs", e.lhs_get ());
    scheduler::rJob lhs = new Interpreter (*this, ast::new_clone(e.lhs_get ()), true);

    // Propagate errors between left-hand side and right-hand side runners.
    link (lhs);

    lhs->start_job ();

    JAECHO ("rhs", e.rhs_get ());
    eval (e.rhs_get ());

    // Wait for lhs to terminate
    yield_until_terminated (*lhs);

    current_ = object::void_class;
  }

  // Apply a function written in Urbi.
  object::rObject
  Interpreter::apply_urbi (const rObject& func,
                           const libport::Symbol& msg,
                           const object::objects_type& args,
                           rObject call_message)
  {
    // The called function.
    ast::Code& fn = func.unsafe_cast<object::Code> ()->value_get ();
    // Whether it's an explicit closure
    bool closure = dynamic_cast<ast::Closure*>(&fn);

    // If the function is lazy and there's no call message, forge
    // one. This happen when a lazy function is invoked with eval, for
    // instance.
    if (!fn.strict() && !call_message)
    {
      object::objects_type lazy_args;
      foreach (const rObject& o, args)
	lazy_args.push_back(mkLazy(*this, o));
      call_message = build_call_message(args[0], msg, lazy_args);
    }

    // Create the function's outer scope, with the first argument as
    // 'self'. The inner scope will be created when executing ()
    // on ast::Scope.
    rObject scope;
    if (closure)
      // For closures, use the context as parent scope.
      scope = func->slot_get(SYMBOL(context));
    else
    {
      scope = object::Object::make_method_scope(args.front());
      scope->slot_set(SYMBOL(code), func);
    }
    // If this is a strict function, check the arity and bind the formal
    // arguments. Otherwise, bind the call message.
    if (fn.strict())
    {
      const ast::symbols_type& formals = *fn.formals_get();
      object::check_arg_count (formals.size() + 1, args.size(), msg.name_get());
      // Effective (evaluated) argument iterator.
      // Skip "self" which has already been handled.
      object::objects_type::const_iterator ei = ++args.begin();
      foreach (const libport::Symbol s, formals)
	scope->slot_set (s, *ei++);
    }
    else
      if (!closure)
        scope->slot_set (SYMBOL(call), call_message);

    // Change the current context and call. But before, check that we
    // are not exhausting the stack space, for example in an infinite
    // recursion.
    std::swap(scope, locals_);
    Finally finally(swap(scope, locals_));

    check_stack_space ();

    try
    {
      current_ = eval (*fn.body_get());
    }
    catch (object::BreakException& be)
    {
      object::PrimitiveError error("break", "outside a loop");
      propagate_error_(error, be.location_get());
      throw error;
    }
    catch (object::ReturnException& re)
    {
      current_ = re.result_get();
      if (!current_)
	current_ = object::void_class;
    }
    PROPAGATE_EXCEPTION(fn);

    return current_;
  }

  namespace
  {
    // Helper to determine whether a function accepts void parameters
    static inline bool
    acceptVoid(object::rObject f)
    {
      assertion(f);
      try
      {
	return object::is_true(f->slot_get(SYMBOL(acceptVoid)));
      }
      catch (object::LookupError&)
      {
	// acceptVoid is undefined. Refuse void parameter by default.
	return false;
      }
    }
  }

  object::rObject
  Interpreter::apply (const rObject& func,
		      const libport::Symbol msg,
		      object::objects_type args,
		      rObject call_message)
  {
    precondition(func);

    // If we try to call a C++ primitive with a call message, make it
    // look like a strict function call
    if (call_message &&
	(func->kind_get() != object::object_kind_code
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
    if (!acceptVoid(func))
    {
      bool first = true;
      foreach (const rObject& arg, args)
      {
	if (!first && arg == object::void_class)
	  throw object::WrongArgumentType (msg);
	first = false;
      }
    }

    switch (func->kind_get ())
    {
      case object::object_kind_primitive:
	current_ =
	  func.unsafe_cast<object::Primitive>()->value_get()(*this, args);
	break;
      case object::object_kind_delegate:
	current_ =
	  func.unsafe_cast<object::Delegate>()
          ->value_get()
	  ->operator()(*this, args);
	break;
      case object::object_kind_code:
	current_ = apply_urbi (func, msg, args, call_message);
	break;
      default:
	object::check_arg_count (1, args.size(), msg.name_get());
	current_ = func;
	break;
    }

    return current_;
  }

  object::rObject
  Interpreter::apply (const rObject& func, const libport::Symbol msg,
		      const object::rList& args)
  {
    object::objects_type apply_args;
    foreach (const rObject arg, args->value_get ())
      apply_args.push_back (arg);
    return apply (func, msg, apply_args);
  }

  void
  Interpreter::push_evaluated_arguments (object::objects_type& args,
					 const ast::exps_type& ue_args,
					 bool check_void)
  {
    bool tail = false;
    foreach (const ast::Exp& arg, ue_args)
    {
      // Skip target, the first argument.
      if (!tail++)
	continue;
      eval (arg);
      // Check if any argument is void. This will be checked again in
      // Interpreter::apply, yet raising exception here gives better
      // location (the argument and not the whole function invocation).
      if (check_void && current_ == object::void_class)
      {
	object::WrongArgumentType e("");
	e.location_set(arg.location_get());
	throw e;
      }

      passert (arg, current_);
      args.push_back (current_);
    }
  }

  object::rObject
  Interpreter::build_call_message (const rObject& tgt,
                                   const libport::Symbol& msg,
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

    object::List::value_type largs;
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
  Interpreter::build_call_message (const rObject& tgt, const libport::Symbol& msg,
				   const ast::exps_type& args)
  {
    // Build the list of lazy arguments
    object::objects_type lazy_args;
    boost::sub_range<const ast::exps_type> range(args);
    // The target can be unspecified.
    if (args.front().implicit())
    {
      lazy_args.push_back(object::nil_class);
      range = make_iterator_range(range, 1, 0);
    }
    foreach (const ast::Exp& e, range)
      lazy_args.push_back(object::mkLazy(*this, e));

    return build_call_message(tgt, msg, lazy_args);
  }

  void
  Interpreter::visit (const ast::Call& e)
  {
    try
    {
      // The invoked slot (probably a function).
      const ast::Exp& ast_tgt = e.args_get().front();
      rObject tgt = ast_tgt.implicit() ? locals_ : eval(ast_tgt);
      assertion(tgt);
      rObject val = tgt->slot_get(e.name_get());
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
      if (val->kind_get () == object::object_kind_code
          && !val.unsafe_cast<object::Code> ()->value_get ().strict())
        call_message = build_call_message (tgt, e.name_get(), e.args_get ());
      else
        push_evaluated_arguments (args, e.args_get (), !acceptVoid(val));

      call_stack_.push_back(&e);
      Finally finally(bind(&call_stack_type::pop_back, &call_stack_));
      apply (val, e.name_get(), args, call_message);
    }
    PROPAGATE_EXCEPTION(e);

    // Because while returns 0, we can't have a call that returns 0
    // (a function that runs a while for instance).
    // passert (e, current_);
    // ECHO (AST(e) << " result: " << *current_);
  }


  void
  Interpreter::visit (const ast::Float& e)
  {
    current_ = object::Float::fresh(e.value_get());
  }


  void
  Interpreter::visit (const ast::Foreach& e)
  {
    // Evaluate the list attribute, and check its type.
    JAECHO ("foreach list", e.list_get());
    operator() (e.list_get());
    try
    {
      TYPE_CHECK(current_, object::List);
    }
    PROPAGATE_EXCEPTION(e);

    JAECHO("foreach body", e.body_get());

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
    foreach (const rObject& o, content)
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
	Interpreter* new_runner = new Interpreter(*this, new_clone(e.body_get()), true);
	link(new_runner);
	runners.push_back(new_runner);
	new_runner->locals_ = locals;
	new_runner->start_job();
      }
      else // for| and for;
      {
	std::swap(locals, locals_);
	Finally finally(swap(locals, locals_));

	if (first_iteration)
	  first_iteration = false;
	else
	  MAYBE_YIELD(e.flavor_get());

	try
	{
	  operator() (e.body_get());
	}
	catch (object::BreakException&)
	{
	  break;
	}
	// Restore previous locals_, even if an exception was thrown.
	PROPAGATE_EXCEPTION(e);
      }
    }

    // Wait for all runners to terminate.
    foreach(const scheduler::rJob& r, runners)
      yield_until_terminated(*r);

    // For the moment return void.
    current_ = object::void_class;
  }

  object::rObject
  Interpreter::make_code(const ast::Code& e) const
  {
    rObject res = object::Code::fresh(*new_clone(e));
    // Store the function declaration context. Use make_scope to add
    // an empty object above it, so as variables injected in the
    // context do not appear in the declaration scope.
    res->slot_set(SYMBOL(context), locals_);
    return res;
  }


  void
  Interpreter::visit (const ast::Function& e)
  {
    current_ = make_code(e);
  }

  void
  Interpreter::visit (const ast::Closure& e)
  {
    current_ = make_code(e);
  }


  void
  Interpreter::visit (const ast::If& e)
  {
    // Evaluate the test.
    JAECHO ("test", e.test_get ());
    operator() (e.test_get());

    if (object::is_true(current_))
    {
      JAECHO ("then", e.thenclause_get ());
      operator() (e.thenclause_get());
    }
    else
    {
      JAECHO ("else", e.elseclause_get ());
      operator() (e.elseclause_get());
    }
  }


  void
  Interpreter::visit (const ast::List& e)
  {
    object::List::value_type res;
    // Evaluate every expression in the list
    foreach (const ast::Exp& c, e.value_get())
      res.push_back(eval(c));
    current_ = object::List::fresh(res);
    //ECHO ("result: " << *current_);
  }


  void
  Interpreter::visit (const ast::Message& e)
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
    }                                                           \
    else                                                        \
      throw;							\
  }

  void
  Interpreter::visit (const ast::Nary& e)
  {
    // List of runners for Stmt flavored by a comma.
    std::list<scheduler::rJob> runners;

    // In case we're empty.
    current_ = object::void_class;

    bool tail = false;
    foreach (const ast::Exp& c, e.children_get())
    {
      // Allow some time to pass before we execute what follows.  If
      // we don't do this, the ;-operator would act almost like the
      // |-operator because it would always start to execute its RHS
      // immediately. However, we don't want to do it before the first
      // statement or if we only have one statement in the scope.
      if (tail++)
	YIELD ();

      current_.reset ();
      JAECHO ("child", c);

      if (dynamic_cast<const ast::Stmt*>(&c) &&
	  dynamic_cast<const ast::Stmt*>(&c)->flavor_get() == ast::flavor_comma)
      {
	// The new runners are attached to the same tags as we are.
	Interpreter* subrunner =
	  new Interpreter(*this, new_clone(c), true);
	runners.push_back(subrunner);
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
	      operator() (c);
	    }
	    PROPAGATE_EXCEPTION(e);
          }
	  CATCH_FLOW_EXCEPTION(object::BreakException,
			       "break", "outside a loop")
	  CATCH_FLOW_EXCEPTION(object::ReturnException,
			       "return", "outside a function")

	  if (e.toplevel_get () && current_.get ())
	  {
	    try
	    {
	      assertion(current_);
	      ECHO("toplevel: returning a result to the connection.");
	      lobby_->value_get ().connection.new_result (current_);
              current_.reset ();
	    }
	    catch (std::exception &ke)
	    {
	      std::cerr << "Exception when printing result: "
                        << ke.what() << std::endl;
	    }
	    catch (object::UrbiException& e)
	    {
	      show_error_(e);
	    }
	    catch (...)
	    {
	      std::cerr << "Unknown exception when printing result"
                        << std::endl;
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
        CATCH_FLOW_EXCEPTION(object::BreakException,
			     "break", "outside a loop")
        CATCH_FLOW_EXCEPTION(object::ReturnException,
                             "return", "outside a function")
      }
    }

    // If the Nary is not the toplevel one, all subrunners must be finished when
    // the runner exits the Nary node.
    // FIXME: There is a memory leak if the Nary is a toplevel one.
    if (!e.toplevel_get ())
    {
      foreach(const scheduler::rJob& r, runners)
	yield_until_terminated(*r);
    }

    // FIXME: We violate the constness, but anyway this should not
    // be done here.  Not to mention the leaks, as we don't delete the
    // AST here.
    if (e.toplevel_get ())
      const_cast<ast::Nary&>(e).clear();
  }

  void
  Interpreter::visit (const ast::Noop&)
  {
    current_ = object::void_class;
  }


  void
  Interpreter::visit (const ast::Pipe& e)
  {
    // lhs
    JAECHO ("lhs", e.lhs_get ());
    operator() (e.lhs_get());

    // rhs:  start the execution immediately.
    JAECHO ("rhs", e.rhs_get ());
    operator() (e.rhs_get());
  }

  namespace
  {
    static
    inline
    boost::function0<void>
    non_interruptible_set(Interpreter* o, bool& value)
    {
      // Strangely, this indirection is needed
      return boost::bind(&scheduler::Job::non_interruptible_set,
                         o, boost::ref(value));
    }
  }

  void
  Interpreter::visit (const ast::AbstractScope& e, rObject locals)
  {
    bool was_non_interruptible = non_interruptible_get ();
    std::swap(locals, locals_);
    Finally finally;
    finally << swap(locals, locals_)
            << runner::non_interruptible_set(this, was_non_interruptible);
    try
    {
      super_type::operator()(e.body_get());
    }
    PROPAGATE_EXCEPTION(e);
  }

  void
  Interpreter::visit (const ast::Scope& e)
  {
    visit (static_cast<const ast::AbstractScope&>(e),
           object::Object::make_scope(locals_));
  }

  void
  Interpreter::visit (const ast::Do& e)
  {
    rObject tgt = eval(e.target_get());
    visit (static_cast<const ast::AbstractScope&>(e),
           object::Object::make_method_scope(tgt, locals_));
    // This is arguable. Do, just like Scope, should maybe return
    // their last inner value.
    current_ = tgt;
  }

  void
  Interpreter::visit (const ast::Stmt& e)
  {
    JAECHO ("expression", e.expression_get ());
    operator() (e.expression_get());
  }

  void
  Interpreter::visit (const ast::String& e)
  {
    current_ = object::String::fresh(libport::Symbol(e.value_get()));
  }

  void
  Interpreter::visit (const ast::Tag& t)
  {
    eval_tag (t.exp_get ());
  }

  void
  Interpreter::visit (const ast::TaggedStmt& t)
  {
    try
    {
      push_tag (extract_tag (eval (t.tag_get ())));
      Finally finally(bind(&Interpreter::pop_tag, this));
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
      if (blocked ())
	throw;
      // Execution will go on as planned, but the interrupted expression
      // will evaluate to void.
      current_ = object::void_class;
      return;
    }
    PROPAGATE_EXCEPTION(t);
  }

  object::rObject
  Interpreter::eval_tag (const ast::Exp& e)
  {
    try {
      // Try to evaluate e as a normal expression.
      return eval(e);
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
      std::pair<const ast::Exp*, tag_chain_type> res = decompose_tag_chain (&e);
      // If the left part of the implicit tag is a call with argument,
      // evaluate it to find the owner. Otherwise, store the new tag
      // as a local variable.
      //
      // FIXME: This is naive. Perform a real setSlot.
      rObject where = res.first ? eval(*res.first) : locals_;
      // If it is a tag, consider it as the parent of the new tag as well.
      rObject base = is_a(where, object::tag_class) ? where : toplevel;
      tag_chain_type chain = res.second;
      foreach (const ast::Call* element, chain)
      {
	// Check whether the concerned level in the chain already
	// exists.
	if (rObject owner = base->slot_locate (element->name_get()))
	  base = owner->own_slot_get (element->name_get());
	else
	{
	  // We have to create a new tag, which will be attached
	  // to the upper level (hierarchical tags, implicitly
	  // rooted by Tag).
	  rObject new_tag = toplevel->clone();
	  object::objects_type args;
	  args.push_back (new_tag);
	  args.push_back (object::String::fresh (element->name_get()));
	  args.push_back (base);
	  apply (toplevel->own_slot_get (SYMBOL (init)), SYMBOL(init), args);
	  where->slot_set (element->name_get(), new_tag);
	  base = where = new_tag;
	}
      }

      return base;
    }
  }

  void
  Interpreter::visit (const ast::Throw& e)
  {
    switch (e.kind_get())
    {
      case ast::Throw::exception_break:
	throw object::BreakException(e.location_get());

      case ast::Throw::exception_return:
	if (e.value_get())
	  operator() (*e.value_get());
	else
	  current_.reset();
	throw object::ReturnException(e.location_get(), current_);
    }
  }


  void
  Interpreter::visit (const ast::While& e)
  {
    bool first_iteration = true;
    // Evaluate the test.
    while (true)
    {
      if (first_iteration)
	first_iteration = false;
      else
	MAYBE_YIELD (e.flavor_get());
      JAECHO ("while test", e.test_get ());
      operator() (e.test_get());
      if (!object::is_true(current_))
	break;

      JAECHO ("while body", e.body_get ());

      try
      {
	operator() (e.body_get());
      }
      catch (object::BreakException&)
      {
	// FIXME: Fix for flavor "," and "&".
	if (e.flavor_get() == ast::flavor_semicolon ||
	    e.flavor_get() == ast::flavor_pipe)
	  break;
      };
    }
    current_ = object::void_class;
  }

  void
  Interpreter::show_backtrace(const call_stack_type& bt, const std::string& chan)
  {
    foreach (const ast::Call* c,
             boost::make_iterator_range(boost::rbegin(bt),
                                        boost::rend(bt)))
    {
      std::ostringstream o;
      o << "!!!    called from: " << c->location_get () << ": " << c->name_get ();
      send_message_(chan, o.str());
    }
  }

  void
  Interpreter::show_backtrace(const std::string& chan)
  {
    show_backtrace(call_stack_, chan);
  }

  Interpreter::backtrace_type
  Interpreter::backtrace_get() const
  {
    backtrace_type res;
    foreach (const ast::Call* c, call_stack_)
    {
      std::ostringstream o;
      o << c->location_get();
      res.push_back(std::make_pair(c->name_get().name_get(), o.str()));
    }
    return res;
  }

} // namespace runner
