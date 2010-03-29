/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <ast/analyzer.hh>
#include <ast/nary.hh>
#include <ast/factory.hh>

namespace ast
{

  Analyzer::Analyzer()
    : ast::Cloner()
    , errors_()
    , factory_(new ast::Factory)
  {}

  Analyzer::~Analyzer()
  {}

  ast::Error&
  Analyzer::errors_get()
  {
    return errors_;
  }


  ast::rExp
  analyze(Analyzer& analyze, ast::rConstAst a)
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
