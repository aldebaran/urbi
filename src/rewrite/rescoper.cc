/**
 ** \file rewrite/rescoper.cc
 ** \brief Implementation of rewrite::Rescoper.
 */

#include <ast/flavor.hh>
#include <ast/parametric-ast.hh>
#include <parser/ast-factory.hh>
#include <rewrite/rescoper.hh>

namespace rewrite
{

  /*----------.
  | Helpers.  |
  `----------*/

  namespace
  {
    static
    ast::rExp
    make_declaration(const ast::loc& l, libport::Symbol s)
    {
      static ast::ParametricAst a("nil");
      return new ast::Declaration(l, s, exp(a));
    }

    static
    ast::rExp
    make_assignment(const ast::loc& l, libport::Symbol s, ast::rExp value)
    {
      return parser::ast_closure(new ast::Assignment(l, s, value, 0));
    }
  }

  Rescoper::Rescoper()
  {}

  Rescoper::~Rescoper()
  {}

  void Rescoper::visit(ast::rConstAnd a)
  {
    ast::rAnd res = new ast::And(a->location_get(), ast::exps_type());
    foreach (ast::rExp child, a->children_get())
      // Wrap every children in a closure
      res->children_get().push_back(recurse(parser::ast_closure(child)));
    result_ = res;
  }

  void
  Rescoper::visit(ast::rConstNary nary)
  {
    ast::rNary res = new ast::Nary(nary->location_get());
    foreach (ast::rExp child, (nary->children_get()))
    {
      ast::rStmt stm = child.unsafe_cast<ast::Stmt>();
      if (stm && stm->flavor_get() == ast::flavor_comma)
      {
        if (ast::rDeclaration dec =
            stm->expression_get().unsafe_cast<ast::Declaration>())
        {
          const ast::loc l = dec->location_get();
          const libport::Symbol s = dec->what_get();
          res->push_back(recurse(make_declaration(l, s)),
                         ast::flavor_semicolon);
          res->push_back(recurse(make_assignment(l, s, dec->value_get())),
                         ast::flavor_comma);
        }
        else
          res->push_back(recurse(parser::ast_closure(child)),
                         ast::flavor_comma);
      }
      else
        res->push_back(recurse(child));
    }
    result_ = res;
  }

}
