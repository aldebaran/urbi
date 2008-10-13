#include <rewrite/desugarer.hh>
#include <rewrite/pattern-binder.hh>
#include <rewrite/rewrite.hh>
#include <rewrite/rescoper.hh>

namespace rewrite
{
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

  ast::rPipe pattern_bind(const ast::rConstAst& pattern,
                          const ast::rExp& exp,
                          bool before)
  {
    ast::rPipe res = new ast::Pipe(pattern->location_get(), ast::exps_type());
    rewrite::PatternBinder bind(res);
    bind(pattern.get());
    if (before)
      res->children_get().push_back(exp);
    else
      res->children_get().push_front(exp);
    return res;
  }

}
