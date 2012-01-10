/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
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
# include <libport/finally.hh>
# include <libport/foreach.hh>
# include <libport/format.hh>

# include <ast/all.hh>
# include <ast/print.hh>

# include <urbi/kernel/uconnection.hh>
# include <urbi/kernel/userver.hh>

# include <object/profile.hh>
# include <urbi/object/symbols.hh>

# include <runner/interpreter.hh>
# include <urbi/runner/raise.hh>

# include <sched/exception.hh>

# include <object/code.hh>
# include <urbi/object/event.hh>
# include <urbi/object/event-handler.hh>
# include <urbi/object/global.hh>
# include <urbi/object/list.hh>
# include <urbi/object/dictionary.hh>
# include <urbi/object/tag.hh>
# include <urbi/object/slot.hh>
# include <urbi/object/string.hh>

GD_CATEGORY(Urbi);

namespace runner
{
  using boost::bind;
  using libport::Finally;
  using object::rSlot;
  using object::rTag;
  using object::Slot;
  using object::Tag;

  LIBPORT_SCOPE_SET_DECLARE(bool, bool);

  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::visit(const ast::And* e)
  {
    Job::Collector collector(this, e->children_get().size() - 1);
    Profile* p = 0;

    // Create separate runners for every child but the first
    foreach (const ast::rConstExp& child,
             boost::make_iterator_range(e->children_get(), 1, 0))
    {
      Interpreter* job =
        new Interpreter(*this, operator()(child.get()),
                        libport::Symbol::fresh_string(name_get()));
      if (profile_)
      {
        p = new Profile;
        // Max function call depth has to start at current depth.
        p->function_call_depth_max_ = profile_->function_call_depth_max_;
        job->profile_start(p, libport::Symbol(), profile_function_current_);
      }
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

    if (profile_)
    {
      foreach (libport::intrusive_ptr<Job> job, collector)
      {
        // We are sure these jobs are Interpeters.
        Interpreter* i = static_cast<Interpreter*>(job.get());
        if (i->profile_)
        {
          i->profile_->step(i->profile_checkpoint_, i->profile_function_current_);
          *profile_ += *i->profile_;
          delete i->profile_;
          i->profile_ = 0;
        }
      }
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

  typedef std::pair<object::Event*, object::Event::Subscription> Subscription;
  struct Interpreter::WatchEventData
  {
    WatchEventData(urbi::object::rEvent ev, object::rCode e)
      : event(ev.get())
      , exp(e)
      , current(0)
      , subscriptions()
    {}

    object::Event* event;
    object::rEvent event_ward;
    object::rCode exp;
    object::EventHandler* current;
    std::vector<Subscription> subscriptions;
    object::rProfile profile;
  };

  LIBPORT_SPEED_INLINE object::rObject
  Interpreter::watch_eval(WatchEventData* data)
  {
    runner::Interpreter& r = ::kernel::interpreter();
    GD_CATEGORY(Urbi.At);
    GD_FPUSH_TRACE("Evaluating watch expression: %s",
                   data->exp->body_string());

    rObject v;
    // Do not leave dependencies_log to true while registering on dependencies
    // below.
    {

      object::objects_type args;
      args.push_back(data->exp);
      bool profiled = false;
      bool interruptible = r.non_interruptible_;
      try
      {
        const ast::Routine* ast = dynamic_cast<const ast::Routine*>
          (data->exp->ast_get().get());
        assert(ast);
        libport::Symbol name(libport::format("at: %s", *ast->body_get()));
        if (!r.profile_ && data->profile)
        {
          profiled = true;
          r.profile_start(data->profile, name, data->exp.get());
        }
        r.dependencies_log_ = true;
        r.non_interruptible_ = true;
        v = r.apply(data->exp, name, args);
        r.non_interruptible_ = interruptible;
        r.dependencies_log_ = false;
        if (profiled)
          r.profile_stop();
      }
      catch (...)
      {
        r.non_interruptible_ = interruptible;
        r.dependencies_log_ = false;
        if (profiled)
          r.profile_stop();
        throw;
      }
      unsigned hooks_removed = 0;
      std::vector<Subscription>::iterator it = data->subscriptions.begin();
      while (it != data->subscriptions.end())
      {
        dependencies_type::iterator find = r.dependencies_.find(it->first);
        if (find != r.dependencies_.end())
        {
          r.dependencies_.erase(find);
          ++it;
        }
        else
        {
          ++hooks_removed;
          it->second.stop();
          it = data->subscriptions.erase(it);
        }
      }
      GD_FINFO_DEBUG("Watch event has %s hooks: %s new, %s removed.",
                     r.dependencies_.size() + data->subscriptions.size(),
                     r.dependencies_.size(), hooks_removed);
    }
    return v;
  }

  inline void
  Interpreter::watch_run(WatchEventData* data, const object::objects_type&)
  {
    GD_CATEGORY(Urbi.At);
    GD_FPUSH_TRACE("Evaluating watch event expression: %s",
                   data->exp->body_string());

    runner::Interpreter& r = ::kernel::interpreter();
    rObject v = watch_eval(data);
    foreach (object::Event* evt, r.dependencies_)
      data->subscriptions <<
      Subscription(evt, evt->onEvent(boost::bind(watch_run, data, _1)));
    r.dependencies_.clear();
    object::objects_type args;
    args << v;
    data->event->emit(args);
  }

  inline void
  Interpreter::watch_stop(Interpreter::WatchEventData* data)
  {
    GD_CATEGORY(Urbi.At);
    GD_FPUSH_TRACE("Stopping watch event: %s", data->exp->body_string());

    foreach (Subscription& s, data->subscriptions)
      s.second.stop();
    delete data;
  }

  inline void
  Interpreter::watch_ward(Interpreter::WatchEventData* data)
  {
    GD_CATEGORY(Urbi.At);
    GD_FPUSH_TRACE("Subscribed watch event: %s", data->exp->body_string());
    data->event_ward = data->event;
  }

  inline void
  Interpreter::watch_unward(Interpreter::WatchEventData* data)
  {
    GD_CATEGORY(Urbi.At);
    GD_FPUSH_TRACE("Unsubscribed watch event: %s, %s",
                   data->exp->body_string(), data->event->counter_get());
    // If this event is alive only because it's up, terminate it.
    if (data->current && data->event->counter_get() == 2)
    {
      GD_FINFO_TRACE("KILL %s", data->current->counter_get());
      data->current->stop();
      GD_FINFO_TRACE("KILL %s, %s", data->current->counter_get(), data->event->counter_get());
    }
    data->event_ward = 0;
  }

  inline void
  Interpreter::at_run(WatchEventData* data, const object::objects_type&)
  {
    GD_CATEGORY(Urbi.At);

    runner::Interpreter& r = ::kernel::interpreter();
    rObject res = watch_eval(data);
    if (dynamic_cast<object::Event*>(res.get()))
    {
      CAPTURE_GLOBAL(Global);
      Global->call("warn", new object::String(
                     "at (<event>) without a '?', "
                     "this is probably not what you meant."));
    }
    bool v = object::from_urbi<bool>(res);
    foreach (object::Event* evt, r.dependencies_)
      data->subscriptions <<
      Subscription(evt, evt->onEvent(boost::bind(at_run, data, _1)));
    r.dependencies_.clear();
    if (v && !data->current)
    {
      GD_PUSH_TRACE("Triggering at block (enter)");
      data->current = data->event->trigger(object::objects_type()).get();
    }
    else if (!v && data->current)
    {
      GD_PUSH_TRACE("Triggering at block (exit)");

      object::EventHandler* current = data->current;
      data->current = 0;
      current->stop();
    }
  }

#define URBI_EVENT_VISIT(Type, Fun)                                     \
  LIBPORT_SPEED_INLINE object::rObject                                  \
  Interpreter::visit(const ast::Type* e)                                \
  {                                                                     \
    object::rCode code = dynamic_cast<object::Code*>                    \
      (operator()(e->exp_get().get()).get());                           \
                                                                        \
    GD_CATEGORY(Urbi.At);                                               \
    GD_FPUSH_TRACE("Create watch event: %s", code->body_string());      \
                                                                        \
    object::rEvent res = new object::Event;                             \
    WatchEventData* data =                                              \
      new WatchEventData(res, code);                                    \
    data->profile = profile_;                                           \
    res->destructed_get().connect(boost::bind(&watch_stop, data));      \
    /* Maintain that event alive as long as it is subscribed to. */     \
    res->subscribed_get().connect(boost::bind(watch_ward, data));       \
    res->unsubscribed_get().connect(boost::bind(watch_unward, data));   \
    /* Test it a first time. */                                         \
    Fun(data);                                                          \
                                                                        \
    return res;                                                         \
  }                                                                     \

  URBI_EVENT_VISIT(Watch, watch_run);
  URBI_EVENT_VISIT(Event, at_run);

#undef URBI_EVENT_VISIT

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
    return stacks_.this_get();
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
    return (operator()(e->test_get().get())->as_bool()
            ? e->thenclause_get()->eval(*this)
            : e->elseclause_get()->eval(*this));
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
      if (dependencies_log_)
      {
        try
        {
          dependencies_log_set(false);
          GD_CATEGORY(Urbi.At);
          GD_FPUSH_DEBUG("Register local variable '%s' for at monitoring",
                         e->name_get());
          object::Event* evt = static_cast<object::Event*>
            (slot->property_get(SYMBOL(changed)).get());
          dependencies_log_set(true);
          dependency_add(evt);
        }
        catch (...)
        {
          dependencies_log_set(true);
          throw;
        }
      }
      return value;
    }
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

    bool first = true;
    try
    {
      foreach (const ast::rConstExp& c, e->children_get())
      {
        // Allow some time to pass before we execute what follows.  If
        // we don't do this, the ;-operator would act almost like the
        // |-operator because it would always start to execute its RHS
        // immediately. However, we don't want to do it before the first
        // statement or if we only have one statement in the scope.
        if (!first)
          yield();
        first = false;

        const ast::Stmt* stmt = dynamic_cast<const ast::Stmt*>(c.get());
        const ast::Exp* exp = stmt ? stmt->expression_get().get() : c.get();

        if (stmt && stmt->flavor_get() == ast::flavor_comma)
        {
          // The new runners are attached to the same tags as we are.
          sched::rJob subrunner =
            new Interpreter(*this, operator()(exp),
                            libport::Symbol::fresh_string(name_get()));
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
    foreach (ast::dictionary_elts_type::value_type exp, e->value_get())
    {
      rObject k = operator()(exp.first.get());
      aver(k);
      rObject v = operator()(exp.second.get());
      aver(v);
      // Refuse void in literals.
      if (v == object::void_class || k == object::void_class)
        raise_unexpected_void_error();
      res->set(k, v);
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
    object::from_urbi<object::Tag>(unchecked_tag);
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
      // trigger_leave will yield, which might throw and crush our
      // exception so copy it.
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
    return stacks_.this_get();
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
    bool first = true;

    Job::Collector collector(this);

    try
    {
      while (true)
      {
        if (must_yield)
        {
          if (!first)
            yield();
          first = false;
        }
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
                            libport::Symbol::fresh_string(name_get()));
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
    pabort("invalid " #Node " node in the Interpreter: " << *n);        \
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
