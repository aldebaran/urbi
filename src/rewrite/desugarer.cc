/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <libport/finally.hh>

#include <ast/cloner.hxx>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>

#include <urbi/object/symbols.hh>

#include <ast/factory.hh>
#include <ast/event-match.hh>
#include <parser/parse.hh>

#include <rewrite/desugarer.hh>
#include <rewrite/pattern-binder.hh>

DECLARE_LOCATION_FILE;

namespace rewrite
{
  Desugarer::Desugarer()
    : pattern_(false)
    , allow_decl_(false)
    , allow_subdecl_(false)
  {}


  template <typename T>
  libport::intrusive_ptr<typename boost::remove_const<T>::type>
  Desugarer::recurse_with_subdecl(T* s)
  {
    libport::Finally finally;
    finally << libport::scoped_set(allow_subdecl_, true);
    super_type::visit(s);
    passert(*s, result_);
    typedef typename boost::remove_const<T>::type T_no_const;
    return result_.unsafe_cast<T_no_const>();
  }

  void
  Desugarer::err(const ast::loc& loc, const std::string& msg)
  {
    errors_.err(loc, msg, "syntax error");
  }

  template <typename T>
  libport::intrusive_ptr<typename boost::remove_const<T>::type>
  Desugarer::recurse_with_subdecl(libport::intrusive_ptr<T> s)
  {
    libport::Finally finally;
    finally << libport::scoped_set(allow_subdecl_, true);
    aver(s);
    return recurse(s);
  }

  void Desugarer::visit(const ast::And* s)
  {
    recurse_with_subdecl(s);
  }

  void
  Desugarer::desugar_modifiers(const ast::Assign* assign)
  {
    ast::rLValue what = assign->what_get().unsafe_cast<ast::LValue>();
    if (!what)
      err(what->location_get(),
          "cannot use modifiers on pattern assignments");

    ast::rConstBinding binding = what.unsafe_cast<const ast::Binding>();
    if (binding)
      what = binding->what_get();

    const ast::loc& loc = assign->location_get();
    ast::rLValue tgt = factory_->make_lvalue_once(what);

    // Complete the dictionary with the base, getter and setter.
    // FIXME: There is a copy here that we would like to get rid of.
    ast::modifiers_type source = *assign->modifiers_get();
    source[SYMBOL(base)] = assign->value_get();
    source[SYMBOL(getter)] = factory_->make_closure(new_clone(tgt));
    PARAMETRIC_AST(setter, "closure (v) { %exp:1 }");
    source[SYMBOL(setter)] =
      exp(setter
          % new ast::Assign(loc, new_clone(tgt),
                            factory_->make_call(loc, SYMBOL(v)), 0));

    // Convert into a dictionary.  Sort it to improve determinism in
    // the desugared output.  The cost is neglectible.
    ast::rExp dict;
    {
      std::vector<libport::Symbol> keys;
      foreach (const ast::modifiers_type::value_type& i, source)
        keys << i.first;
      std::sort(keys.begin(), keys.end());

      ast::dictionary_elts_type d;
      foreach (libport::Symbol k, keys)
        d << ast::dictionary_elt_type(new ast::String(loc, k),
                                      recurse(source[k]));
      dict = new ast::Dictionary(loc, d);
    }
    PARAMETRIC_AST(traj, "TrajectoryGenerator.new(%exp:1).run");
    ast::rExp res(factory_->make_lvalue_wrap(what, exp(traj % dict).get()));
    // Parametric ASTs forward the location of the user code to the
    // desugared code.  In the case of trajectories, this is a
    // nuisance, as it results in errors in "run" calls _not_ being
    // flagged as system errors.  So restore a location that really
    // means it's a system feature that should be hidden from the user
    // in the back traces.
    res->location_set(LOCATION_HERE);

    if (binding)
    {
      PARAMETRIC_AST(declare, "var %lvalue:1 | %exp:2");
      res = exp(declare % what % res);
    }

    res->original_set(assign);
    result_ = recurse(res);
  }

