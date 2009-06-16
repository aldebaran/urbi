#ifndef RUNNER_INTERPRETER_VISIT_HXX
# define RUNNER_INTERPRETER_VISIT_HXX

# include <boost/bind.hpp>
# include <boost/format.hpp>
# include <boost/scoped_ptr.hpp>

# include <libport/compilation.hh>
# include <libport/compiler.hh>
# include <libport/contract.hh>
# include <libport/finally.hh>
# include <libport/foreach.hh>

# include <ast/all.hh>
# include <ast/print.hh>

# include <kernel/uconnection.hh>

# include <object/code.hh>
# include <object/global.hh>
# include <object/list.hh>
# include <object/dictionary.hh>
# include <object/tag.hh>
# include <object/slot.hh>
# include <object/string.hh>
# include <object/symbols.hh>

# include <runner/interpreter.hh>
# include <runner/raise.hh>

# include <sched/exception.hh>

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
  using object::rSlot;
  using object::rTag;
  using object::Slot;
  using object::Tag;

  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::And* e)
  {
    Job::ChildrenCollecter children(this, e->children_get().size() - 1);

    // Collect all subrunners
    sched::jobs_type jobs;

    // Create separate runners for every child but the first
    foreach (const ast::rConstExp& child,
             boost::make_iterator_range(e->children_get(), 1, 0))
    {
      Interpreter* job =
	new Interpreter(*this, operator()(child.get()),
			libport::Symbol::fresh(name_get()));
      register_child(job, children);
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
      args.push_back(rObject());
      apply_urbi(code, libport::Symbol::make_empty(), args, 0);
      // Wait for all other jobs to terminate.
      yield_until_terminated(jobs);
    }
    catch (const sched::ChildException& ce)
    {
      // If a child caused us to die, then throw the encapsulated exception.
      ce.rethrow_child_exception();
    }

    return object::void_class;
  }


  static inline void check_void(const object::rObject& v)
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
    rObject tgt = operator()(ast_tgt.get());
    return apply_ast(tgt, e->name_get(), e->arguments_get(), e->location_get());
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::CallMsg*)
  {
    return stacks_.call();
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Implicit*)
  {
    return stacks_.self();
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::LocalDeclaration* d)
  {
    ast::rExp v = d->value_get();
    rObject res = v ? operator()(v.get()) : object::void_class;
    if (v)
      check_void(res);
    stacks_.def(d, res, d->constant_get());
    return res;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Float* e)
  {
    return new object::Float(e->value_get());
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Routine* e)
  {
    rCode res = make_routine(e);

    // Capture variables.
    foreach (const ast::rLocalDeclaration& dec,
             *e->captured_variables_get())
    {
      ast::rLocal local = dec->value_get().unsafe_cast<ast::Local>();
      assert(local);
      res->captures_get().push_back(stacks_.rget(local));
    }

    // Capture 'this' and 'call' in closures.
    if (e->closure_get())
    {
      res->self_set(stacks_.self());
      res->call_get() = stacks_.call();
    }

    return res;
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
    // Evaluate every expression in the list.
    foreach (const ast::rConstExp& c, e->value_get())
    {
      rObject v = operator()(c.get());
      // Refuse void in literal lists.
      if (v == object::void_class)
	raise_unexpected_void_error();
      res.push_back(v);
    }
    return new object::List(res);
    //ECHO ("result: " << *result_);
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
    const ast::exps_type& exps = e->children_get();

    // An empty Nary does nothing and returns void. We do not need
    // to setup everything below or create intermediate objects.
    if (exps.empty())
      return object::void_class;

    // Optimize a common case where an inner (non-toplevel) Nary
    // contains only one statement not ending with a comma. This is
    // for example the case of Nary created by a "if" branch without
    // braces. In this case, we will make a tail-call to avoid
    // cluttering the stack.
    if (!e->toplevel_get() && exps.size() == 1)
    {
      const ast::Stmt* stmt =
	dynamic_cast<const ast::Stmt*>(exps.front().get());
      if (stmt && stmt->flavor_get() != ast::flavor_comma)
	return visit(stmt);
    }

    // List of runners for Stmt flavored by a comma.
    sched::jobs_type runners;

    Job::ChildrenCollecter children(this, 0);

    // Initialize the result to void to account for a Nary which would
    // contain only comma-terminated statements.
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
	  sched::rJob subrunner =
	    new Interpreter(*this, operator()(exp),
			    libport::Symbol::fresh(name_get()));
          // If the subrunner throws an exception, propagate it here
          // ASAP, unless we are at the top level. It we are at the
          // toplevel, we do not even have to register it as a
          // subrunner.
          if (!e->toplevel_get())
	  {
	    register_child(subrunner, children);
	    runners.push_back(subrunner);
	  }
          subrunner->start_job ();
        }
        else
        {
	  // Visual Studio on Windows does not rewind the stack before the
	  // end of a "try/catch" block, "catch" included. It means that
	  // we cannot display the exception in the "catch" block in case
	  // this is a scheduling error due to stack exhaustion, as we
	  // may need the stack. The following objects will be set if we
	  // have an exception to show, and it will be printed after
	  // the "catch" block, or if we have an exception to rethrow.
	  sched::exception_ptr exception_to_throw;
	  boost::scoped_ptr<object::UrbiException> exception_to_show;

          // If at toplevel, print errors and continue, else rethrow them
          if (e->toplevel_get())
          {
            try
            {
              res = operator()(exp);
              // We need to keep checking for void here because it
              // cannot be passed to the << function.
              if (res != object::void_class)
              {
                static bool toplevel_debug = getenv("TOPLEVEL_DEBUG");

                ECHO("toplevel: returning a result to the connection.");

                // Display the value using the topLevel channel.  If
                // it is not (yet) defined, do nothing, unless the
                // environment variable TOPLEVEL_DEBUG is set.
                if (rSlot topLevel =
                    object::global_class->slot_locate
                    (SYMBOL(topLevel), false).second)
                  topLevel->value()->call(SYMBOL(LT_LT), res);
                else if (toplevel_debug)
                {
                  try
                  {
                    rObject result = res->call(SYMBOL(asToplevelPrintable));
                    std::ostringstream os;
                    result->print(os);
                    std::string r = os.str();
                    lobby_->connection_get().send(r.c_str(), r.size());
                    lobby_->connection_get().endline();
                  }
                  catch (object::UrbiException&)
                  {
                    // nothing
                  }
                }
              }
            }
            // Catch and print unhandled exceptions
            catch (object::UrbiException& exn)
            {
	      exception_to_show.reset
	        (new object::UrbiException(exn.value_get(),
					   exn.backtrace_get()));
            }
            // Forward scheduler exception
            catch (const sched::exception& e)
            {
              exception_to_throw = e.clone();
            }
            // Stop invalid exceptions thrown by primitives
            catch (const std::exception& e)
            {
              static boost::format format("Invalid exception `%s' caught");
              send_message("error", str(format % e.what()));
            }
            catch (...)
            {
              send_message("error", "Invalid unknown exception caught");
            }

            if (exception_to_throw.get())
              exception_to_throw->rethrow();
            else if (exception_to_show.get())
	    {
              show_exception_(*exception_to_show);

	      // In the case of a Nary with multiple elements at the toplevel,
	      // we want to reset the result to void to make sure that we
	      // do not return a reference onto the previously evaluated
	      // expression.
	      res = object::void_class;
	    }
          }
          else
            res = operator()(exp);
        }
      }
      // If we get a scope tag, stop the runners tagged with it.
      if (const sched::rTag& tag = scope_tag_get())
	tag->stop(scheduler_get(), object::void_class);
      yield_until_terminated(runners);
    }
    catch (const sched::ChildException& ce)
    {
      ce.rethrow_child_exception();
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
    object::rObject res = object::void_class;

    // Run all child without yielding
    foreach (const ast::rConstExp& child, e->children_get())
      res = operator()(child.get());

    return res;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Property* p)
  {
    ast::rExp owner = p->owner_get();
    if (ast::rCall call = owner.unsafe_cast<ast::Call>())
    {
      rObject owner = operator()(call->target_get().get());
      return owner->call(SYMBOL(getProperty),
                         new object::String(call->name_get()),
                         new object::String(p->name_get()));
    }
    else if (ast::rLocal local = owner.unsafe_cast<ast::Local>())
    {
      rObject res = stacks_.rget(local)->property_get(p->name_get());
      return res ? res : object::void_class;
    }
    else
      pabort("Unrecognized property owner");
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::PropertyWrite* p)
  {
    rObject value = operator()(p->value_get().get());
    ast::rExp owner = p->owner_get();
    if (ast::rCall call = owner.unsafe_cast<ast::Call>())
    {
      rObject owner = operator()(call->target_get().get());
      return owner->call(SYMBOL(setProperty),
                         new object::String(call->name_get()),
                         new object::String(p->name_get()),
                         value);
    }
    else if (ast::rLocal local = owner.unsafe_cast<ast::Local>())
    {
      stacks_.rget(local)->property_set(p->name_get(), value);
      return value;
    }
    else
      pabort("Unrecognized property owner");
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
  Interpreter::visit(const ast::Dictionary* e)
  {
    object::rDictionary res = new object::Dictionary();
    /// Dictonary with a base was caught at parse time.
    assert(!e->base_get());
    foreach (ast::modifiers_type::value_type exp, e->value_get())
    {
      rObject v = operator()(exp.second.get());
      // Refuse void in literals.
      if (v == object::void_class)
	raise_unexpected_void_error();
      passert(v, v);
      res->set(exp.first, v);
    }
    return res;
  }

  /// Trigger leave event of all \a tags
  static void
  trigger_leave(const std::vector<object::rTag>& tags)
  {
    // Start with the most specific tag in the derivation chain.
    foreach (const object::rTag& tag, tags)
      tag->triggerLeave();
  }

  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::TaggedStmt* t)
  {
    // FIXME: might be simplified after type checking code is moved
    // to Object.
    object::rObject unchecked_tag = eval_tag(t->tag_get());
    object::type_check<object::Tag>(unchecked_tag);
    object::rTag urbi_tag = unchecked_tag->as<object::Tag>();

    size_t result_depth = tag_stack_size();
    std::vector<object::rTag> applied;
    try
    {
      Finally finally;
      bool some_frozen = false;
      // Apply tag as well as its ancestors to the current runner.
      do
      {
	// If tag is blocked, do not start and ignore the
	// statement completely but use the provided payload.
	// Since the list of parents starts from the specific tag
	// and goes up to the root tag, the most specific payload
	// will be retrieved.
	if (urbi_tag->value_get()->blocked())
	  return boost::any_cast<rObject>(urbi_tag->value_get()->payload_get());
	// If tag is frozen, remember it.
	some_frozen = some_frozen || urbi_tag->value_get()->frozen();
	applied.push_back(urbi_tag);
	urbi_tag = urbi_tag->parent_get();
      } while (urbi_tag);
      // Apply the tags, starting with the uppermost one in the ancestry
      // chain so that a "stop" on a parent removes the corresponding
      // children.
      foreach (const rTag& tag, applied)
	apply_tag(tag, &finally);
      // If one of the tags caused us to be frozen, let the scheduler
      // handle this properly to avoid duplicating the logic.
      if (some_frozen)
	yield();
      // Start with the uppermost tag in the derivation chain.
      rforeach (const object::rTag& tag, applied)
        tag->triggerEnter();
      rObject res = operator()(t->exp_get().get());
      trigger_leave(applied);
      return res;
    }
    catch (sched::StopException& e)
    {
      trigger_leave(applied);
      // Rewind up to the appropriate depth.
      if (e.depth_get() < result_depth)
	throw;

      // If we are frozen, reenter the scheduler for a while.
      if (frozen())
	yield();
      // Extract the value from the exception.
      return boost::any_cast<rObject>(e.payload_get());
    }
    catch (sched::exception&)
    {
      trigger_leave(applied);
      throw;
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
  }



  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Try* e)
  {
    sched::exception_ptr exception;
    try
    {
      return operator()(e->body_get().get());
    }
    catch (object::UrbiException& exn)
    {
      exception = exn.clone();
    }


    rObject value =
      static_cast<object::UrbiException*>(exception.get())->value_get();
    foreach (ast::rCatch handler, e->handlers_get())
    {
      if (handler->match_get())
      {
        rObject pattern = operator()(handler->match_get()->pattern_get().get());
        if (!is_true(pattern->call(SYMBOL(match), value)))
          continue;
        operator()(handler->match_get()->bindings_get().get());
        if (handler->match_get()->guard_get()
            && !is_true(operator()(handler->match_get()->guard_get().get())))
        {
          // Clear pattern
          pattern = operator()(handler->match_get()->pattern_get().get());
          continue;
        }
      }
      libport::Finally f(libport::scoped_set(current_exception_, value));
      return operator()(handler->body_get().get());
    }
    // No handler matched, rethrow.
    exception->rethrow();
    unreached();
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

  // Invalid nodes
#define INVALID(Node)                                                   \
  LIBPORT_SPEED_INLINE object::rObject                                  \
  Interpreter::visit(const ast::Node* n)                                \
  {                                                                     \
    static_cast<void>(n);                                               \
    pabort("Invalid " #Node " node in the Interpreter: " << *n);        \
  }                                                                     \

  INVALID(Assign);
  INVALID(Assignment);
  INVALID(Binding);
  INVALID(Break);
  INVALID(Catch);
  INVALID(Class);
  INVALID(Continue);
  INVALID(Declaration);
  INVALID(Decrementation);
  INVALID(Emit);
  INVALID(Foreach);
  INVALID(Incrementation);
  INVALID(Match);
  INVALID(MetaArgs);
  INVALID(MetaCall);
  INVALID(MetaExp);
  INVALID(MetaId);
  INVALID(MetaLValue);
  INVALID(OpAssignment);
  INVALID(Return);
  INVALID(Subscript);
  INVALID(Unscope);

#undef INVALID
}


#endif
