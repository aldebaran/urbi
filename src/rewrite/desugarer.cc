#include <libport/finally.hh>

#include <ast/cloner.hxx>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>

#include <object/symbols.hh>

#include <parser/ast-factory.hh>
#include <parser/parse.hh>

#include <rewrite/desugarer.hh>
#include <rewrite/pattern-binder.hh>

namespace rewrite
{
  using parser::ast_call;
  using parser::ast_string;
  using parser::ast_lvalue_once;
  using parser::ast_lvalue_wrap;

  Desugarer::Desugarer()
    : pattern_(false)
    , allow_decl_(false)
    , allow_subdecl_(false)
  {}

  void Desugarer::visit(const ast::And* s)
  {
    allow_subdecl_ = true;
    super_type::visit(s);
  }

  void
  Desugarer::desugar_modifiers(const ast::Assign* assign)
  {
    const ast::loc& loc = assign->location_get();
    const ast::modifiers_type* source = assign->modifiers_get();
    ast::rLValue what = assign->what_get().unsafe_cast<ast::LValue>();
    ast::rConstBinding binding = what.unsafe_cast<const ast::Binding>();

    if (!what)
      errors_.error(what->location_get(),
                    "cannot use modifiers on pattern assignments");

    if (binding)
      what = binding->what_get();

    PARAMETRIC_AST(dict, "Dictionary.new");

    ast::rExp modifiers = exp(dict);
    foreach (const ast::modifiers_type::value_type& elt, *source)
    {
      PARAMETRIC_AST(add, "%exp:1.set(%exp:2, %exp:3)");

      add % modifiers
        % new ast::String(loc, elt.first)
        % recurse(elt.second);
      modifiers = exp(add);
    }

    ast::rExp target_value = recurse(assign->value_get());

    ast::rLValue tgt = ast_lvalue_once(what);
    PARAMETRIC_AST(trajectory,
                   "TrajectoryGenerator.new("
                   "  closure ( ) { %exp:1 }," // getter
                   "  closure (v) { %exp:2 }," // Setter
                   "  %exp:3," // Target value
                   "  %exp:4" // modifiers
                   ").run"
      );

    ast::rExp read = new_clone(tgt);
    ast::rExp write = new ast::Assignment(loc, new_clone(tgt),
                                          parser::ast_call(loc, SYMBOL(v)));

    trajectory
      % read
      % write
      % target_value
      % modifiers;

    ast::rExp res = ast::rExp(ast_lvalue_wrap(what, exp(trajectory)).get());

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

    // Handle modifiers
    if (assign->modifiers_get())
      return desugar_modifiers(assign);
    assert(!assign->modifiers_get());

    // Simple declaration: var x = value
    if (ast::rBinding what = assign->what_get().unsafe_cast<ast::Binding>())
    {
      if (!allow_decl_)
        errors_.error(what->location_get(), "declaration not allowed here");
      result_ = new ast::Declaration(loc, what->what_get(),
                                     assign->value_get());
      result_ = recurse(result_);
      return;
    }

    // Simple assignment: x = value
    if (ast::rCall call = assign->what_get().unsafe_cast<ast::Call>())
    {
      result_ = new ast::Assignment(loc, call, assign->value_get());
      result_ = recurse(result_);
      return;
    }

    // Subscript assignment: x[y] = value
    if (ast::rSubscript sub =
        assign->what_get().unsafe_cast<ast::Subscript>())
    {
      ast::exps_type* args = maybe_recurse_collection(sub->arguments_get());
      args->push_back(recurse(assign->value_get()));
      result_ = ast_call(loc, recurse(sub->target_get()),
                         SYMBOL(SBL_SBR_EQ), args);
      return;
    }

    // Property assignment: x->prop = value
    if (ast::rProperty prop =
        assign->what_get().unsafe_cast<ast::Property>())
    {
      result_ = new ast::PropertyWrite(prop->location_get(),
                                       prop->owner_get(),
                                       prop->name_get(),
                                       assign->value_get());
      return;
    }

    PARAMETRIC_AST(rewrite,
                   "{"
                   "  var '$value' = %exp:2 |"
                   "  var '$pattern' = Pattern.new(%exp:1) |"
                   "  if (!'$pattern'.match('$value'))"
                   "    throw MatchFailure.new() |"
                   "  {"
                   "    %unscope:2 |"
                   "    %exp:3"
                   "  } |"
                   "  '$value'"
                   "}"
      );

    ast::rExp pattern = assign->what_get();
    rewrite::PatternBinder bind
      (ast_call(loc, SYMBOL(DOLLAR_pattern)), loc, true);
    bind(pattern.get());

    rewrite.location_set(assign->location_get());
    ast::rExp res = exp(rewrite
                        % bind.result_get().unchecked_cast<ast::Exp>()
                        % assign->value_get()
                        % bind.bindings_get());
    res->location_set(assign->location_get());
    result_ = recurse(res);
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
      errors_.error(binding->location_get(), "declaration not allowed here");
    ast::loc loc = binding->location_get();

    result_ = new ast::Declaration(loc, binding->what_get(), 0);
    result_ = recurse(result_);
  }

