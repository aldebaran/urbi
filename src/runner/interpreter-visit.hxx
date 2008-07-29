#ifndef RUNNER_INTERPRETER_VISIT_HXX
# define RUNNER_INTERPRETER_VISIT_HXX

# include <boost/bind.hpp>

# include <libport/compiler.hh>
# include <libport/finally.hh>
# include <libport/foreach.hh>

# include <ast/all.hh>

# include <kernel/exception.hh>
# include <kernel/uconnection.hh>

# include <object/code-class.hh>
# include <object/global-class.hh>
# include <object/list-class.hh>
# include <object/tag-class.hh>
# include <object/symbols.hh>

# include <runner/interpreter.hh>

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

namespace runner
{
  using boost::bind;
  using libport::Finally;

  inline void
  Interpreter::visit(const ast::And* e)
  {
    // Collect all subrunners
    scheduler::jobs_type jobs;

    // Create separate runners for every child but the first
    foreach (const ast::rConstExp& child,
             boost::make_iterator_range(e->children_get(), 1, 0))
    {
      Interpreter* job = new Interpreter(*this, eval(child));
      // Propagate errors from subrunners.
      link(job);
      jobs.push_back(job);
      job->start_job();
    }

    // Evaluate the first child in this runner
    rRoutine code = eval(e->children_get().front()).unsafe_cast<object::Code>();
    assert(code);
    // This is a closure, it won't use its 'this'
    object::objects_type args;
    args.push_back(rObject());
    apply_urbi(code, SYMBOL(), args, 0);

    result_ = object::void_class;

    // Wait for all other jobs to terminate
    yield_until_terminated(jobs);
  }

  inline void
  Interpreter::visit(const ast::Assignment* e)
  {
    stacks_.set(e, eval(e->value_get()));
  }

  inline void
  Interpreter::visit(const ast::Call* e)
  {
    // The invoked slot (probably a function).
    const ast::rConstExp& ast_tgt = e->target_get();
    rObject tgt = ast_tgt->implicit() ? stacks_.self() : eval(ast_tgt);
    apply(tgt, e->name_get(), e->arguments_get(), e->location_get());
  }

  inline void
  Interpreter::visit(const ast::CallMsg*)
  {
    result_ = stacks_.call();
  }


  inline void
  Interpreter::visit(const ast::Declaration* d)
  {
    rObject value = eval(d->value_get());
    stacks_.def(d, value);
  }

  inline void
  Interpreter::visit(const ast::Float* e)
  {
    result_ = new object::Float(e->value_get());
  }


  inline void
  Interpreter::visit(const ast::Foreach* e)
  {
    (void)e;
    pabort(e);
  }


  inline void Interpreter::visit(const ast::Routine* e, bool closure)
  {
    rRoutine res = make_routine(e);
    result_ = res;

    // Capture variables
    foreach (const ast::rDeclaration& dec, *e->captured_variables_get())
    {
      ast::rLocal local = dec->value_get().unsafe_cast<ast::Local>();
      assert(local);
      res->captures_get().push_back(stacks_.rget(local));
    }

    // Capture 'this' and 'call' in closures
    if (closure)
    {
      res->self_get() = stacks_.self();
      res->call_get() = stacks_.call();
    }
  }

  inline void
  Interpreter::visit(const ast::Function* e)
  {
    visit(e, false);
  }

  inline void
  Interpreter::visit(const ast::Closure* e)
  {
    visit(e, true);
  }


  inline void
  Interpreter::visit(const ast::If* e)
  {
    // Evaluate the test.
    JAECHO ("test", e->test_get ());
    operator() (e->test_get().get());

    if (object::is_true(result_, SYMBOL(if)))
    {
      JAECHO ("then", e->thenclause_get());
      operator() (e->thenclause_get().get());
    }
    else
    {
      JAECHO ("else", e->elseclause_get());
      operator() (e->elseclause_get().get());
    }
  }