  void
  Desugarer::visit(const ast::Assign* assign)
  {
    ast::loc loc = assign->location_get();

    // Handle modifiers.
    if (assign->modifiers_get())
      return desugar_modifiers(assign);
    aver(!assign->modifiers_get());

    // Simple declaration: var x = value.
    if (ast::rBinding what = assign->what_get().unsafe_cast<ast::Binding>())
    {
      if (!allow_decl_)
        err(what->location_get(), "declaration not allowed here");
      ast::rDeclaration res =
        new ast::Declaration(loc, what->what_get(), assign->value_get());
      res->constant_set(what->constant_get());
      result_ = recurse(res);
      return;
    }

    // Simple assignment: x = value.
    if (ast::rCall call = assign->what_get().unsafe_cast<ast::Call>())
    {
      // except for f(...) which is considered as a pattern matching.
      if (!call->arguments_get())
      {
        result_ = new ast::Assignment(loc, call, assign->value_get());
        result_ = recurse(result_);
        return;
      }
    }

    // Subscript assignment: x[y] = value.
    if (ast::rSubscript sub =
        assign->what_get().unsafe_cast<ast::Subscript>())
    {
      ast::exps_type* args = maybe_recurse_collection(sub->arguments_get());
      *args << recurse(assign->value_get());
      result_ = factory_->make_call(loc, recurse(sub->target_get()),
                                    SYMBOL(SBL_SBR_EQ), args);
      return;
    }

    // Property assignment: x->prop = value.
    if (ast::rProperty prop =
        assign->what_get().unsafe_cast<ast::Property>())
    {
      result_ = new ast::PropertyWrite(prop->location_get(),
                                       recurse(prop->owner_get()),
                                       prop->name_get(),
                                       recurse(assign->value_get()));
      return;
    }

    PARAMETRIC_AST
      (rewrite,
       "{\n"
       "  var '$value' = %exp:2 |\n"
       "  var '$pattern' = Pattern.new(%exp:1) |\n"
       "  if (!'$pattern'.match('$value'))\n"
       "    throw Exception.MatchFailure.new |\n"
       "  {\n"
       "    %unscope:2 |\n"
       "    %exp:3\n"
       "  } |\n"
       "  '$value'\n"
       "}\n"
      );

    ast::rExp pattern = assign->what_get();
    rewrite::PatternBinder bind
      (factory_->make_call(loc, SYMBOL(DOLLAR_pattern)), loc);
    bind(pattern.get());

    rewrite.location_set(assign->location_get());
    ast::rExp res = exp(rewrite
                        % bind.result_get().unchecked_cast<ast::Exp>()
                        % assign->value_get()
                        % bind.bindings_get());
    res->location_set(assign->location_get());
    result_ = recurse(res);
  }

  void
  Desugarer::visit(const ast::At* at)
  {
    ast::Factory factory;

    ast::loc loc = at->location_get();
    ast::loc cond_loc = at->cond_get()->location_get();
    ast::rRoutine closure =
      new ast::Routine(cond_loc, true, new ast::local_declarations_type,
                       factory.make_scope(cond_loc, at->cond_get()));

    ast::EventMatch match(new ast::Event(cond_loc, closure), 0, at->duration_get(), 0);
    result_ = factory.make_at_event(loc,
                                    at->flavor_location_get(), at->flavor_get(),
                                    at->sync_get(),
                                    match, at->body_get(), at->onleave_get());
    recurse(result_);
  }

  void Desugarer::operator()(const ast::Ast* node)
  {
    libport::Finally finally;
    finally << libport::scoped_set(allow_decl_, allow_subdecl_);
    finally << libport::scoped_set(allow_subdecl_, false);
    super_type::operator()(node);
  }

  void
  Desugarer::visit(const ast::Binding* binding)
  {
    if (!allow_decl_)
      err(binding->location_get(), "declaration not allowed here");
    ast::loc loc = binding->location_get();

    result_ = new ast::Declaration(loc, binding->what_get(), 0);
    result_ = recurse(result_);
  }

  void Desugarer::visit(const ast::Class* c)
  {
    ast::loc l = c->location_get();
    ast::rLValue what = recurse(c->what_get());
    libport::Symbol name = what->call()->name_get();

    // Bouncing to a 'class' function provides a simple means to make
    // sure that the "setProtos($exp:3)" is properly evaluated in the
    // context of the caller.  Otherwise, if put it inside the "do",
    // the protos will be looked for in the context of the newly
    // created object.
    PARAMETRIC_AST
      (desugar,
       "const var %lvalue:1 =\n"
       "  do (Object.'class'(%exp:2, %exp:3))\n"
       "  {\n"
       "    %exp:4\n"
       "  }\n"
       );

    desugar % what
      % factory_->make_string(l, name)
      % factory_->make_list(l, maybe_recurse_collection(c->protos_get()))
      % c->content_get();

    result_ = recurse_with_subdecl(exp(desugar));
    result_->original_set(c);
  }

  void Desugarer::visit(const ast::Decrementation* dec)
  {
    visit_dincrementation(dec->exp_get(), SYMBOL(MINUS_MINUS));
    result_->original_set(dec);
  }

  void Desugarer::visit(const ast::Do* s)
  {
    ast::rExp target = recurse(s->target_get());
    ast::rExp body = recurse_with_subdecl(s->body_get());
    result_ = new ast::Do(s->location_get(), body, target);
    result_->original_set(s);
  }

