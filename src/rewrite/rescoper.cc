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
    /// Build 'var <s>'
    static ast::rExp
    make_declaration(const ast::loc& l, libport::Symbol s)
    {
      static ast::ParametricAst a("nil");
      return new ast::Declaration(l, s, exp(a));
    }

    /// Build '<s> = <value>'
    static ast::rExp
    make_assignment(const ast::loc& l, libport::Symbol s, ast::rExp value)
    {
      return new ast::Assignment(l, s, value, 0);
    }

    /**
     *  Helper to extract declarations to a nary.
     *
     *  If \a subject is 'var x = v;', push back 'var x;' in \a nary
     *  and return 'x = v;'
     *  Otherwise, just return \a subject.
     */
    static ast::rExp
    unscope(ast::rExp subject, ast::rNary nary)
    {
      if (ast::rConstDeclaration dec =
          subject.unsafe_cast<const ast::Declaration>())
      {
        ast::loc l = subject->location_get();
        const libport::Symbol name = dec->what_get();
        nary->push_back(make_declaration(l, name),
                        ast::flavor_pipe);
        return make_assignment(l, name, dec->value_get());
      }
      else
        return subject;
    }
  }

  Rescoper::Rescoper()
  {}

  Rescoper::~Rescoper()
  {}

  void Rescoper::visit(const ast::And* a)
  {
    ast::loc l = a->location_get();
    ast::rAnd res = new ast::And(l, ast::exps_type());
    ast::rNary nary = new ast::Nary(l);
    foreach (ast::rExp child, a->children_get())
    {
      child = unscope(child, nary);
      // Wrap every child in a closure
      res->children_get().push_back(recurse(parser::ast_closure(child)));
    }

    nary->push_back(res);
    result_ = nary;
  }

  void
  Rescoper::visit(const ast::Nary* nary)
  {
    ast::rNary res = new ast::Nary(nary->location_get());
    foreach (ast::rExp child, (nary->children_get()))
    {
      ast::rStmt stm = child.unsafe_cast<ast::Stmt>();
      if (stm && stm->flavor_get() == ast::flavor_comma)
      {
        ast::rExp child = stm->expression_get();
        child = unscope(child, res);
        res->push_back(recurse(parser::ast_closure(child)),
                       ast::flavor_comma);
      }
      else
        res->push_back(recurse(child));
    }
    result_ = res;
  }

}
