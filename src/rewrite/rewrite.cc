/*
 * Copyright (C) 2008-2012, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

#include <ast/print.hh>
#include <rewrite/desugarer.hh>
#include <rewrite/pattern-binder.hh>
#include <rewrite/rewrite.hh>
#include <rewrite/rescoper.hh>

namespace rewrite
{
  ast::rAst
  desugar(ast::rConstAst ast)
  {
    Desugarer desugar;
    return ast::analyze(desugar, ast);
  }

  ast::rAst
  rescope(ast::rConstAst ast)
  {
    // GD_CATEGORY(Urbi.Ast.Rescope);
    // GD_FINFO_TRACE("Rescope in: %s", *ast);
    Rescoper rescope;
    ast::rAst res = ast::analyze(rescope, ast);
    // GD_FINFO_TRACE("Rescope out: %s", *res);
    return res;
  }

  ast::rAst
  rewrite(ast::rConstAst a)
  {
    return rescope(desugar(a));
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
