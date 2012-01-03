/*
 * Copyright (C) 2008-2010, 2012, Gostai S.A.S.
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
#include <urbi/object/object.hh>

namespace ast
{

  Analyzer::Analyzer()
    : ast::Cloner()
    , errors_()
    , factory_(new ast::Factory)
  {}

  Analyzer::~Analyzer()
  {}

  void
  Analyzer::throw_if_err()
  {
    if (!errors_.empty())
      throw errors_;
  }


  ast::rExp
  analyze(Analyzer& analyze, ast::rConstAst a)
  {
    if (!a)
      return 0;
    analyze(a.get());
    analyze.throw_if_err();
    return analyze.result_get().unchecked_cast<ast::Exp>();
  }

} // namespace binder
