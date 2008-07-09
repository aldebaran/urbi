#include <rewrite/rewrite.hh>
#include <rewrite/pattern-rewriter.hh>
#include <rewrite/rescoper.hh>

namespace rewrite
{
  ast::rNary
  rewrite(ast::rConstNary nary)
  {
    PatternRewriter rewrite_patterns;
    Rescoper rescope;
    ast::rAst res;

    rescope(nary);
    res = rescope.result_get();

    rewrite_patterns(res);
    res = rewrite_patterns.result_get();

    return res.unsafe_cast<ast::Nary>();
  }
}
