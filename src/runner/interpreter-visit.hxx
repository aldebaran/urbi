/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#ifndef RUNNER_INTERPRETER_VISIT_HXX
# define RUNNER_INTERPRETER_VISIT_HXX

# include <boost/scoped_ptr.hpp>

# include <libport/bind.hh>
# include <libport/compilation.hh>
# include <libport/echo.hh>
# include <libport/finally.hh>
# include <libport/foreach.hh>
# include <libport/format.hh>

# include <ast/all.hh>
# include <ast/print.hh>

# include <kernel/uconnection.hh>
# include <kernel/userver.hh>

# include <object/symbols.hh>

# include <runner/interpreter.hh>
# include <urbi/runner/raise.hh>

# include <sched/exception.hh>

# include <urbi/object/code.hh>
# include <urbi/object/event.hh>
# include <urbi/object/global.hh>
# include <urbi/object/list.hh>
# include <urbi/object/dictionary.hh>
# include <urbi/object/tag.hh>
# include <urbi/object/slot.hh>
# include <urbi/object/string.hh>

GD_CATEGORY(Urbi);

/// Job echo.
#define JECHO(Title, Content)                                   \
  LIBPORT_DEBUG("job " << ME << ", " Title ": " << Content)

/// Job & astecho.
#define JAECHO(Title, Ast)                      \
  JECHO(Title, AST(Ast))

/* Yield and trace. */
#define YIELD()                                         \
  do {                                                  \
    LIBPORT_DEBUG("job " << ME << " yielding on AST: "  \
                  << &e << ' ' << AST(e));              \
    yield ();                                           \
  } while (false)


namespace urbi
{
  namespace object
  {
    extern bool squash;
  }
}

namespace runner
{
  using boost::bind;
  using libport::Finally;
  using object::rSlot;
  using object::rTag;
  using object::Slot;
  using object::Tag;

