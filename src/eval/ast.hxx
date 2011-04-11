/*
 * Copyright (C) 2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file runner/eval/ast.hxx
 ** \brief Definition of eval::ast.
 */

#ifndef EVAL_AST_HXX
# define EVAL_AST_HXX

# include <boost/scoped_ptr.hpp>

# include <libport/bind.hh>
# include <libport/compilation.hh>
# include <libport/finally.hh>
# include <libport/foreach.hh>
# include <libport/format.hh>

# include <sched/exception.hh>

# include <kernel/uconnection.hh>
# include <kernel/userver.hh>

# include <ast/visitor.hh>

# include <object/symbols.hh>
# include <object/system.hh>

# include <urbi/object/code.hh>
# include <urbi/object/event.hh>
# include <urbi/object/event-handler.hh>
# include <urbi/object/global.hh>
# include <urbi/object/list.hh>
# include <urbi/object/dictionary.hh>
# include <urbi/object/object.hh>
# include <urbi/object/tag.hh>
# include <urbi/object/slot.hh>
# include <urbi/object/string.hh>

# include <urbi/runner/raise.hh>

# include <runner/state.hh>
# include <runner/job.hh>

# include <eval/ast.hh>
# include <eval/call.hh>
# include <eval/raise.hh>
# include <eval/send-message.hh>


namespace eval
{
  namespace ast_impl
  {
    using boost::bind;
    using libport::Finally;
    using object::rSlot;
    using object::rTag;
    using object::Slot;
    using object::Tag;

    GD_CATEGORY(Urbi.Ast);

    LIBPORT_SCOPE_SET_DECLARE(bool, bool);

    struct Visitor
    // : public ast::ConstVisitor
    {
      Visitor(Job& job);

      Job& this_;

      struct WatchEventData;
      static rObject watch_eval(WatchEventData* data);
      static void
      watch_run(WatchEventData* data,
                const object::objects_type& = object::objects_type());
      static void watch_stop(WatchEventData* data);
      static void watch_ward(WatchEventData* data);
      static void watch_unward(WatchEventData* data);

      static void
      at_run(WatchEventData* data,
             const object::objects_type& = object::objects_type());


      static object::rCode
      make_routine(ast::rConstRoutine e);


      static void
      tags_trigger_leave(const std::vector<object::rTag>& tags);

      typedef std::vector<libport::Symbol> tag_chain_type;

      static tag_chain_type decompose_tag_chain(ast::rConstExp e);

      object::rObject eval_tag(ast::rConstExp e);



#define FINALLY_Scope(DefineOrUse)                          \
    FINALLY_ ## DefineOrUse                                 \
    (Scope,                                                 \
     ((Job&, this_))                                        \
     ((bool, non_interruptible))                            \
     ((bool, redefinition_mode))                            \
     ((bool, void_error)),                                  \
     this_.state.cleanup_scope_tag(this_.scheduler_get());  \
     this_.non_interruptible_set(non_interruptible);        \
     this_.state.redefinition_mode_set(redefinition_mode);  \
     this_.state.void_error_set(void_error);                \
   )

#define FINALLY_Do(DefineOrUse)                 \
    FINALLY_ ## DefineOrUse                     \
    (Do,                                        \
     ((Job&, this_))                            \
     ((rObject&, old_tgt)),                     \
     this_.state.this_switch(old_tgt))

#define FINALLY_Try(DefineOrUse)                \
    FINALLY_ ## DefineOrUse                     \
    (Try,                                       \
     ((Job&, this_))                            \
     ((rObject&, old_exception)),               \
     this_.state.current_exception_set(old_exception))

    FINALLY_Do(DEFINE);
    FINALLY_Scope(DEFINE);
    FINALLY_Try(DEFINE);


# define VISIT(Macro, Data, Node)         \
      LIBPORT_SPEED_ALWAYS_INLINE rObject \
      visit(const ast::Node* n);

      AST_FOR_EACH_NODE(VISIT);
#undef VISIT
    };


    LIBPORT_SPEED_ALWAYS_INLINE
    Visitor::Visitor(Job& job)
      : this_(job)
    {
    }