  void Desugarer::visit(const ast::Class* c)
  {
    ast::loc l = c->location_get();
    ast::rLValue what = recurse(c->what_get());
    libport::Symbol name = what->call()->name_get();

    PARAMETRIC_AST(desugar,
      "var %lvalue:1 ="
      "{"
      "  var '$tmp' = Object.clone |"
      "  %exp:2 |"
      "  '$tmp'.setSlot(\"type\", %exp:3) |"
      "  '$tmp'.setSlot(%exp:4, function () { this }) |"
      "  do ('$tmp')"
      "  {"
      "    %exp:5 |"
      "  } |"
      "  '$tmp'"
      "}"
      );

    ast::exps_type* protos = maybe_recurse_collection(c->protos_get());
    ast::rExp protos_set;
    if (protos)
    {
      PARAMETRIC_AST(setProtos, "'$tmp'.setProtos(%exp:1)");
      protos_set = exp(setProtos % new ast::List(l, protos));
    }
    else
      protos_set = new ast::Noop(l, 0);

    desugar % what
      % protos_set
      % ast_string(l, name)
      % ast_string(l, libport::Symbol("as" + name.name_get()))
      % c->content_get();

    allow_subdecl_ = true;
    result_ = recurse(exp(desugar));
    result_->original_set(c);
  }

  void Desugarer::visit(const ast::Decrementation* dec)
  {
    PARAMETRIC_AST(decrement, "(%lvalue:1 -= 1) + 1");

    ast::rExp res = recurse(exp(decrement % dec->exp_get()));
    res->original_set(dec);
    result_ = res;
  }

  void Desugarer::visit(const ast::Emit* e)
  {
    ast::rExp event = recurse(e->event_get());
    ast::exps_type* args =
      maybe_recurse_collection(e->arguments_get());

    if (ast::rExp duration = e->duration_get())
    {
      PARAMETRIC_AST(emit,
                     "{"
		     "  var '$emit' = %exp:1.trigger(%exps:2) |"
		     "  var '$duration' = %exp:3 |"
                     "  if ('$duration' != inf)"
                     "    Control.finally(closure () { sleep('$duration') },"
                     "                    closure () { '$emit'.stop })"
		     "}");

      // FIXME: children desugared twice
      result_ =  recurse(exp(emit %  event % args % duration));
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
    PARAMETRIC_AST(increment, "(%lvalue:1 += 1) - 1");

    ast::rExp res = recurse(exp(increment % inc->exp_get()));
    res->original_set(inc);
    result_ = res;
  }

  void Desugarer::visit(const ast::OpAssignment* a)
  {
    PARAMETRIC_AST(desugar, "%lvalue:1 = %exp:2 . %id:3 (%exp:4)");

    ast::rLValue what = recurse(a->what_get());
    ast::rLValue tgt = ast_lvalue_once(what);

    desugar % tgt
      % new_clone(tgt)
      % a->op_get()
      % a->value_get();

    result_ = ast_lvalue_wrap(what, exp(desugar));
    result_ = recurse(result_);
    result_->original_set(a);
  }

  void Desugarer::visit(const ast::Pipe* s)
  {
    allow_subdecl_ = true;
    super_type::visit(s);
  }

  void Desugarer::visit(const ast::Scope* s)
  {
    allow_subdecl_ = true;
    super_type::visit(s);
  }

  void Desugarer::visit(const ast::Stmt* s)
  {
    allow_subdecl_ = true;
    super_type::visit(s);
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

  void Desugarer::visit(const ast::Try* t)
  {
    ast::loc loc = t->location_get();
    ast::rTry res = new ast::Try(loc, recurse(t->body_get()), ast::catches_type());

    foreach (const ast::rCatch& c, t->handlers_get())
    {
      const ast::loc& loc = c->location_get();
      ast::rCatch desugared;

      PARAMETRIC_AST(pattern, "var '$pattern' = Pattern.new(%exp:1)");

      if (c->match_get())
      {
        rewrite::PatternBinder bind(ast_call(loc, SYMBOL(DOLLAR_pattern)), loc);
        bind(c->match_get()->pattern_get().get());
        ast::rExp p = exp(pattern % bind.result_get().unchecked_cast<ast::Exp>());
        {
          allow_subdecl_ = true;
          p = recurse(p);
          allow_subdecl_ = false;
        }
        ast::rMatch match =
          new ast::Match(loc, p, recurse(c->match_get()->guard_get()));
        match->bindings_set(recurse(bind.bindings_get()));
        match->original_set(c->match_get());
        desugared = new ast::Catch(loc, match, recurse(c->body_get()));
      }
      else
      {
        super_type::visit(c.get());
        desugared = c.unchecked_cast<ast::Catch>();
      }
      res->handlers_get().push_back(desugared);
    }
    result_ = res;
  }

  void Desugarer::visit(const ast::While* s)
  {
    allow_subdecl_ = true;
    super_type::visit(s);
  }

}
