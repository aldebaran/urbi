/**
 ** \file runner/interpreter.cc
 ** \brief Implementation of runner::Interpreter.
 */

//#define ENABLE_DEBUG_TRACES
#include <libport/compiler.hh>

#include <algorithm>
#include <deque>

#include <boost/range/iterator_range.hpp>

#include <libport/finally.hh>
#include <libport/foreach.hh>
#include <libport/symbol.hh>

#include <kernel/exception.hh>
#include <kernel/uconnection.hh>

#include <ast/declarations-type.hh>
#include <ast/exps-type.hh>
#include <ast/print.hh>

#include <object/atom.hh>
#include <object/global-class.hh>
#include <object/idelegate.hh>
#include <object/lazy.hh>
#include <object/object.hh>
#include <object/symbols.hh>
#include <object/tag-class.hh>
#include <object/urbi-exception.hh>
#include <object/flow-exception.hh>

#include <runner/interpreter.hh>
#include <parser/uparser.hh>

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
  // into a list of identifiers.
  typedef std::deque<libport::Symbol> tag_chain_type;
  static
  tag_chain_type
  decompose_tag_chain (ast::rConstExp e)
  {
    tag_chain_type res;
    while (!e->implicit())
    {
      ast::rConstCall c = e.unsafe_cast<const ast::Call>();
      if (!c || c->arguments_get())
        throw object::ImplicitTagComponentError(e->location_get());
      res.push_front (c->name_get());
      e = c->target_get();
    }
    return res;
  }




  /*--------------.
  | Interpreter.  |
  `--------------*/


  Interpreter::Interpreter (rLobby lobby,
			    scheduler::Scheduler& sched,
			    ast::rConstAst ast,
			    const libport::Symbol& name)
    : Interpreter::super_type(),
      Runner(lobby, sched, name),
      ast_(ast),
      code_(0),
      current_(0),
      locals_(object::Object::make_method_scope(lobby))
  {
    scope_tags_.push_back(0);
    init();
  }

  Interpreter::Interpreter(const Interpreter& model, rObject code,
			   const libport::Symbol& name)
    : Interpreter::super_type(),
      Runner(model, name),
      ast_(0),
      code_(code),
      current_(0),
      locals_(model.locals_)
  {
    scope_tags_.push_back(0);
    init();
  }

  Interpreter::Interpreter(const Interpreter& model,
			   ast::rConstAst ast,
			   const libport::Symbol& name)
    : Interpreter::super_type (),
      Runner(model, name),
      ast_(ast),
      code_(0),
      current_(0),
      locals_(model.locals_)
  {
    scope_tags_.push_back(0);
  }

  Interpreter::~Interpreter ()
  {
  }

  void
  Interpreter::init()
  {
    // If the lobby has a slot connectionTag, push it
    rObject connection_tag = lobby_->slot_locate(SYMBOL(connectionTag));
    if (connection_tag)
      push_tag(extract_tag(connection_tag->slot_get(SYMBOL(connectionTag))));
  }

  void
  Interpreter::show_error_ (const object::UrbiException& ue)
  {
    std::ostringstream o;
    o << "!!! " << ue.location_get () << ": " << ue.what () << std::endl;
    send_message("error", o.str ());
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
      operator()(ast_);
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
  Interpreter::operator() (ast::rConstAst e)
  {
    /// Catch exceptions, display the error if not already done, and
    /// rethrow it.
    try
    {
      if (e)
        e->accept(*this);
    }
    catch (object::UrbiException& x)
    {
      current_.reset();
      propagate_error_(x, e->location_get());
      throw;
    }
    catch (object::FlowException&)
    {
      current_.reset();
      throw;
    }
    catch (scheduler::SchedulerException&)
    {
      current_.reset();
      throw;
    }
    catch (kernel::exception& x)
    {
      std::cerr << "Unexpected exception propagated: " << x.what() << std::endl;
      throw;
    }
    catch (std::exception& x)
    {
      std::cerr << "Unexpected exception propagated: " << x.what() << std::endl;
      throw;
    }
    catch (...)
    {
      std::cerr << "Unknown exception propagated" << std::endl;
      throw;
    }
  }


  void
  Interpreter::visit (ast::rConstAnd e)
  {
    // lhs will be evaluated in another Interpreter, while rhs will be evaluated
    // in this one. We will be the new runner parent, as we have the same
    // tags.

    JAECHO ("lhs", e->lhs_get ());
    scheduler::rJob lhs = new Interpreter(*this, ast::rConstAst(e->lhs_get()));

    // Propagate errors between left-hand side and right-hand side runners.
    link (lhs);

    lhs->start_job ();

    JAECHO ("rhs", e.rhs_get ());
    eval (e->rhs_get ());

    // Wait for lhs to terminate
    yield_until_terminated (*lhs);

    current_ = object::void_class;
  }

  void
  Interpreter::visit (ast::rConstAssignment e)
  {
    rObject tgt = context(e->depth_get());
    tgt->slot_update(*this, e->what_get(), eval(e->value_get()));
  }

  // Apply a function written in Urbi.
  object::rObject
  Interpreter::apply_urbi (const rObject& func,
                           const libport::Symbol& msg,
                           const object::objects_type& args,
                           rObject call_message)
  {
    // The called function.
    ast::rConstCode fn = func.unsafe_cast<object::Code>()->value_get().ast;
    // Whether it's an explicit closure
    bool closure = fn.unsafe_cast<const ast::Closure>();

    // If the function is lazy and there's no call message, forge
    // one. This happen when a lazy function is invoked with eval, for
    // instance.
    if (!fn->strict() && !call_message)
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
    if (fn->strict())
    {
      const ast::declarations_type& formals = *fn->formals_get();
      object::check_arg_count (formals.size() + 1, args.size(), msg.name_get());
      // Effective (evaluated) argument iterator.
      // Skip "self" which has already been handled.
      object::objects_type::const_iterator ei = ++args.begin();
      foreach (ast::rConstDeclaration s, formals)
	scope->slot_set (s->what_get(), *ei++);
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
      current_ = eval (fn->body_get());
    }
    catch (object::BreakException& be)
    {
      object::PrimitiveError error("break", "outside a loop");
      propagate_error_(error, be.location_get());
      throw error;
    }
    catch (object::ContinueException& be)
    {
      object::PrimitiveError error("continue", "outside a loop");
      propagate_error_(error, be.location_get());
      throw error;
    }
    catch (object::ReturnException& re)
    {
      current_ = re.result_get();
      if (!current_)
	current_ = object::void_class;
    }

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
	 || func->value<object::Code>().ast->strict()))
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
      object::objects_type::iterator end = args.end();
      if (std::find(++args.begin(), end, object::void_class) != end)
	throw object::WrongArgumentType (msg);
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

  void
  Interpreter::push_evaluated_arguments (object::objects_type& args,
					 const ast::exps_type& ue_args,
					 bool check_void)
  {
    bool tail = false;
    foreach (ast::rConstExp arg, ue_args)
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
	e.location_set(arg->location_get());
	throw e;
      }

      passert (*arg, current_);
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

    res->slot_set (SYMBOL(args), object::List::fresh(args));

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
    if (!args.front() || args.front()->implicit())
    {
      lazy_args.push_back(object::nil_class);
      range = make_iterator_range(range, 1, 0);
    }
    foreach (ast::rConstExp e, range)
      lazy_args.push_back(object::mkLazy(*this, e));

    return build_call_message(tgt, msg, lazy_args);
  }

  void
  Interpreter::visit (ast::rConstCall e)
  {
    // The invoked slot (probably a function).
    ast::rConstExp ast_tgt = e->target_get();
    rObject tgt = ast_tgt->implicit() ?
      locals_->slot_get(SYMBOL(self)) : eval(ast_tgt);

    call_stack_.push_back(e);
    Finally finally(bind(&call_stack_type::pop_back, &call_stack_));
    apply(tgt, e->name_get(), e->arguments_get());
  }

  void
  Interpreter::visit (ast::rConstCallMsg)
  {
    current_ = locals_->slot_get(SYMBOL(call));
  }

  Interpreter::rObject
  Interpreter::apply (rObject tgt, const libport::Symbol& message,
                      const ast::exps_type* input_ast_args)
  {
    // The invoked slot (probably a function).
    rObject val = tgt->slot_get(message);
    assertion(val);

    /*-------------------------.
    | Evaluate the arguments.  |
    `-------------------------*/

    // Gather the arguments, including the target.
    object::objects_type args;
    args.push_back (tgt);

    ast::exps_type ast_args =
      input_ast_args ? *input_ast_args : ast::exps_type();

    // FIXME: This is the target, for compatibility reasons. We need
    // to remove this, and stop assuming that arguments start at
    // calls.args.nth(1)
    ast_args.push_front(0);

    // Build the call message for non-strict functions, otherwise
    // the evaluated argument list.
    rObject call_message;
    if (val->kind_get () == object::object_kind_code
        && !val.unsafe_cast<object::Code>()->value_get().ast->strict())
      call_message = build_call_message (tgt, message, ast_args);
    else
      push_evaluated_arguments (args, ast_args, !acceptVoid(val));
    return apply (val, message, args, call_message);
  }

  void
  Interpreter::visit (ast::rConstDeclaration d)
  {
    locals_->slot_set(d->what_get(), eval(d->value_get()));
  }

  void
  Interpreter::visit (ast::rConstFloat e)
  {
    current_ = object::Float::fresh(e->value_get());
  }


  void
  Interpreter::visit (ast::rConstForeach e)
  {
    // Evaluate the list attribute, and check its type.
    JAECHO ("foreach list", e.list_get());
    operator() (e->list_get());
    TYPE_CHECK(current_, object::List);
    JAECHO("foreach body", e.body_get());

    // We need to copy the pointer on the list, otherwise the list will be
    // destroyed when children are visited and current_ is modified.
    object::List::value_type content = current_->value<object::List>();

    // The list of runners launched for each value in the list if the flavor
    // is "&".
    scheduler::jobs_type runners;
    if (e->flavor_get() == ast::flavor_and)
      runners.reserve(content.size());

    bool tail = false;
    ast::rConstAst body = e->body_get();
    ast::flavor_type flavor = e->flavor_get();
    libport::Symbol index = e->index_get()->what_get();

    // Iterate on each value.
    foreach (const rObject& o, content)
    {
      // Define a new local scope for each loop, and set the index.
      rObject locals = object::Object::fresh();
      locals->locals_set(true);
      locals->proto_add(locals_);
      locals->slot_set(index, o);

      // for& ... in loop.
      if (flavor == ast::flavor_and)
      {
	// Create the new runner and launch it. We create a link so
	// that an error in evaluation will stop other evaluations
	// as well and propagate the exception.
	Interpreter* new_runner = new Interpreter(*this, body);
	link(new_runner);
	runners.push_back(new_runner);
	new_runner->locals_ = locals;
	new_runner->start_job();
      }
      else // for| and for;
      {
	std::swap(locals, locals_);
	Finally finally(swap(locals, locals_));

	if (tail++)
	  MAYBE_YIELD(flavor);

	try
	{
	  operator() (body);
	}
	catch (object::BreakException&)
	{
	  break;
	}
	catch (object::ContinueException&)
	{
	}
      }
    }

    // Wait for all runners to terminate.
    yield_until_terminated(runners);

    // For the moment return void.
    current_ = object::void_class;
  }

  object::rObject
  Interpreter::make_code(ast::rConstCode e) const
  {
    rObject res = object::Code::fresh(e);
    // Store the function declaration context. Use make_scope to add
    // an empty object above it, so as variables injected in the
    // context do not appear in the declaration scope.
    res->slot_set(SYMBOL(context), locals_);
    return res;
  }


  void
  Interpreter::visit (ast::rConstFunction e)
  {
    current_ = make_code(e);
  }

  void
  Interpreter::visit (ast::rConstClosure e)
  {
    current_ = make_code(e);
  }


  void
  Interpreter::visit (ast::rConstIf e)
  {
    // Evaluate the test.
    JAECHO ("test", e->test_get ());
    operator() (e->test_get());

    if (object::is_true(current_))
    {
      JAECHO ("then", e.thenclause_get ());
      operator() (e->thenclause_get());
    }
    else
    {
      JAECHO ("else", e.elseclause_get ());
      operator() (e->elseclause_get());
    }
  }


  void
  Interpreter::visit (ast::rConstList e)
  {
    object::List::value_type res;
    // Evaluate every expression in the list
    foreach (ast::rConstExp c, e->value_get())
      res.push_back(eval(c));
    current_ = object::List::fresh(res);
    //ECHO ("result: " << *current_);
  }

  Interpreter::rObject Interpreter::context(unsigned n)
  {
    rObject res = locals_;
    for (int i = n; i; i--)
    {
      res = res->slot_get(SYMBOL(code))->slot_get(SYMBOL(context));
      assert(res);
    }
    return res;
  }

  void
  Interpreter::visit (ast::rConstLocal e)
  {
    rObject tgt = context(e->depth_get());
    // FIXME: Register in the call stack
    if (e->arguments_get())
      current_ = apply(tgt, e->name_get(), e->arguments_get());
    else
      current_ = tgt->slot_get(e->name_get());
  }

  void
  Interpreter::visit (ast::rConstMessage e)
  {
    send_message(e->tag_get(), e->text_get() + "\n");
  }


