#include <rewrite/desugarer.hh>
#include <rewrite/pattern-binder.hh>
#include <rewrite/rewrite.hh>
#include <rewrite/rescoper.hh>

namespace rewrite
{
  // FIXME: template-factor those two
  ast::rNary
  rewrite(ast::rConstNary nary)
  {
    Desugarer desugar;
    Rescoper rescope;
    ast::rAst res;

    desugar(nary.get());
    res = desugar.result_get();

    rescope(res.get());
    res = rescope.result_get();

    return res.unsafe_cast<ast::Nary>();
  }

  ast::rExp
  rewrite(ast::rConstExp nary)
  {
    Desugarer desugar;
    Rescoper rescope;
    ast::rAst res;

    desugar(nary.get());
    res = desugar.result_get();

    rescope(res.get());
    res = rescope.result_get();

    return res.unsafe_cast<ast::Exp>();
  }
}
