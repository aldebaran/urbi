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

  ast::modifiers_type*
  Desugarer::handle(const ast::modifiers_type* originals)
  {
    ast::modifiers_type* res = 0;
    if (originals)
    {
      res = new ast::modifiers_type();
      foreach (ast::modifiers_type::value_type original, *originals)
        (*res)[original.first] =  recurse(original.second);
    }
    return res;
  }

  void
  Desugarer::visit(const ast::Assign* assign)
  {
//     std::cerr << "ASSIGN: " << *assign << std::endl;
    ast::loc loc = assign->location_get();

    // Simple declaration: var x = value
    if (ast::rBinding what = assign->what_get().unsafe_cast<ast::Binding>())
    {
      if (!allow_decl_)
        errors_.error(what->location_get(), "declaration not allowed here");
      ast::modifiers_type* modifiers = handle(assign->modifiers_get());
      result_ = new ast::Declaration(loc,
                                     what->what_get(),
                                     assign->value_get(),
                                     modifiers);
      result_ = recurse(result_);
      return;
    }

    // Simple assignment: x = value
    if (ast::rCall call = assign->what_get().unsafe_cast<ast::Call>())
    {
      // Build dictionary for the (potential) modifiers
      ast::rExp modifiers = 0;
      if (const ast::modifiers_type* source = assign->modifiers_get())
      {
        PARAMETRIC_AST(dict, "Dictionary.new");

        modifiers = exp(dict);
        foreach (const ast::modifiers_type::value_type& elt, *source)
        {
          PARAMETRIC_AST(add, "%exp:1.set(%exp:2, %exp:3)");

          add % modifiers
            % new ast::String(loc, elt.first)
            % recurse(elt.second);
          modifiers = exp(add);
        }
      }

      if (modifiers)
      {
        ast::rExp target_value = recurse(assign->value_get());
        ast::rLValue tgt = ast_lvalue_once(call);
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
                                              parser::ast_call(loc, SYMBOL(v)), 0);

        trajectory
          % read
          % write
          % target_value
          % modifiers;

        result_ = ast_lvalue_wrap(call, exp(trajectory)).get();
      }
      else
        result_ = new ast::Assignment(loc, call, assign->value_get(), 0);

      result_ = recurse(result_);
      return;
    }

    // Subscript assignment: x[y] = value
    if (ast::rSubscript sub =
        assign->what_get().unsafe_cast<ast::Subscript>())
    {
      ast::Assignment* res = new ast::Assignment(loc,
	ast::rLValue(reinterpret_cast<ast::LValue*>(sub->target_get().get())),
	assign->value_get(), handle(assign->modifiers_get()));
      res->method_set(new libport::Symbol(SYMBOL(SBL_SBR_EQ)));
      res->extra_args_set(maybe_recurse_collection(sub->arguments_get()));
      result_ = recurse(ast::rExp(res));
      return;
    }

 // No modifiers allowed below this point.
    if (assign->modifiers_get())
      errors_.error(assign->location_get(), "Modifiers not allowed here");

    // Property assignment: x->prop = value
    if (ast::rProperty prop =
        assign->what_get().unsafe_cast<ast::Property>())
    {
      PARAMETRIC_AST(rewrite, "%exp:1 . setProperty(%exp:2, %exp:3, %exp:4)");

      rewrite
        % recurse(prop->owner_get()->target_get())
        % ast_string(loc, prop->owner_get()->name_get())
        % ast_string(loc, prop->name_get())
        % recurse(assign->value_get());

      result_ = recurse(exp(rewrite));
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
      (ast_call(loc, SYMBOL(DOLLAR_pattern)), loc);
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

    result_ = new ast::Declaration(loc, binding->what_get(), 0, 0);
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
		     " var '$emit' = %exp:1.trigger(%exps:2) |"
		     " var '$duration' = %exp:3 |"
                     " if ('$duration' != inf) "
		     "  detach({ sleep('$duration') | '$emit'.stop})"
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

  void Desugarer::visit(const ast::Property* p)
  {
    PARAMETRIC_AST(read, "%exp:1.getProperty(%exp:2, %exp:3)");

    ast::rCall owner = p->owner_get();
    read % owner->target_get()
      % ast_string(owner->location_get(), owner->name_get())
      % ast_string(p->location_get(), p->name_get());
    result_ = exp(read);
    result_->original_set(p);
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