  void Desugarer::visit(const ast::Emit* e)
  {
    ast::rExp event = recurse(e->event_get());
    ast::exps_type* args = maybe_recurse_collection(e->arguments_get());

    if (ast::rExp duration = e->duration_get())
    {
      PARAMETRIC_AST
        (emit,
         "{\n"
         "  var '$emit'|\n"
         "  var '$duration' = %exp:3 |\n"
         "  try\n"
         "  {\n"
         "    '$emit' = %exp:1.trigger(%exps:2)|\n"
         "     sleep('$duration')|\n"
         "  }\n"
         "  finally\n"
         "  {\n"
         "    '$emit'.stop|\n"
         "  }\n"
         "}");

      // FIXME: children desugared twice
      result_ = recurse(exp(emit % event % args % duration));
    }
    else
    {
      PARAMETRIC_AST(emit, "%exp:1 . 'emit'(%exps:2)");
      result_ = exp(emit % event % args);
    }
    result_->original_set(e);
  }

  void Desugarer::visit(const ast::Incrementation* inc)
  {
    visit_dincrementation(inc->exp_get(), SYMBOL(PLUS_PLUS));
    result_->original_set(inc);
  }


  void Desugarer::visit_dincrementation(ast::rLValue what, libport::Symbol meth)
  {
    PARAMETRIC_AST(desugar, "{var '$save' = %exp:1 | %lvalue:2 = %exp:3.%id:4() | '$save'}");

    ast::rLValue tgt = factory_->make_lvalue_once(what);

    desugar % tgt % tgt % tgt % meth;
    result_ = factory_->make_lvalue_wrap(what, exp(desugar));
    result_ = recurse(result_);
  }

  void Desugarer::visit(const ast::If* s)
  {
    ast::rExp test = recurse_with_subdecl(s->test_get());
    ast::rScope thenclause = recurse(s->thenclause_get());
    ast::rScope elseclause = recurse(s->elseclause_get());
    result_ = new ast::If(s->location_get(), test, thenclause, elseclause);
    result_->original_set(s);
  }

  void Desugarer::visit(const ast::OpAssignment* a)
  {
    PARAMETRIC_AST(desugar, "%lvalue:1 = %exp:2.%id:3(%exp:4)");

    ast::rLValue what = a->what_get();
    ast::rLValue tgt = factory_->make_lvalue_once(what);

    desugar % tgt % tgt % a->op_get() % a->value_get();

    result_ = factory_->make_lvalue_wrap(what, exp(desugar));
    result_ = recurse(result_);
    result_->original_set(a);
  }

  void Desugarer::visit(const ast::Pipe* s)
  {
    recurse_with_subdecl(s);
  }

  void Desugarer::visit(const ast::Scope* s)
  {
    recurse_with_subdecl(s);
  }

  void Desugarer::visit(const ast::Stmt* s)
  {
    recurse_with_subdecl(s);
  }

  void Desugarer::visit(const ast::Subscript* s)
  {
    PARAMETRIC_AST(rewrite, "%exp:1 .'[]'(%exps:2)");
    // FIXME: arguments desugared twice
    rewrite.location_set(s->location_get());
    result_ = ast::exp(rewrite
                       % s->target_get()
                       % maybe_recurse_collection(s->arguments_get()));
    result_ = recurse(result_);
  }


  void
  Desugarer::visit(const ast::Catch* c)
  {
    const ast::loc& loc = c->location_get();
    ast::rMatch match = c->match_get();

    if (match)
    {
      rewrite::PatternBinder bind
        (factory_->make_call(loc, SYMBOL(DOLLAR_pattern)), loc);
      bind(c->match_get()->pattern_get().get());
      PARAMETRIC_AST(pattern, "var '$pattern' = Pattern.new(%exp:1)");
      ast::rExp p =
        exp(pattern % bind.result_get().unchecked_cast<ast::Exp>());
      p = recurse_with_subdecl(p);
      match = new ast::Match(loc, p, recurse(c->match_get()->guard_get()));
      match->bindings_set(recurse(bind.bindings_get()));
      match->original_set(c->match_get());
    }
    result_ = new ast::Catch(loc, match,
                             recurse_with_subdecl(c->body_get()));
  }

  void Desugarer::visit(const ast::Try* t)
  {
    result_ = factory_->make_try(t->location_get(),
                                 recurse(t->body_get()),
                                 recurse_collection(t->handlers_get()),
                                 recurse(t->elseclause_get()));
  }

  void Desugarer::visit(const ast::While* s)
  {
    recurse_with_subdecl(s);
  }

}
