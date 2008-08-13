#include <rewrite/desugarer.hh>
#include <rewrite/rewrite.hh>
#include <rewrite/pattern-rewriter.hh>
#include <rewrite/rescoper.hh>

namespace rewrite
{
  ast::rNary
  rewrite(ast::rConstNary nary)
  {
    Desugarer desugar;
    PatternRewriter rewrite_patterns;
    Rescoper rescope;
    ast::rAst res;

    desugar(nary.get());
    res = desugar.result_get();

    rescope(res.get());
    res = rescope.result_get();

    rewrite_patterns(res.get());
    res = rewrite_patterns.result_get();

    return res.unsafe_cast<ast::Nary>();
  }
}
