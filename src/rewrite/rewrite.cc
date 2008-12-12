#include <rewrite/desugarer.hh>
#include <rewrite/pattern-binder.hh>
#include <rewrite/rewrite.hh>
#include <rewrite/rescoper.hh>

namespace rewrite
{
  ast::rNary
  rewrite(ast::rConstNary nary)
  {
    return rewrite(ast::rConstExp(nary)).unsafe_cast<ast::Nary>();
  }

  ast::rExp
  rewrite(ast::rConstExp nary)
  {
    Desugarer desugar;
    Rescoper rescope;
    ast::rAst res;

    res = ast::analyze(desugar, nary);

    rescope(res.get());
    res = rescope.result_get();

    return res.unsafe_cast<ast::Exp>();
  }
}
