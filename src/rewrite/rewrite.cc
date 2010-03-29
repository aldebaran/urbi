/*
 * Copyright (C) 2008-2010, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <rewrite/desugarer.hh>
#include <rewrite/pattern-binder.hh>
#include <rewrite/rewrite.hh>
#include <rewrite/rescoper.hh>

namespace rewrite
{
  ast::rAst
  desugar(ast::rConstAst Ast)
  {
    Desugarer desugar;
    return ast::analyze(desugar, Ast);
  }

  ast::rAst
  rescope(ast::rConstAst Ast)
  {
    Rescoper rescope;
    return ast::analyze(rescope, Ast);
  }

  ast::rExp
  rewrite(ast::rConstExp nary)
  {
    Desugarer desugar;
    Rescoper rescope;
    ast::rAst res = ast::analyze(desugar, nary);
    rescope(res.get());
    res = rescope.result_get();
    return res.unsafe_cast<ast::Exp>();
  }

  ast::rNary
  rewrite(ast::rConstNary nary)
  {
    return rewrite(ast::rConstExp(nary)).unsafe_cast<ast::Nary>();
  }
}
