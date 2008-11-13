#include <ast/cloner.hxx>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>

#include <object/symbols.hh>

#include <parser/ast-factory.hh>
#include <parser/parse.hh>

#include <rewrite/desugarer.hh>

namespace rewrite
{
  using parser::ast_call;
  using parser::ast_string;
  using parser::ast_lvalue_once;
  using parser::ast_lvalue_wrap;

  void
  Desugarer::visit(const ast::Binding* binding)
  {
    PARAMETRIC_AST(rewrite,
                   "Pattern.Binding.new(%exp:1)");
    rewrite % parser::ast_string(binding->location_get(),
                                 binding->name_get());
    result_ = exp(rewrite);
    result_->original_set(binding);
  }

  void Desugarer::visit(const ast::Class* c)
  {
    ast::loc l = c->location_get();
    ast::rLValue what = recurse(c->what_get());
    libport::Symbol name = what->call()->name_get();
    ast::rNary content = recurse(c->content_get());

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
      % content;

    result_ = exp(desugar);
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
                     "var '$emit' = %exp:1.trigger(%exps:2) |"
                     "detach({ sleep(%exp:3) | '$emit'.stop})");

      result_ = exp(emit %  event % args % duration);
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
      % recurse(a->value_get());

    result_ = ast_lvalue_wrap(what, exp(desugar));
    result_->original_set(a);
  }

  void Desugarer::visit(const ast::PropertyRead* p)
  {
    PARAMETRIC_AST(read, "%exp:1.getProperty(%exp:2, %exp:3)");

    ast::rCall owner = p->owner_get();
    read % owner->target_get()
      % ast_string(owner->location_get(), owner->name_get())
      % ast_string(p->location_get(), p->name_get());
    result_ = exp(read);
    result_->original_set(p);
  }

  void Desugarer::visit(const ast::PropertyWrite* p)
  {
    PARAMETRIC_AST(read, "%exp:1.setProperty(%exp:2, %exp:3, %exp:4)");

    ast::rCall owner = p->owner_get();
    read % owner->target_get()
      % ast_string(owner->location_get(), owner->name_get())
      % ast_string(p->location_get(), p->name_get())
      % p->value_get();
    result_ = exp(read);
    result_->original_set(p);
  }
}
