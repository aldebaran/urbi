/*
 * Copyright (C) 2008-2011, Gostai S.A.S.
 *
 * This software is provided "as is" without warranty of any kind,
 * either expressed or implied, including but not limited to the
 * implied warranties of fitness for a particular purpose.
 *
 * See the LICENSE file for more information.
 */

/**
 ** \file rewrite/rescoper.cc
 ** \brief Implementation of rewrite::Rescoper.
 */

#include <ast/flavor.hh>
#include <ast/parametric-ast.hh>
#include <ast/factory.hh>
#include <rewrite/rescoper.hh>
#include <ast/cloner.hxx>

DECLARE_LOCATION_FILE;

namespace rewrite
{

  /*----------.
  | Helpers.  |
  `----------*/

  /// Build 'var <s>'
  ast::rExp
  Rescoper::make_declaration(const ast::loc& l, ast::rConstLValue what)
  {
    return new ast::Declaration(l, recurse(what), 0);
  }

  /// Build '<s> = <value>'
  ast::rExp
  Rescoper::make_assignment(const ast::loc& l, ast::rConstLValue what,
                            ast::rConstExp value)
  {
    if (!value)
    {
      PARAMETRIC_AST(ast, "{}");
      return exp(ast);
    }
    return new ast::Assignment(l, recurse(what), recurse(value));
  }

  /**
   *  Helper to extract declarations to a nary.
   *
   *  If \a subject is 'var x = v;', push back 'var x;' in \a nary
   *  and return 'x = v;'
   *  Otherwise, just return \a subject.
   */
  ast::rExp
  Rescoper::unscope(ast::rExp subject, ast::rNary nary)
  {
    if (ast::rConstDeclaration dec =
        subject.unsafe_cast<const ast::Declaration>())
    {
      ast::loc l = dec->location_get();
      ast::rConstLValue call = dec->what_get();
      nary->push_back(make_declaration(l, call),
                      ast::flavor_pipe);
      return make_assignment(l, call, dec->value_get());
    }
    else if (ast::rConstPipe pipe =
             subject.unsafe_cast<const ast::Pipe>())
    {
      ast::rPipe res = new ast::Pipe(pipe->location_get(), ast::exps_type());
      foreach (const ast::rExp& child, pipe->children_get())
        res->children_get().push_back(unscope(child, nary));
      return res;
    }
    else
      return subject;
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
      res->children_get().push_back(recurse(factory_->make_closure(child)));
    }

    nary->push_back(res);
    result_ = nary;
  }

  void
  Rescoper::visit(const ast::Nary* nary)
  {
    ast::rNary res = new ast::Nary(nary->location_get());
    foreach (ast::rExp child, nary->children_get())
    {
      ast::rStmt stm = child.unsafe_cast<ast::Stmt>();
      if (stm && stm->flavor_get() == ast::flavor_comma)
      {
        ast::rExp child = stm->expression_get();
        child = unscope(child, res);
        res->push_back(recurse(factory_->make_closure(child)),
                       ast::flavor_comma);
      }
      else
        res->push_back(recurse(child));
    }
    result_ = res;
  }

  void Rescoper::visit(const ast::While* a)
  {
    if (a->flavor_get() == ast::flavor_comma)
      {
        ast::loc l = a->location_get();
        ast::rExp body = recurse(factory_->make_closure(a->body_get()));
        result_ = factory_->make_while(l,
                                       l, a->flavor_get(),
                                       recurse(a->test_get()),
                                       body);
      }
    else
      super_type::visit(a);
  }

}
