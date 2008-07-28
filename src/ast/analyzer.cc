#include <ast/analyzer.hh>
#include <ast/nary.hh>

namespace ast
{

  inline
  ast::Error&
  Analyzer::errors_get()
  {
    return errors_;
  }


  ast::rNary
  analyze(Analyzer& analyze, ast::rConstNary a)
  {
    analyze(a.get());

    ast::rNary res;
    if (analyze.errors_get().empty())
      res = analyze.result_get().unsafe_cast<ast::Nary>();
    else
    {
      res = new ast::Nary();
      analyze.errors_get().process_errors(*res);
    }
    return res;
  }

} // namespace binder