// Forward flow exceptions up to the top-level, and handle them
// there.  Makes only sense in a Nary.
#define CATCH_FLOW_EXCEPTION(Type, Keyword, Error)              \
  catch (Type flow_exception)                                   \
  {                                                             \
    if (e->toplevel_get ())                                     \
    {                                                           \
      object::PrimitiveError error(Keyword, Error);             \
      propagate_error_(error, flow_exception.location_get());   \
      throw error;						\
    }                                                           \
    else                                                        \
      throw;							\
  }

  void
  Interpreter::visit (ast::rConstNary e)
  {
    // List of runners for Stmt flavored by a comma.
    scheduler::jobs_type runners;

    // In case we're empty.
    current_ = object::void_class;

    bool tail = false;
    foreach (ast::rConstExp c, e->children_get())
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

      if (c.unsafe_cast<const ast::Stmt>() &&
	  c.unsafe_cast<const ast::Stmt>()->flavor_get() == ast::flavor_comma)
      {
	// The new runners are attached to the same tags as we are.
	Interpreter* subrunner = new Interpreter(*this, ast::rConstAst(c));
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
            operator() (c);
          }
	  CATCH_FLOW_EXCEPTION(object::BreakException,
			       "break", "outside a loop")
	  CATCH_FLOW_EXCEPTION(object::ContinueException,
			       "continue", "outside a loop")
	  CATCH_FLOW_EXCEPTION(object::ReturnException,
			       "return", "outside a function")

	  if (e->toplevel_get () && current_.get ())
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
	  if (e->toplevel_get())
	    show_error_(ue);
	  else
	    throw;
	}
        CATCH_FLOW_EXCEPTION(object::BreakException,
			     "break", "outside a loop")
	CATCH_FLOW_EXCEPTION(object::ContinueException,
			     "continue", "outside a loop")
        CATCH_FLOW_EXCEPTION(object::ReturnException,
                             "return", "outside a function")
      }
    }

    // If the Nary is not the toplevel one, all subrunners must be finished when
    // the runner exits the Nary node.
    if (!e->toplevel_get ())
      yield_until_terminated(runners);
  }

  void
  Interpreter::visit (ast::rConstNoop)
  {
    current_ = object::void_class;
  }


  void
  Interpreter::visit (ast::rConstPipe e)
  {
    // lhs
    JAECHO ("lhs", e->lhs_get ());
    operator() (e->lhs_get());

    // rhs:  start the execution immediately.
    JAECHO ("rhs", e->rhs_get ());
    operator() (e->rhs_get());
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
  Interpreter::visit (ast::rConstAbstractScope e, rObject locals)
  {
    bool was_non_interruptible = non_interruptible_get ();
    std::swap(locals, locals_);
    Finally finally;
    finally << swap(locals, locals_)
            << runner::non_interruptible_set(this, was_non_interruptible);
    super_type::operator()(e->body_get());
  }

  scheduler::rTag
  Interpreter::scope_tag()
  {
    scheduler::rTag tag = scope_tags_.back();
    if (!tag)
    {
      // Create the tag on demand.
      tag = scheduler::Tag::fresh
             (libport::Symbol::fresh(SYMBOL(LT_scope_SP_tag_GT)));
      *scope_tags_.rbegin() = tag;
    }
    return tag;
  }

  void
  Interpreter::cleanup_scope_tag()
  {
    scheduler::rTag tag = scope_tags_.back();
    scope_tags_.pop_back();
    if (tag)
      tag->stop(scheduler_get(), object::void_class);
  }

  void
  Interpreter::visit (ast::rConstScope e)
  {
    scope_tags_.push_back(0);
    libport::Finally finally(boost::bind(&Interpreter::cleanup_scope_tag,
					 this));
    visit (e.unsafe_cast<const ast::AbstractScope>(),
           object::Object::make_scope(locals_));
  }

  void
  Interpreter::visit (ast::rConstDo e)
  {
    rObject tgt = eval(e->target_get());
    visit (e.unsafe_cast<const ast::AbstractScope>(),
           object::Object::make_method_scope(tgt, locals_));
    // This is arguable. Do, just like Scope, should maybe return
    // their last inner value.
    current_ = tgt;
  }

  void
  Interpreter::visit (ast::rConstStmt e)
  {
    JAECHO ("expression", e.expression_get ());
    operator() (e->expression_get());
  }

  void
  Interpreter::visit (ast::rConstString e)
  {
    current_ = object::String::fresh(libport::Symbol(e->value_get()));
  }

  void
  Interpreter::visit (ast::rConstTag t)
  {
    eval_tag (t->exp_get ());
  }

  void
  Interpreter::visit (ast::rConstTaggedStmt t)
  {
    int current_depth = tags_.size();
    try
    {
      scheduler::rTag tag = extract_tag(eval(t->tag_get()));
      // If tag is blocked, do not start and ignore the
      // statement completely.
      if (tag->blocked())
	return;
      push_tag (tag);
      Finally finally(bind(&Interpreter::pop_tag, this));
      // If the latest tag causes us to be frozen, let the
      // scheduler handle this properly to avoid duplicating the
      // logic.
      if (tag->frozen())
	yield();
      eval (t->exp_get());
    }
    catch (scheduler::StopException& e)
    {
      // Rewind up to the appropriate depth.
      if (e.depth_get() < current_depth)
	throw;
      // Extract the value from the exception.
      current_ = boost::any_cast<rObject>(e.payload_get());
      // If we are frozen, reenter the scheduler for a while.
      if (frozen())
	yield();
      return;
    }
  }

  void
  Interpreter::visit (ast::rConstThis)
  {
    current_ = locals_->slot_get(SYMBOL(self));
  }

  object::rObject
  Interpreter::eval_tag (ast::rConstExp e)
  {
    try {
      // Try to evaluate e as a normal expression.
      return eval(e);
    }
    catch (object::LookupError &ue)
    {
      ECHO("Implicit tag: " << *e);
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
      rObject parent = toplevel;
      rObject where = locals_->slot_get(SYMBOL(self));
      tag_chain_type chain = decompose_tag_chain(e);
      foreach (const libport::Symbol& elt, chain)
      {
	// Check whether the concerned level in the chain already
	// exists.
	if (rObject owner = where->slot_locate (elt))
        {
          ECHO("Component " << elt << " exists.");
	  where = owner->own_slot_get (elt);
          if (object::is_a(where, toplevel))
          {
            ECHO("It is a tag, so use it as the new parent.");
            parent = where;
          }
        }
	else
	{
          ECHO("Creating component " << elt << ".");
	  // We have to create a new tag, which will be attached
	  // to the upper level (hierarchical tags, implicitly
	  // rooted by Tag).
	  rObject new_tag = toplevel->clone();
	  object::objects_type args;
	  args.push_back (new_tag);
	  args.push_back (object::String::fresh (elt));
	  args.push_back (parent);
	  apply (toplevel->own_slot_get (SYMBOL (init)), SYMBOL(init), args);
	  where->slot_set (elt, new_tag);
	  where = parent = new_tag;
	}
      }

      return parent;
    }
  }

  void
  Interpreter::visit (ast::rConstThrow e)
  {
    switch (e->kind_get())
    {
      case ast::Throw::exception_break:
	throw object::BreakException(e->location_get());

      case ast::Throw::exception_continue:
        throw object::ContinueException(e->location_get());

      case ast::Throw::exception_return:
	if (e->value_get())
	  operator() (e->value_get());
	else
	  current_.reset();
	throw object::ReturnException(e->location_get(), current_);
    }
  }


  void
  Interpreter::visit (ast::rConstWhile e)
  {
    bool tail = false;
    // Evaluate the test.
    while (true)
    {
      if (tail++)
	MAYBE_YIELD (e->flavor_get());
      JAECHO ("while test", e.test_get ());
      operator() (e->test_get());
      if (!object::is_true(current_))
	break;

      JAECHO ("while body", e.body_get ());

      try
      {
	operator() (e->body_get());
      }
      catch (object::BreakException&)
      {
	break;
      }
      catch (object::ContinueException&)
      {
      }
    }
    current_ = object::void_class;
  }

  void
  Interpreter::show_backtrace(const call_stack_type& bt,
                              const std::string& chan)
  {
    foreach (ast::rConstCall c,
             boost::make_iterator_range(boost::rbegin(bt),
                                        boost::rend(bt)))
    {
      std::ostringstream o;
      o << "!!!    called from: " << c->location_get () << ": "
	<< c->name_get () << std::endl;
      send_message(chan, o.str());
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
    foreach (ast::rConstCall c, call_stack_)
    {
      std::ostringstream o;
      o << c->location_get();
      res.push_back(std::make_pair(c->name_get().name_get(), o.str()));
    }
    return res;
  }

} // namespace runner