  static object::Event* slotGet_changed(object::rObject o)
  {
    object::rObject changed = o->call(SYMBOL(changed));
    aver(changed);
    object::rEvent evt = changed->as<object::Event>();
    aver(evt);
    return evt.get();
  }

  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::And* e)
  {
    Job::Collector collector(this, e->children_get().size() - 1);

    // Create separate runners for every child but the first
    foreach (const ast::rConstExp& child,
             boost::make_iterator_range(e->children_get(), 1, 0))
    {
      Interpreter* job =
        new Interpreter(*this, operator()(child.get()),
                        libport::Symbol::fresh(name_get()));
      register_child(job, collector);
      job->start_job();
    }

    rCode code = operator()(e->children_get().front().get())
      .unsafe_cast<object::Code>();
    aver(code);

    try
    {
      // Evaluate the first child in this runner
      object::objects_type args;
      // FIXME: This is a closure, it won't use its 'this', but this is
      // gory.
      args << rObject();
      apply_urbi(code, libport::Symbol::make_empty(), args, 0);
      // Wait for all other jobs to terminate.
      yield_until_terminated(collector);
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

  struct Interpreter::AtEventData
  {
    AtEventData(urbi::object::rEvent ev, object::rCode e)
      : event(ev)
      , exp(e)
      , current(0)
      , subscriptions()
    {}

    object::rEvent event;
    object::rCode exp;
    object::rEvent current;
    std::vector<object::Event::Subscription> subscriptions;
  };

  inline void
  Interpreter::at_run(AtEventData* data, const object::objects_type&)
  {
    runner::Interpreter& r = ::kernel::interpreter();
    bool v;
    // FIXME: optimize: do not unregister and reregister the same dependency
    {
      GD_FPUSH_TRACE("Evaluating at condition: %s", data->exp->body_string());
      foreach (object::Event::Subscription& s, data->subscriptions)
        s.stop();
      data->subscriptions.clear();

      bool& squash = object::squash;
      bool prev = squash;

      bool& dependencies_log = r.dependencies_log_;
      // Do not leave dependencies_log to true while registering on dependencies
      // below.
      {
        FINALLY_at_run(USE);
        dependencies_log = true;
        squash = false;

        object::objects_type args;
        args.push_back(data->exp);
        v = object::from_urbi<bool>(r.apply(data->exp, SYMBOL(at_cond), args));
      }
      foreach (object::rEvent evt, r.dependencies_)
        data->subscriptions << evt->onEvent(boost::bind(at_run, data, _1));
      r.dependencies_.clear();
    }
    if (v && !data->current)
    {
      GD_PUSH_TRACE("Triggering at block (enter)");
      data->current = data->event->trigger(object::objects_type());
    }
    else if (!v && data->current)
    {
      GD_PUSH_TRACE("Triggering at block (exit)");
      data->current->stop();
      data->current = 0;
    }
  }

  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Event* e)
  {
    object::rEvent res = new object::Event;
    at_run(new AtEventData
           (res,
            dynamic_cast<object::Code*>(operator()(e->exp_get().get()).get())));

    return res;
    // std::cerr << "VISIT" << std::endl;
    // std::cerr << *e << std::endl;
    // return object::void_class;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Call* e)
  {
    // The invoked slot (probably a function).
    const ast::rConstExp& ast_tgt = e->target_get();
    rObject tgt = operator()(ast_tgt.get());
    GD_FPUSH_TRACE("Call %s", e->name_get());
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
    object::rObject res = stacks_.this_get();
    if (!object::squash && dependencies_log_get())
      dependency_add(slotGet_changed(res));
    return res;
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
  Interpreter::visit(const ast::Finally* f)
  {
    rObject res;
    try
    {
      res = operator()(f->body_get().get());
      operator()(f->finally_get().get());
    }
    catch (...)
    {
      operator()(f->finally_get().get());
      throw;
    }
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
      aver(local);
      res->captures_get() << stacks_.rget(local);
    }

    // Capture 'this' and 'call' in closures.
    if (e->closure_get())
    {
      res->this_set(stacks_.this_get());
      res->call_get() = stacks_.call();
      res->lobby_set(lobby_get());
    }

    return res;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::If* e)
  {
    // Evaluate the test.
    JAECHO ("test", e->test_get ());
    rObject cond = operator()(e->test_get().get());

    if (cond->as_bool())
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
      res << v;
    }
    return new object::List(res);
  }

  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Local* e)
  {
    rSlot slot = stacks_.rget(e);
    rObject value = slot->value();

    passert("Local variable read before being set", value);

    if (e->arguments_get())
      return apply_ast(stacks_.this_get(), value,
                       e->name_get(), e->arguments_get(),
                       e->location_get());
    else
    {
      if (!object::squash && dependencies_log_get())
      {
        bool prev = object::squash;
        bool& squash = object::squash;
        FINALLY_Local(USE);
        squash = true;

        {
          GD_FPUSH_TRACE("Register local variable '%s' for at monitoring",
                         e->name_get());
          dependency_add(static_cast<object::Event*>(slot->property_get(SYMBOL(changed)).get()));
          dependency_add(slotGet_changed(*slot));
        }
      }

      return value;
    }
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Message* e)
  {
    // FIXME: Some more work is needed on syntactic exceptions.
    if (e->tag_get() == "error")
      raise_syntax_error(e->location_get(), e->text_get(), "FIXME:");
    else
      send_message(e->tag_get(),
                   libport::format("!!! %s: %s",
                                   e->location_get(), e->text_get()));
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

    // Optimize a common case where a Nary contains only one statement
    // not ending with a comma. This is for example the case of Nary
    // created by a "if" branch without braces. In this case, we will
    // make a tail-call to avoid cluttering the stack.
    if (exps.size() == 1)
    {
      const ast::Stmt* stmt =
        dynamic_cast<const ast::Stmt*>(exps.front().get());
      if (stmt && stmt->flavor_get() != ast::flavor_comma)
        return visit(stmt);
    }

    Job::Collector collector(this);

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
          register_child(subrunner, collector);
          subrunner->start_job();
        }
        else
        {
          res = operator()(exp);
        }
      }
      // If we get a scope tag, stop the runners tagged with it.
      if (const sched::rTag& tag = scope_tag_get())
        tag->stop(scheduler_get(), object::void_class);
      yield_until_terminated(collector);
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
    Interpreter* i = this;
    bool non_interruptible = non_interruptible_;
    bool redefinition_mode = redefinition_mode_get();
    bool void_error = void_error_get();
    FINALLY_Scope(USE);

    create_scope_tag();
    return operator()(e->body_get().get());
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Do* e)
  {
    rObject tgt = operator()(e->target_get().get());
    rObject old_tgt = stacks_.this_get();
    stacks_.this_switch(tgt);
    FINALLY_Do(USE);
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
    /// Dictionary with a base was caught at parse time.
    aver(!e->base_get());
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
        applied << urbi_tag;
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
    catch (sched::StopException& e_)
    {
      // trigger_leave will yield, which migth throw and crush our exception
      // so copy it
      sched::StopException e(e_);
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
    object::rObject res = stacks_.this_get();
    if (!object::squash && dependencies_log_get())
      dependency_add(slotGet_changed(res));
    return res;
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Throw* e)
  {
    raise(e->value_get()
          ? operator()(e->value_get().get())
          : current_exception_);
    pabort("Unreachable");
  }


  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::Try* e)
  {
    // Evaluate the body, catch the exception, and process it.
    sched::exception_ptr exception;
    object::rObject res;
    try
    {
      res = operator()(e->body_get().get());
    }
    catch (object::UrbiException& exn)
    {
      exception = exn.clone();
    }

    // Don't run the "else" clause in this "try", as exceptions in
    // this "else" are not covered by the "try".
    if (!exception.get())
      return (e->elseclause_get() ? operator()(e->elseclause_get().get())
              : res);

    rObject value =
      static_cast<object::UrbiException*>(exception.get())->value_get();

    // Find the right handler.
    foreach (ast::rCatch handler, e->handlers_get())
    {
      if (handler->match_get())
      {
        rObject pattern =
          operator()(handler->match_get()->pattern_get().get());
        if (!pattern->call(SYMBOL(match), value)->as_bool())
          continue;
        operator()(handler->match_get()->bindings_get().get());
        if (handler->match_get()->guard_get()
            && !operator()(handler->match_get()->guard_get().get())->as_bool())
        {
          // Clear pattern
          pattern = operator()(handler->match_get()->pattern_get().get());
          continue;
        }
      }
      rObject old_exception = current_exception_;
      FINALLY_Try(USE);
      current_exception_ = value;
      return operator()(handler->body_get().get());
    }
    // No handler matched, rethrow.
    exception->rethrow();
    unreachable();
  }


  /// FIXME: There is a lot in common with Nary, factor.
  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::While* e)
  {
    const bool must_yield = e->flavor_get() != ast::flavor_pipe;
    bool tail = false;

    Job::Collector collector(this);

    try
    {
      while (true)
      {
        if (must_yield && tail++)
          YIELD();
        if (!operator()(e->test_get().get())->as_bool())
          break;
        if (e->flavor_get() == ast::flavor_comma)
        {
          // Collect jobs that are already terminated.
          collector.collect();
          // The new runners are attached to the same tags as we are.
          sched::rJob subrunner =
            new Interpreter(*this,
                            operator()(e->body_get().get()),
                            libport::Symbol::fresh(name_get()));
          register_child(subrunner, collector);
          subrunner->start_job();
        }
        else
          e->body_get()->eval(*this);
      }
    }
    catch (const sched::ChildException& ce)
    {
      ce.rethrow_child_exception();
    }
    yield_until_terminated(collector);

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
  INVALID(At);
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
