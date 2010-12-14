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

  ast::rAst
  rewrite(ast::rConstAst a)
  {
    Desugarer desugar;
    ast::rAst res = ast::analyze(desugar, a);
    Rescoper rescope;
    rescope(res.get());
    return rescope.result_get();
  }

#define REWRITE(In, Out)                                                \
  libport::intrusive_ptr<Out>                                           \
  rewrite(libport::intrusive_ptr<In> a)                                 \
  {                                                                     \
    return rewrite(a.unsafe_cast<const ast::Ast>()).unsafe_cast<Out>(); \
  }

  REWRITE(ast::Ast,  ast::Ast);
  REWRITE(ast::Exp,  ast::Exp);
  REWRITE(ast::Nary, ast::Nary);

  REWRITE(const ast::Exp,  ast::Exp);
  REWRITE(const ast::Nary, ast::Nary);
#undef REWRITE
}
