//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \file ast/var.cc
 ** \brief Implementation of ast::Var.
 */

#include "ast/visitor.hh"
#include "ast/var.hh"

namespace ast
{

  Var::Var (const loc& location):
    Exp (location)
  { }

  Var::~Var ()
  {
  }

  void
  Var::accept (ConstVisitor& v) const
  {
    v (*this);
  }
  void
  Var::accept (Visitor& v)
  {
    v (*this);
  }
    

} // namespace ast

