#include <ast/cloner.hxx>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>

#include <object/symbols.hh>

#include <parser/ast-factory.hh>
#include <parser/parse.hh>

#include <rewrite/desugarer.hh>

namespace rewrite
{
  using ast::ParametricAst;
  using parser::ast_call;
  using parser::ast_exp;
  using parser::ast_string;
  using parser::ast_lvalue_once;
  using parser::ast_lvalue_wrap;

  void Desugarer::visit(const ast::Decrementation* dec)
  {
    static ast::ParametricAst decrement("(%lvalue:1 -= 1) + 1");

    ast::rExp res = recurse(exp(decrement % dec->exp_get()));
    res->original_set(dec);
    result_ = res;
  }

  void Desugarer::visit(const ast::Delete* d)
  {
    static ParametricAst del("%exp:1.removeSlot(%exp:2)");

    ast::rCall call = d->what_get()->call();
    del % call->target_get()
      % ast_string(call->location_get(), call->name_get());
    result_ = exp(del);
    result_->original_set(d);
  }

  void Desugarer::visit(const ast::Incrementation* inc)
  {
    static ast::ParametricAst increment("(%lvalue:1 += 1) - 1");

    ast::rExp res = recurse(exp(increment % inc->exp_get()));
    res->original_set(inc);
    result_ = res;
  }

  void Desugarer::visit(const ast::OpAssignment* a)
  {
    static ParametricAst desugar("%lvalue:1 = %exp:2 . %id:3 (%exp:4)");

    ast::rLValue what = recurse(a->what_get());
    ast::rLValue tgt = ast_lvalue_once(what);

    desugar % tgt
      % ast_exp(new_clone(tgt))
      % a->op_get()
      % recurse(a->value_get());

    result_ = ast_lvalue_wrap(what, exp(desugar));
    result_->original_set(a);
  }

  void Desugarer::visit(const ast::PropertyRead* p)
  {
    static ParametricAst read("%exp:1.getProperty(%exp:2, %exp:3)");

    ast::rCall owner = p->owner_get();
    read % owner->target_get()
      % ast_string(owner->location_get(), owner->name_get())
      % ast_string(p->location_get(), p->name_get());
    result_ = exp(read);
    result_->original_set(p);
  }

  void Desugarer::visit(const ast::PropertyWrite* p)
  {
    static ParametricAst read("%exp:1.setProperty(%exp:2, %exp:3, %exp:4)");

    ast::rCall owner = p->owner_get();
    read % owner->target_get()
      % ast_string(owner->location_get(), owner->name_get())
      % ast_string(p->location_get(), p->name_get())
      % p->value_get();
    result_ = exp(read);
    result_->original_set(p);
  }
}