    // Handle:
    // closure() { ... } & closure() { ... } & closure() { ... }
    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::And* e)
    {
      sched::Job::Collector collector(&this_, e->children_get().size() - 1);

      // Create separate runners for every child but the first
      foreach (const ast::rConstExp& child,
               boost::make_iterator_range(e->children_get(), 1, 0))
      {
        Job* job =
          this_.spawn_child(call(ast(this_, child.get())),
                            collector)
          ->name_set(libport::Symbol::fresh_string(this_.name_get()));

        if (this_.is_profiling())
          job->profile_fork(this_);
        job->start_job();
      }

      // We fetch the code of the closure.
      object::rCode code =
        ast(this_, e->children_get().front().get())
        .unsafe_cast<object::Code>();
      aver(code);

      try
      {
        // Evaluate the first child in this runner
        object::objects_type args;
        // FIXME: This is a closure, it won't use its 'this', but this is
        // gory.
        args << rObject();
        call_funargs(this_, code, libport::Symbol::make_empty(), args);
        // Wait for all other jobs to terminate.
        this_.yield_until_terminated(collector);
      }
      catch (const sched::ChildException& ce)
      {
        // If a child caused us to die, then throw the encapsulated exception.
        ce.rethrow_child_exception();
      }

      if (this_.is_profiling())
      {
        foreach (libport::intrusive_ptr<sched::Job> job, collector)
        {
          // We are sure these jobs are Interpeters.
          Job* i = static_cast<Job*>(job.get());
          if (i->is_profiling())
            this_.profile_join(*i);
        }
      }

      return object::void_class;
    }



