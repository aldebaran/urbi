#include <parser/ast-factory.hh>
#include <parser/tweast.hh>
#include <rewrite/desugarer.hh>

namespace rewrite
{
  void Desugarer::visit(const ast::Incrementation* inc)
  {
    // FIXME: We can't use parametric ast here because of the grammar
    // illness
    // static ast::ParametricAst increment("%exp:1 += 1) - 1");
    parser::Tweast tweast;

    operator()(inc->exp_get().get());
    // FIXME: Use a static cast
    tweast << "(" << result_.cast<ast::Call>() << " += 1) - 1";
    ast::rExp res = parser::desugar(tweast);
    res->original_set(inc);
    result_ = res;
  }

  void Desugarer::visit(const ast::Decrementation* dec)
  {
    parser::Tweast tweast;

    operator()(dec->exp_get().get());
    tweast << "(" << result_.cast<ast::Call>() << " -= 1) + 1";
    ast::rExp res = parser::desugar(tweast);
    res->original_set(dec);
    result_ = res;
  }
}
