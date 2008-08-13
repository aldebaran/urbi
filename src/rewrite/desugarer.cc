#include <ast/cloner.hxx>
#include <ast/new-clone.hh>
#include <ast/parametric-ast.hh>

#include <parser/ast-factory.hh>
#include <parser/parse.hh>
#include <parser/tweast.hh>

#include <rewrite/desugarer.hh>

namespace rewrite
{
  using parser::ast_string;
  using ast::ParametricAst;

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
    parser::Tweast tweast;
    tweast << "{";
    ast::rCall what = parser::ast_lvalue_once(a->what_get()->call(), tweast);
    tweast << new_clone(what) << " = "
           << what << ".'" << a->op_get() << "'(" << a->value_get() << ")";
    tweast << "}";
    ast::rExp res = parser::parse(tweast)->ast_get();
    res->original_set(a);
    result_ = res;
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
