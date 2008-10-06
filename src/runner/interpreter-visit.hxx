#ifndef RUNNER_INTERPRETER_VISIT_HXX
# define RUNNER_INTERPRETER_VISIT_HXX

# include <boost/bind.hpp>

# include <libport/compiler.hh>
# include <libport/finally.hh>
# include <libport/foreach.hh>

# include <ast/all.hh>
# include <ast/print.hh>

# include <kernel/exception.hh>
# include <kernel/uconnection.hh>

# include <object/code.hh>
# include <object/global.hh>
# include <object/list.hh>
# include <object/tag.hh>
# include <object/symbols.hh>

# include <runner/call.hh>
# include <runner/interpreter.hh>
# include <runner/raise.hh>

# include <libport/compilation.hh>

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

// Rewind up to the appropriate depth when catching a StopException
#define REWIND_STOP(Lvl)                        \
  if (e.depth_get() < Lvl)             \
    throw;                                      \


namespace runner
{
  using boost::bind;
  using libport::Finally;
  using object::rTag;
  using object::Tag;

  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::And* e)
  {
    libport::Finally finally;
    // Collect all subrunners
    scheduler::jobs_type jobs;

    // Create separate runners for every child but the first
    foreach (const ast::rConstExp& child,
             boost::make_iterator_range(e->children_get(), 1, 0))
    {
      Interpreter* job = new Interpreter(*this, operator()(child.get()));
      register_child(job, finally);
      jobs.push_back(job);
      job->start_job();
    }

    rCode code = operator()(e->children_get().front().get())
      .unsafe_cast<object::Code>();
    assert(code);

    try
    {
      // Evaluate the first child in this runner
      object::objects_type args;
      // FIXME: This is a closure, it won't use its 'this', but this is
      // gory.
      apply_urbi(rObject(), code, SYMBOL(), args, 0);
      // Wait for all other jobs to terminate.
      yield_until_terminated(jobs);
    }
    catch (const scheduler::ChildException& ce)
    {
      // If a child caused us to die, then throw the encapsulated exception.
      kernel::rethrow(ce.child_exception_get());
    }

    return object::void_class;
  }


  static void check_void(const object::rObject& v)
  {
    if (v == object::void_class)
      raise_unexpected_void_error();
  }

  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::LocalAssignment* e)
  {
    rObject val = operator()(e->value_get().get());
    check_void(val);
    stacks_.set(e, val);
    return val;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Call* e)
  {
    // The invoked slot (probably a function).
    const ast::rConstExp& ast_tgt = e->target_get();
    rObject tgt = ast_tgt->implicit() ? stacks_.self() : operator()(ast_tgt.get());
    return apply_ast(tgt, e->name_get(), e->arguments_get(), e->location_get());
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::CallMsg*)
  {
    return stacks_.call();
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::LocalDeclaration* d)
  {
    ast::rExp v = d->value_get();
    rObject val = v ? operator()(v.get()) : object::void_class;
    if (v)
      check_void(val);
    stacks_.def(d, val);
    return val;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Float* e)
  {
    return new object::Float(e->value_get());
  }


  LIBPORT_SPEED_INLINE object::rObject Interpreter::visit(const ast::Routine* e, bool closure)
  {
    rCode res = make_routine(e);

    // Capture variables
    foreach (const ast::rLocalDeclaration& dec,
             *e->captured_variables_get())
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

    return res;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Function* e)
  {
    return visit(e, false);
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Closure* e)
  {
    return visit(e, true);
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::If* e)
  {
    // Evaluate the test.
    JAECHO ("test", e->test_get ());
    rObject cond = operator()(e->test_get().get());

    if (object::is_true(cond))
    {
      JAECHO ("then", e->thenclause_get());
      return e->thenclause_get()->eval(*this);
    }
    else
    {
      JAECHO ("else", e->elseclause_get());
      return e->elseclause_get()->eval(*this);
    }
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::List* e)
  {
    object::List::value_type res;
    // Evaluate every expression in the list
    foreach (const ast::rConstExp& c, e->value_get())
    {
      rObject v = operator()(c.get());
      // Refuse void in literal lists
      if (v == object::void_class)
	raise_unexpected_void_error();
      res.push_back(v);
    }
    return new object::List(res);
    //ECHO ("result: " << *result_);
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Lazy* e)
  {
    return operator()(e->strict_get().get());
  }

  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Local* e)
  {
    const rObject& value = stacks_.get(e);

    passert("Local variable read before being set", value);

    if (e->arguments_get())
      // FIXME: Register in the call stack
      return apply_ast(stacks_.self(), value,
                       e->name_get(), e->arguments_get(),
                       e->location_get());
    else
      return value;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Message* e)
  {
    send_message(e->tag_get(), e->text_get());
    return object::void_class;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Nary* e)
  {
    // List of runners for Stmt flavored by a comma.
    scheduler::jobs_type runners;

    Finally finally;

    // In case we're empty, {} evaluates to void.
    rObject res = object::void_class;

    bool tail = false;
    try
    {
      foreach (const ast::rConstExp& c, e->children_get())
      {
        // Allow some time to pass before we execute what follows.  If
        // we don't do this, the ;-operator would act almost like the
        // |-operator because it would always start to execute its RHS
        // immediately. However, we don't want to do it before the first
        // statement or if we only have one statement in the scope.
        if (tail++)
          YIELD();

        JAECHO("child", c);

        const ast::Stmt* stmt = dynamic_cast<const ast::Stmt*>(c.get());
        const ast::Exp* exp = stmt ? stmt->expression_get().get() : c.get();

        if (stmt && stmt->flavor_get() == ast::flavor_comma)
        {
          // The new runners are attached to the same tags as we are.
	  scheduler::rJob subrunner = new Interpreter(*this, operator()(exp));
          // If the subrunner throws an exception, propagate it here ASAP, unless
          // we are at the top level. It we are at the toplevel, we do not even
	  // have to register it as a subrunner.
          if (!e->toplevel_get())
	  {
	    register_child(subrunner, finally);
	    runners.push_back(subrunner);
	  }
          subrunner->start_job ();
        }
        else
        {
          // If at toplevel, print errors and continue, else rethrow them
          try
          {
            res = operator()(exp);
            // We need to keep checking for void here because it can not be passed
            // to the << function
            if (e->toplevel_get()
                && res.get() // FIXME: What's that for?
                && res != object::void_class)
            {
	      assertion(res);
	      ECHO("toplevel: returning a result to the connection.");

	      // Display the value using the topLevel channel.
	      // If it is not (yet) defined, do nothing, unless the environment
	      // variable TOPLEVEL_DEBUG is set.

	      static bool toplevel_debug = getenv("TOPLEVEL_DEBUG");
	      if (rObject topLevel =
		  object::global_class->slot_locate(SYMBOL(topLevel), false,
						    true))
		object::urbi_call(*this, topLevel, SYMBOL(LT_LT), res);
	      else if (toplevel_debug)
		lobby_->value_get().connection.new_result(res);
	    }
          }
          catch (object::Exception& ue)
          {
            propagate_error_(ue, c->location_get());

	    if (e->toplevel_get())
	      show_error_(ue);
	    else
	      throw;
          }
          // Catch and print unhandled exceptions
          catch (object::UrbiException& exn)
          {
            if (e->toplevel_get())
              show_exception_(exn);
            else
              throw;
          }
        }
      }
      // If we get a scope tag, stop the runners tagged with it.
      if (const scheduler::rTag& tag = scope_tag_get())
	tag->stop(scheduler_get(), object::void_class);
      yield_until_terminated(runners);
    }
    catch (const scheduler::ChildException& ce)
    {
      kernel::rethrow(ce.child_exception_get());
    }

    return res;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Noop*)
  {
    return object::void_class;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Pipe* e)
  {
    object::rObject res;

    // Run all child without yielding
    foreach (const ast::rConstExp& child, e->children_get())
      res = operator()(child.get());

    return res;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Scope* e)
  {
    libport::Finally finally;
    create_scope_tag();
    finally << boost::bind(&Interpreter::cleanup_scope_tag, this);
    finally << libport::restore(non_interruptible_);
    return operator()(e->body_get().get());
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Do* e)
  {
    Finally finally;

    rObject tgt = operator()(e->target_get().get());

    finally << stacks_.switch_self(tgt);

    visit(static_cast<const ast::Scope*>(e));
    // This is arguable. Do, just like Scope, should maybe return
    // their last inner value.
    return tgt;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Stmt* e)
  {
    JAECHO ("expression", e->expression_get());
    return operator()(e->expression_get().get());
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::String* e)
  {
    return new object::String(e->value_get());
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Tag* t)
  {
    return eval_tag(t->exp_get());
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::TaggedStmt* t)
  {
    // FIXME: might be simplified after type checking code is moved
    // to Object
    object::rObject unchecked_tag = operator()(t->tag_get().get());
    object::type_check(unchecked_tag, object::Tag::proto);
    const object::rTag& urbi_tag = unchecked_tag->as<object::Tag>();
    const scheduler::rTag& tag = urbi_tag->value_get();

    // If tag is blocked, do not start and ignore the
    // statement completely but use the provided payload.
    if (tag->blocked())
      return boost::any_cast<rObject>(tag->payload_get());

    size_t result_depth = tags_get().size();
    try
    {
      Finally finally(2);
      apply_tag(tag, &finally);
      finally << scoped_push(urbi_tag, tag_stack_);
      // If the latest tag causes us to be frozen, let the
      // scheduler handle this properly to avoid duplicating the
      // logic.
      if (tag->frozen())
	yield();
      urbi_tag->triggerEnter(*this);
      rObject res = operator()(t->exp_get().get());
      urbi_tag->triggerLeave(*this);
      return res;
    }
    catch (scheduler::StopException& e)
    {
      REWIND_STOP(result_depth);
      // If we are frozen, reenter the scheduler for a while.
      if (frozen())
	yield();
      // Extract the value from the exception.
      return boost::any_cast<rObject>(e.payload_get());
    }
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::This*)
  {
    return stacks_.self();
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Throw* e)
  {
    raise(e->value_get() ?
	  operator()(e->value_get().get()) :
	  current_exception_);
    pabort("Unreachable");
    return 0;
  }



  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Try* e)
  {
    try
    {
      return operator()(e->body_get().get());
    }
    catch (object::UrbiException& exn)
    {
      rObject value = exn.value_get();
      foreach (ast::rCatch handler, e->handlers_get())
      {
        if (ast::rExp match = handler->match_get())
          if (!is_a(value, operator()(match.get())))
            continue;
	if (handler->declaration_get())
	  stacks_.def(handler->declaration_get().get(), value);
	{
	  libport::Finally finally(scoped_set(current_exception_, value));
	  return operator()(handler->body_get().get());
	}
      }
      // No handler matched, rethrow.
      throw;
    }
  }


  LIBPORT_SPEED_INLINE object::rObject
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
      rObject cond = operator()(e->test_get().get());
      if (!object::is_true(cond))
	break;

      JAECHO ("while body", e->body_get());

      e->body_get()->eval(*this);
    }
    return object::void_class;
  }