  inline void
  Interpreter::visit(const ast::List* e)
  {
    object::List::value_type res;
    // Evaluate every expression in the list
    foreach (const ast::rConstExp& c, e->value_get())
    {
      rObject v = eval(c);
      // Refuse void in literal lists
      if (v == object::void_class)
      {
        object::WrongArgumentType e(SYMBOL(new));
        e.location_set(c->location_get());
        throw e;
      }
      res.push_back(v);
    }
    result_ = new object::List(res);
    //ECHO ("result: " << *result_);
  }

  inline void
  Interpreter::visit(const ast::Lazy* e)
  {
    operator()(e->strict_get().get());
  }

  inline void
  Interpreter::visit(const ast::Local* e)
  {
    const rObject& value = stacks_.get(e);

    passert("Local variable read before being set", value);

    if (e->arguments_get())
      // FIXME: Register in the call stack
      result_ = apply(stacks_.self(), value,
                      e->name_get(), e->arguments_get(),
                      e->location_get());
    else
      result_ = value;
  }

  inline void
  Interpreter::visit(const ast::Message* e)
  {
    send_message(e->tag_get(), e->text_get());
  }


  inline void
  Interpreter::visit(const ast::Nary* e)
  {
    // List of runners for Stmt flavored by a comma.
    scheduler::jobs_type runners;

    // In case we're empty.
    result_ = object::void_class;

    bool tail = false;
    foreach (const ast::rConstExp& c, e->children_get())
    {
      // Allow some time to pass before we execute what follows.  If
      // we don't do this, the ;-operator would act almost like the
      // |-operator because it would always start to execute its RHS
      // immediately. However, we don't want to do it before the first
      // statement or if we only have one statement in the scope.
      if (tail++)
	YIELD();

      result_.reset();
      JAECHO("child", c);

      if (c.unsafe_cast<const ast::Stmt>() &&
	  c.unsafe_cast<const ast::Stmt>()->flavor_get() == ast::flavor_comma)
      {
	// The new runners are attached to the same tags as we are.
	Interpreter* subrunner = new Interpreter(*this, eval(c));
	// If the subrunner throws an exception, propagate it here ASAP, unless
	// we are at the top level.
	if (!e->toplevel_get())
	  link(subrunner);
	runners.push_back(subrunner);
	subrunner->start_job ();
      }
      else
      {
	// If at toplevel, print errors and continue, else rethrow them
	try
	{
          // We do not use operator() to avoid duplicating the catch
          // of UrbiExceptions
          res = c->eval(*this);
	  // We need to keep checking for void here because it can not be passed
	  // to the << function
	  if (e->toplevel_get() && result_.get()
	    && result_ != object::void_class)
	  {
	    try
	    {
	      assertion(result_);
	      ECHO("toplevel: returning a result to the connection.");

	      // Display the value using the topLevel channel.
	      // If it is not (yet) defined, do nothing, unless the environment
	      // variable TOPLEVEL_DEBUG is set.

	      static bool toplevel_debug = getenv("TOPLEVEL_DEBUG");
	      if (rObject topLevel =
	        object::global_class->slot_locate(SYMBOL(topLevel), false,
		  true))
	      {
		rObject e = topLevel->slot_get(SYMBOL(LT_LT));
                objects_type args;
                args.push_back(topLevel);
                args.push_back(result_);
		apply(e, SYMBOL(topLevel), args);
	      }
	      else if (toplevel_debug)
		lobby_->value_get().connection.new_result(result_);
              result_.reset();
	    }
	    catch (std::exception &ke)
	    {
	      std::cerr << "Exception when printing result: "
                        << ke.what() << std::endl;
	    }
	    catch (object::UrbiException& e)
	    {
	      throw;
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
          propagate_error_(ue, c->location_get());
	  if (e->toplevel_get())
	    show_error_(ue);
	  else
	    throw;
	}
      }
    }

    // If the Nary is not the toplevel one, all subrunners must be finished when
    // the runner exits the Nary node. However, it we have a scopeTag, we must
    // issue a "stop" which may interrupt subrunners.
    if (!e->toplevel_get() && !runners.empty())
    {
      const scheduler::rTag& tag = scope_tag_get();
      if (tag)
	tag->stop(scheduler_get(), object::void_class);
      yield_until_terminated(runners);
    }
  }

  inline void
  Interpreter::visit(const ast::Noop*)
  {
    result_ = object::void_class;
  }


  inline void
  Interpreter::visit(const ast::Pipe* e)
  {
    // lhs
    JAECHO ("lhs", e->lhs_get ());
    operator() (e->lhs_get().get());

    // rhs:  start the execution immediately.
    JAECHO ("rhs", e->rhs_get ());
    operator() (e->rhs_get().get());
  }

  inline void
  Interpreter::visit(const ast::Scope* e)
  {
    libport::Finally finally;
    create_scope_tag();
    finally << boost::bind(&Interpreter::cleanup_scope_tag, this);
    finally << libport::restore(non_interruptible_);
    operator()(e->body_get().get());
  }

  inline void
  Interpreter::visit(const ast::Do* e)
  {
    Finally finally;

    rObject tgt = eval(e->target_get());

    finally << stacks_.switch_self(tgt);

    const ast::Scope* scope = reinterpret_cast<const ast::Scope*>(e);
    visit(scope);
    // This is arguable. Do, just like Scope, should maybe return
    // their last inner value.
    result_ = tgt;
  }

  inline void
  Interpreter::visit(const ast::Stmt* e)
  {
    JAECHO ("expression", e->expression_get());
    operator() (e->expression_get().get());
  }

  inline void
  Interpreter::visit(const ast::String* e)
  {
    result_ = new object::String(libport::Symbol(e->value_get()));
  }

  inline void
  Interpreter::visit(const ast::Tag* t)
  {
    result_ = eval_tag(t->exp_get());
  }

  inline void
  Interpreter::visit(const ast::TaggedStmt* t)
  {
    int result_depth = tags_get().size();
    try
    {
      // FIXME: might be simplified after type checking code is moved
      // to Object
      object::rObject unchecked_tag = eval(t->tag_get());
      object::type_check<object::Tag>(unchecked_tag, SYMBOL(tagged_stmt));
      const object::rTag& urbi_tag = unchecked_tag->as<object::Tag>();
      const scheduler::rTag& tag = urbi_tag->value_get();
      // If tag is blocked, do not start and ignore the
      // statement completely but use the provided payload.
      if (tag->blocked())
      {
	result_ = boost::any_cast<rObject>(tag->payload_get());
	return;
      }
      push_tag (tag);
      Finally finally(bind(&Interpreter::pop_tag, this));
      // If the latest tag causes us to be frozen, let the
      // scheduler handle this properly to avoid duplicating the
      // logic.
      if (tag->frozen())
	yield();
      urbi_tag->triggerEnter(*this);
      rObject res = eval (t->exp_get());
      urbi_tag->triggerLeave(*this);
      result_ = res;
    }
    catch (scheduler::StopException& e)
    {
      // Rewind up to the appropriate depth.
      if (e.depth_get() < result_depth)
	throw;
      // Extract the value from the exception.
      result_ = boost::any_cast<rObject>(e.payload_get());
      // If we are frozen, reenter the scheduler for a while.
      if (frozen())
	yield();
    }
  }

  inline void
  Interpreter::visit(const ast::This*)
  {
    result_ = stacks_.self();
  }

  inline void
  Interpreter::visit(const ast::While* e)
  {
    const bool must_yield = e->flavor_get() == ast::flavor_semicolon;
    bool tail = false;
    // Evaluate the test.
    while (true)
    {
      if (must_yield && tail++)
	YIELD();
      JAECHO ("while test", e->test_get());
      operator() (e->test_get().get());
      if (!object::is_true(result_, SYMBOL(while)))
	break;

      JAECHO ("while body", e->body_get());

      operator() (e->body_get().get());
    }
    result_ = object::void_class;
  }

  // Invalid nodes
#define INVALID(Node)                                      \
  inline object::rObject                                   \
  Interpreter::visit(const ast::Node* n)                   \
  {                                                        \
    static_cast<void>(n);                                  \
    pabort("Invalid node in the Interpreter: " << *n);     \
  }                                                        \

  INVALID(Binding);
  INVALID(Break);
  INVALID(Continue);
  INVALID(Implicit);
  INVALID(MetaExp);
  INVALID(Return);

#undef INVALID

}


#endif
