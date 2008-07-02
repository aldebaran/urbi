#include <rewrite/rewrite.hh>
#include <rewrite/pattern-rewriter.hh>

namespace rewrite
{
  ast::rNary
  rewrite(ast::rConstNary a)
  {
    PatternRewriter rewrite;
    rewrite(a);
    return rewrite.result_get().unsafe_cast<ast::Nary>();
  }
}
