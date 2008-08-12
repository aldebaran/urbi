#include <ast/cloner.hxx>
#include <ast/new-clone.hh>

#include <parser/ast-factory.hh>
#include <parser/parse.hh>
#include <parser/tweast.hh>

#include <rewrite/desugarer.hh>

namespace rewrite
{
  void Desugarer::visit(const ast::Decrementation* dec)
  {
    parser::Tweast tweast;

    tweast << "(" << dec->exp_get() << " -= 1) + 1";
    ast::rExp res = recurse(parser::parse(tweast)->ast_get());
    res->original_set(dec);
    result_ = res;
  }

  void Desugarer::visit(const ast::Incrementation* inc)
  {
    // FIXME: We can't use parametric ast here because of the grammar
    // illness
    // static ast::ParametricAst increment("%exp:1 += 1) - 1");
    parser::Tweast tweast;

    tweast << "(" << inc->exp_get() << " += 1) - 1";
    ast::rExp res = recurse(parser::parse(tweast)->ast_get());
    res->original_set(inc);
    result_ = res;
  }

  void Desugarer::visit(const ast::OpAssignment* a)
  {
    parser::Tweast tweast;
    tweast << "{";
    ast::rCall what = parser::ast_lvalue_once(a->what_get(), tweast);
    tweast << new_clone(what) << " = "
           << what << ".'" << a->op_get() << "'(" << a->value_get() << ")";
    tweast << "}";
    ast::rExp res = parser::parse(tweast)->ast_get();
    res->original_set(a);
    result_ = res;
  }
}