    static inline void check_void(const object::rObject& v)
    {
      if (v == object::void_class)
        runner::raise_unexpected_void_error();
    }



    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::LocalAssignment* e)
    {
      rObject val = ast(this_, e->value_get().get());
      check_void(val);
      this_.state.set(e, val);

      return val;
    }


    typedef std::pair<object::Event*, object::Event::Subscription> Subscription;
    struct Visitor::WatchEventData
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


  LIBPORT_SPEED_ALWAYS_INLINE object::rObject
  Visitor::watch_eval(WatchEventData* data)
  {
    runner::Job& r = ::kernel::interpreter();
    GD_CATEGORY(Urbi.At);
    GD_FPUSH_TRACE("Evaluating watch expression: %s",
                   data->exp->body_string());

    rObject v;
    // Do not leave dependencies_log to true while registering on
    // dependencies below.
    {

      object::objects_type args;
      args.push_back(data->exp);
      bool profiled = false;
      bool interruptible = r.non_interruptible_get();
      try
      {
        const ast::Routine* ast = dynamic_cast<const ast::Routine*>
          (data->exp->ast_get().get());
        assert(ast);
        libport::Symbol name(libport::format("at: %s", *ast->body_get()));
        if (!r.is_profiling() && data->profile)
        {
          profiled = true;
          r.profile_start(data->profile, name, data->exp.get());
        }
        r.dependencies_log_set(true);
        r.non_interruptible_set(true);
        v = eval::call_apply(r, data->exp, name, args);
        r.non_interruptible_set(interruptible);
        r.dependencies_log_set(false);
        if (profiled)
          r.profile_stop();
      }
      catch (...)
      {
        r.non_interruptible_set(interruptible);
        r.dependencies_log_set(false);
        if (profiled)
          r.profile_stop();
        throw;
      }
      unsigned hooks_removed = 0;
      std::vector<Subscription>::iterator it = data->subscriptions.begin();
      while (it != data->subscriptions.end())
      {
        Job::dependencies_type::iterator find =
          r.dependencies().find(it->first);
        if (find != r.dependencies().end())
        {
          r.dependencies().erase(find);
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
                     r.dependencies().size() + data->subscriptions.size(),
                     r.dependencies().size(), hooks_removed);
    }
    return v;
  }

  inline void
  Visitor::watch_run(WatchEventData* data, const object::objects_type&)
  {
    GD_CATEGORY(Urbi.At);
    GD_FPUSH_TRACE("Evaluating watch event expression: %s",
                   data->exp->body_string());

    runner::Job& r = ::kernel::interpreter();
    rObject v = watch_eval(data);
    foreach (object::Event* evt, r.dependencies())
      data->subscriptions <<
      Subscription(evt, evt->onEvent(boost::bind(watch_run, data, _1)));
    r.dependencies().clear();
    object::objects_type args;
    args << v;
    data->event->emit(args);
  }

  inline void
  Visitor::watch_stop(Visitor::WatchEventData* data)
  {
    GD_CATEGORY(Urbi.At);
    GD_FPUSH_TRACE("Stopping watch event: %s", data->exp->body_string());

    foreach (Subscription& s, data->subscriptions)
      s.second.stop();
    delete data;
  }

  inline void
  Visitor::watch_ward(Visitor::WatchEventData* data)
  {
    GD_CATEGORY(Urbi.At);
    GD_FPUSH_TRACE("Subscribed watch event: %s", data->exp->body_string());
    data->event_ward = data->event;
  }

  inline void
  Visitor::watch_unward(Visitor::WatchEventData* data)
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
    Visitor::at_run(WatchEventData* data, const object::objects_type&)
    {
      GD_CATEGORY(Urbi.At);

      // FIXME: what is the kernel main interpreter in the new
      // implementation ?!
      Job& r = ::kernel::interpreter();

      rObject res = watch_eval(data);
      if (dynamic_cast<object::Event*>(res.get()))
      {
        CAPTURE_GLOBAL(Global);
        Global->call("warn", new object::String(
                       "at (<event>) without a '?', "
                       "this is probably not what you meant."));
      }

      bool v = object::from_urbi<bool>(res);
      foreach (object::Event* evt, r.dependencies())
        data->subscriptions <<
        Subscription(evt, evt->onEvent(boost::bind(at_run, data, _1)));
      r.dependencies_clear();

      // Check for different evaluation of the condition.
      if (v && !data->current)
      {
        GD_PUSH_TRACE("Triggering at block (enter)");
        data->current = data->event->trigger(object::objects_type()).get();
      }
      else if (!v && data->current)
      {
        GD_PUSH_TRACE("Triggering at block (exit)");
        data->current->stop();
        data->current = 0;
      }
    }

#define URBI_EVENT_VISIT(Type, Fun)                                     \
    LIBPORT_SPEED_ALWAYS_INLINE rObject                                 \
    Visitor::visit(const ast::Type* e)                                  \
    {                                                                   \
      /* Code of the condition associated to this event AST node. */    \
      object::rCode code = dynamic_cast<object::Code*>                  \
        (ast(this_, e->exp_get().get()).get());                         \
                                                                        \
      GD_CATEGORY(Urbi.At);                                             \
      GD_FPUSH_TRACE("Create watch event: %s", code->body_string());    \
                                                                        \
      object::rEvent res = new object::Event;                           \
      WatchEventData* data =                                            \
        new WatchEventData(res, code);                                  \
      data->profile = this_.profile_get();                               \
      res->destructed_get().connect(boost::bind(&watch_stop, data));    \
      /* Maintain that event alive as long as it is subscribed to. */   \
      res->subscribed_get().connect(boost::bind(watch_ward, data));     \
      res->unsubscribed_get().connect(boost::bind(watch_unward, data)); \
      /* Test it a first time. */                                       \
      Fun(data);                                                        \
                                                                        \
      return res;                                                       \
    }                                                                   \

    URBI_EVENT_VISIT(Watch, watch_run);
    URBI_EVENT_VISIT(Event, at_run);
#undef URBI_EVENT_VISIT


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Call* e)
    {
      // The invoked slot (probably a function).
      const ast::rConstExp& ast_tgt = e->target_get();
      rObject tgt = ast(this_, ast_tgt.get());
      return call_msg(this_,
                      tgt, e->name_get(),
                      e->arguments_get(),
                      e->location_get());
    }

    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::CallMsg*)
    {
      return this_.state.call();
    }

    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Implicit*)
    {
      return this_.state.this_get();
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::LocalDeclaration* d)
    {
      ast::rExp v = d->value_get();
      rObject res = v ? ast(this_, v.get()) : object::void_class;
      if (v)
        check_void(res);
      this_.state.def(d, res, d->constant_get());
      return res;
    }

    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Finally* f)
    {
      rObject res = object::void_class;
      try
      {
        res = ast(this_, f->body_get().get());
      }
      catch (...)
      {
        ast(this_, f->finally_get().get());
        throw;
      }
      ast(this_, f->finally_get().get());
      return res;
    }

    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Float* e)
    {
      return new object::Float(e->value_get());
    }

    LIBPORT_SPEED_ALWAYS_INLINE
    object::rCode
    Visitor::make_routine(ast::rConstRoutine e)
    {
      return new object::Code(e);
    }

    // foo = function (var a) { ... }
    // bar = closure (var a) { ... }
    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Routine* e)
    {
      object::rCode res = make_routine(e);

      // Capture variables.
      foreach (const ast::rLocalDeclaration& dec,
               *e->captured_variables_get())
      {
        ast::rLocal local = dec->value_get().unsafe_cast<ast::Local>();
        aver(local);
        res->captures_get() << this_.state.rget(local);
      }

      // Capture 'this' and 'call' in closures.
      if (e->closure_get())
      {
        res->this_set(this_.state.this_get());
        res->call_get() = this_.state.call();
        res->lobby_set(this_.state.lobby_get());
      }

      return res;
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::If* e)
    {
      if (ast(this_, e->test_get().get())->as_bool())
        return ast(this_, e->thenclause_get());
      else
        return ast(this_, e->elseclause_get());

      /*
      e->test_get().get()->visit(*this);
      if (result->as_bool())
        e->thenclause_get()->visit(*this);
      else
        e->elseclause_get()->visit(*this);
      */
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::List* e)
    {
      object::List::value_type res;
      // Evaluate every expression in the list.
      foreach (const ast::rConstExp& c, e->value_get())
      {
        rObject v = ast(this_, c.get());
        // Refuse void in literal lists.
        if (v == object::void_class)
          runner::raise_unexpected_void_error();
        res << v;
      }
      return new object::List(res);
    }

    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Local* e)
    {
      rSlot slot = this_.state.rget(e);
      rObject value = slot->value();

      passert("Local variable read before being set", value);

      if (e->arguments_get())
        return call_msg(this_,
                        this_.state.this_get(), value,
                        e->name_get(), e->arguments_get(),
                        e->location_get());
      else
      {
        if (this_.dependencies_log_get())
        {
          try
          {
            this_.dependencies_log_set(false);
            GD_CATEGORY(Urbi.At);
            GD_FPUSH_DEBUG("Register local variable '%s' for at monitoring",
                           e->name_get());
            object::Event* evt = static_cast<object::Event*>
              (slot->property_get(SYMBOL(changed)).get());
            this_.dependencies_log_set(true);
            this_.dependency_add(evt);
          }
          catch (...)
          {
            this_.dependencies_log_set(true);
            throw;
          }
        }

        return value;
      }
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Nary* e)
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

      sched::Job::Collector collector(&this_);

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
            this_.yield();

          const ast::Stmt* stmt = dynamic_cast<const ast::Stmt*>(c.get());
          const ast::Exp* exp = stmt ? stmt->expression_get().get() : c.get();

          if (stmt && stmt->flavor_get() == ast::flavor_comma)
          {
            // The new runners are attached to the same tags as we are.
            sched::rJob subrunner =
              this_.spawn_child(
                call(ast(this_, exp)),
                collector)
              ->name_set(
                libport::Symbol::fresh_string(this_.name_get()));
            subrunner->start_job();
          }
          else
          {
            res = ast(this_, exp);
          }
        }

        // If we get a scope tag, stop the runners tagged with it. (should be
        // a function close to State::scope_tag)
        if (const sched::rTag& tag = this_.state.scope_tag_get())
          tag->stop(this_.scheduler_get(), object::void_class);

        this_.yield_until_terminated(collector);
      }
      catch (const sched::ChildException& ce)
      {
        ce.rethrow_child_exception();
      }

      return res;
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Noop*)
    {
      return object::void_class;
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Pipe* e)
    {
      object::rObject res = object::void_class;

      // Run children without yielding.
      foreach (const ast::rConstExp& child, e->children_get())
        res = ast(this_, child.get());

      return res;
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Property* p)
    {
      ast::rExp owner = p->owner_get();
      if (ast::rCall call = owner.unsafe_cast<ast::Call>())
      {
        rObject owner = ast(this_, call->target_get().get());
        return owner->call(SYMBOL(getProperty),
                           new object::String(call->name_get()),
                           new object::String(p->name_get()));
      }
      else if (ast::rLocal local = owner.unsafe_cast<ast::Local>())
      {
        rObject res = this_.state.rget(local)->property_get(p->name_get());
        return res ? res : object::void_class;
      }
      else
        pabort("Unrecognized property owner");
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::PropertyWrite* p)
    {
      rObject value = ast(this_, p->value_get().get());
      ast::rExp owner = p->owner_get();
      if (ast::rCall call = owner.unsafe_cast<ast::Call>())
      {
        rObject owner = ast(this_, call->target_get().get());
        return owner->call(SYMBOL(setProperty),
                           new object::String(call->name_get()),
                           new object::String(p->name_get()),
                           value);
      }
      else if (ast::rLocal local = owner.unsafe_cast<ast::Local>())
      {
        this_.state.rget(local)->property_set(p->name_get(), value);
        return value;
      }
      else
        pabort("Unrecognized property owner");
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Scope* e)
    {
      // What ?!
      bool non_interruptible = this_.non_interruptible_get();
      bool redefinition_mode = this_.state.redefinition_mode_get();
      bool void_error = this_.state.void_error_get();
      FINALLY_Scope(USE);

      this_.state.create_scope_tag();
      return ast(this_, e->body_get().get());
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Do* e)
    {
      rObject tgt = ast(this_, e->target_get().get());
      rObject old_tgt = this_.state.this_get();
      this_.state.this_switch(tgt);
      FINALLY_Do(USE);
      visit(static_cast<const ast::Scope*>(e));
      // This is arguable. Do, just like Scope, should maybe return
      // their last inner value.
      return tgt;
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Stmt* e)
    {
      return ast(this_, e->expression_get().get());
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::String* e)
    {
      return new object::String(e->value_get());
    }

    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Dictionary* e)
    {
      object::rDictionary res = new object::Dictionary();
      /// Dictionary with a base was caught at parse time.
      foreach (ast::dictionary_elts_type::value_type exp, e->value_get())
      {
        rObject k = ast(this_, exp.first.get());
        aver(k);
        rObject v = ast(this_, exp.second.get());
        aver(v);
        // Refuse void in literals.
        if (v == object::void_class || k == object::void_class)
          runner::raise_unexpected_void_error();
        res->set(k, v);
      }
      return res;
    }


    /// Trigger leave event of all \a tags
    LIBPORT_SPEED_ALWAYS_INLINE
    void
    Visitor::tags_trigger_leave(const std::vector<object::rTag>& tags)
    {
      // Start with the most specific tag in the derivation chain.
      foreach (const object::rTag& tag, tags)
        tag->triggerLeave();
    }

    // This function takes an expression and attempts to decompose it
    // into a list of identifiers. The resulting chain is stored in
    // reverse order, that is the most specific tag first.
    LIBPORT_SPEED_ALWAYS_INLINE
    Visitor::tag_chain_type
    Visitor::decompose_tag_chain(ast::rConstExp e)
    {
      tag_chain_type res;
      while (!e->implicit())
      {
        ast::rConstCall c = e.unsafe_cast<const ast::Call>();
        if (!c || c->arguments_get())
          runner::raise_urbi(SYMBOL(ImplicitTagComponent));
        res << c->name_get();
        e = c->target_get();
      }
      return res;
    }

    LIBPORT_SPEED_ALWAYS_INLINE
    object::rObject
    Visitor::eval_tag(ast::rConstExp e)
    {
      try
      {
        // Try to evaluate e as a normal expression.
        return ast(this_, e.get());
      }
      catch (object::UrbiException&)
      {
        // We got a lookup error. It means that we have to automatically
        // create the tag. In this case, we only accept k1 style tags,
        // i.e. chains of identifiers, excluding function calls.
        // The reason to do that is:
        //   - we do not want to mix k1 non-declared syntax with k2
        //     clean syntax for tags
        //   - we have no way to know whether the lookup error arrived
        //     in a function call or during the direct resolution of
        //     the name.

        // `Tag.tags' represents the top level tag.
        CAPTURE_GLOBAL2(Tag, tags);
        rObject parent = tags;
        rObject where = this_.state.this_get();
        tag_chain_type chain = decompose_tag_chain(e);
        rforeach (libport::Symbol elt, chain)
        {
          // Check whether the concerned level in the chain already
          // exists.
          if (rObject owner = where->slot_locate(elt).first)
          {
            GD_FINFO_DUMP("Component %s exists.", elt);
            where = owner->local_slot_get(elt)->value();
            if (object::Tag* parent_ = dynamic_cast<object::Tag*>(where.get()))
            {
              GD_INFO_DUMP("It is a tag, so use it as the new parent.");
              parent = parent_;
            }
          }
          else
          {
            // We have to create a new tag, which will be attached
            // to the upper level (hierarchical tags, implicitly
            // rooted by Tags).
            where = parent->call(SYMBOL(new), new object::String(elt));
            parent->slot_set(elt, where);
            parent = where;
          }
        }

        return where;
      }
      pabort("Unreachable");
    }


    /// Trigger leave event of all \a tags
    static void
    trigger_leave(const std::vector<object::rTag>& tags)
    {
      // Start with the most specific tag in the derivation chain.
      foreach (const object::rTag& tag, tags)
        tag->triggerLeave();
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::TaggedStmt* t)
    {
      // FIXME: might be simplified after type checking code is moved
      // to Object.
      object::rObject unchecked_tag = eval_tag(t->tag_get());
      object::from_urbi<object::Tag>(unchecked_tag);
      object::rTag urbi_tag = unchecked_tag->as<object::Tag>();

      size_t result_depth = this_.state.tag_stack_size();
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
          {
            return boost::any_cast<rObject>(
              urbi_tag->value_get()->payload_get());
          }
          // If tag is frozen, remember it.
          some_frozen = some_frozen || urbi_tag->value_get()->frozen();
          applied << urbi_tag;
          urbi_tag = urbi_tag->parent_get();
        } while (urbi_tag);
        // Apply the tags, starting with the uppermost one in the ancestry
        // chain so that a "stop" on a parent removes the corresponding
        // children.
        foreach (const rTag& tag, applied)
          this_.state.apply_tag(tag, &finally);
        // If one of the tags caused us to be frozen, let the scheduler
        // handle this properly to avoid duplicating the logic.
        if (some_frozen)
          this_.yield();

        // Start with the uppermost tag in the derivation chain.
        rObject res;
        try
        {
          rforeach (const object::rTag& tag, applied)
            tag->triggerEnter();
          res = ast(this_, t->exp_get().get());
        }
        catch (...)
        {
          trigger_leave(applied);
          throw;
        }
        trigger_leave(applied);
        return res;
      }
      catch (sched::StopException& e_)
      {
        // trigger_leave will yield, which might throw and crush our
        // exception so copy it.
        sched::StopException e(e_);

        // Rewind up to the appropriate depth.
        if (e.depth_get() < result_depth)
          throw;

        // If we are frozen, reenter the scheduler for a while.
        if (this_.frozen())
          this_.yield();
        // Extract the value from the exception.
        return boost::any_cast<rObject>(e.payload_get());
      }
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::This*)
    {
      return this_.state.this_get();
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Throw* e)
    {
      eval::raise(this_,
                  e->value_get()
                  ? ast(this_, e->value_get().get())
                  : this_.state.current_exception_get());
      pabort("Unreachable");
    }


    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::Try* e)
    {
      // Evaluate the body, catch the exception, and process it.
      sched::exception_ptr exception;
      object::rObject res;
      try
      {
        res = ast(this_, e->body_get().get());
      }
      catch (object::UrbiException& exn)
      {
        exception = exn.clone();
      }

      // Don't run the "else" clause in this "try", as exceptions in
      // this "else" are not covered by the "try".
      if (!exception.get())
      {
        return (e->elseclause_get()
                ? ast(this_, e->elseclause_get().get())
                : res);
      }

      rObject value =
        static_cast<object::UrbiException*>(exception.get())->value_get();

      // Find the right handler.
      foreach (ast::rCatch handler, e->handlers_get())
      {
        if (handler->match_get())
        {
          rObject pattern =
            ast(this_, handler->match_get()->pattern_get().get());
          if (!pattern->call(SYMBOL(match), value)->as_bool())
            continue;
          ast(this_, handler->match_get()->bindings_get().get());
          if (handler->match_get()->guard_get()
              && !ast(this_,
                      handler->match_get()->guard_get().get())->as_bool())
          {
            // Clear pattern
            pattern = ast(this_, handler->match_get()->pattern_get().get());
            continue;
          }
        }
        rObject old_exception = this_.state.current_exception_get();
        FINALLY_Try(USE);
        this_.state.current_exception_set(value);
        return ast(this_, handler->body_get().get());
      }
      // No handler matched, rethrow.
      exception->rethrow();
      unreachable();
    }



    /// FIXME: There is a lot in common with Nary, factor.
    LIBPORT_SPEED_ALWAYS_INLINE rObject
    Visitor::visit(const ast::While* e)
    {
      const bool must_yield = e->flavor_get() != ast::flavor_pipe;
      bool tail = false;

      sched::Job::Collector collector(&this_);

      try
      {
        while (true)
        {
          if (must_yield && tail++)
            this_.yield();
          if (!ast(this_, e->test_get().get())->as_bool())
            break;
          if (e->flavor_get() == ast::flavor_comma)
          {
            // Collect jobs that are already terminated.
            collector.collect();
            // The new runners are attached to the same tags as we are.
            sched::rJob subrunner =
              this_.spawn_child(
                call(ast(this_, e->body_get().get())),
                collector)
              ->name_set(
                libport::Symbol::fresh_string(this_.name_get()));
            subrunner->start_job();
          }
          else
            visit(e->body_get());
        }
      }
      catch (const sched::ChildException& ce)
      {
        ce.rethrow_child_exception();
      }
      this_.yield_until_terminated(collector);

      return object::void_class;
    }



  // Invalid nodes