// MSVC seems to disregard the noreturn attribute of what pabort expands to.
#ifdef _MSC_VER
# define INVALID_RET return object::void_class;
#else
# define INVALID_RET
#endif

  // Invalid nodes
#define INVALID(Node)                                      \
  LIBPORT_SPEED_INLINE object::rObject                     \
  Interpreter::visit(const ast::Node* n)                   \
  {                                                        \
    static_cast<void>(n);                                  \
    pabort("Invalid node in the Interpreter: " << *n);     \
    INVALID_RET                                            \
  }                                                        \

  INVALID(Assignment);
  INVALID(Binding);
  INVALID(Break);
  INVALID(Catch);
  INVALID(Class);
  INVALID(Continue);
  INVALID(Declaration);
  INVALID(Decrementation);
  INVALID(Delete);
  INVALID(Emit);
  INVALID(Foreach);
  INVALID(Implicit);
  INVALID(Incrementation);
  INVALID(MetaArgs);
  INVALID(MetaCall);
  INVALID(MetaExp);
  INVALID(MetaId);
  INVALID(MetaLValue);
  INVALID(OpAssignment);
  INVALID(PropertyRead);
  INVALID(PropertyWrite);
  INVALID(Return);

#undef INVALID
#undef INVALID_RET
}


#endif
