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


  ast::rExp
  analyze(Analyzer& analyze, ast::rConstExp a)
  {
    if (!a)
      return 0;

    analyze(a.get());

    if (analyze.errors_get().empty())
      return analyze.result_get().unsafe_cast<ast::Exp>();
    else
    {
      ast::rNary res = new ast::Nary();
      analyze.errors_get().process_errors(*res);
      return res;
    }
  }

} // namespace binder