#define INVALID(Node)                                                   \
    LIBPORT_SPEED_ALWAYS_INLINE rObject                                 \
    Visitor::visit(const ast::Node* n)                                  \
    {                                                                   \
      LIBPORT_USE(n);                                                   \
      pabort("Interpreter: invalid " << #Node << " node: " << *n);      \
    }

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

#define IMPOSSIBLE(Node)                                                \
    LIBPORT_SPEED_ALWAYS_INLINE rObject                                 \
    Visitor::visit(const ast::Node* n)                                  \
    {                                                                   \
      LIBPORT_USE(n);                                                   \
      pabort("Interpreter: unreachable " << #Node << " node: " << *n);  \
    }

    IMPOSSIBLE(Ast);
    IMPOSSIBLE(Composite);
    IMPOSSIBLE(Exp);
    IMPOSSIBLE(Flavored);
    IMPOSSIBLE(LValue);
    IMPOSSIBLE(LValueArgs);
    IMPOSSIBLE(LocalWrite);
    IMPOSSIBLE(PropertyAction);
    IMPOSSIBLE(Unary);
    IMPOSSIBLE(Write);
#undef IMPOSSIBLE

# define VISIT(Macro, Data, Node)               \
    LIBPORT_SPEED_ALWAYS_INLINE rObject         \
    eval(Job& this_, const ast::Node* n)        \
    {                                           \
      Visitor v(this_);                         \
      return v.visit(n);                        \
    }

    AST_FOR_EACH_NODE(VISIT);
#undef VISIT
  }


#define FINALLY_Context(DefineOrUse)                    \
    FINALLY_ ## DefineOrUse                             \
    (Context,                                           \
      ((Job&, job))                                     \
      ((const runner::State::var_context_type&, ctx)),  \
     job.state.pop_context(ctx);                        \
    )

  FINALLY_Context(DEFINE);

  LIBPORT_SPEED_INLINE
  rObject ast_context(Job& job, const ast::Ast* e, rObject self)
  {
    typedef runner::State::var_context_type var_context_type;
    var_context_type ctx = job.state.push_context(self);
    FINALLY_Context(USE);
    return ast(job, e);
  }


#define FINALLY_Ast(DefineOrUse)                                        \
  FINALLY_ ## DefineOrUse(Ast,                                          \
                          ((Job&, job))                                 \
                          ((const ast::Ast*, previous)),                \
                          job.state.innermost_node_set(previous));

  FINALLY_Ast(DEFINE);

  // !!! GD_* macros are commented because this consume stack space in speed
  // mode, even if messages are not printed.

  LIBPORT_SPEED_INLINE
  rObject ast(Job& job, ast::rConstAst n)
  {
    // GD_CATEGORY(Eval.Ast);

    // GD_FPUSH_TRACE("Ast %s (%s)",
    //                n->node_type(),
    //                string_cast(n->location_get()));
    const ast::Ast* previous = job.state.innermost_node_get();
    FINALLY_Ast(USE);
    job.state.innermost_node_set(n.get());

    // let the AST node bounce on the ast_impl::eval functions
    return n->eval(job);
  }

  LIBPORT_SPEED_ALWAYS_INLINE
  Action ast(ast::rConstAst n)
  {
    return boost::bind(&ast, _1, n);
  }

} // namespace eval


#endif // ! EVAL_AST_HXX
