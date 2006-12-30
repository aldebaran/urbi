//<<-
// Generated, do not edit by hand.
//->>
/**
 ** \file ast/assign-exp.cc
 ** \brief Implementation of ast::AssignExp.
 */

#include "ast/visitor.hh"
#include "ast/assign-exp.hh"

namespace ast
{

  AssignExp::AssignExp (const loc& location, Var* var, Exp* exp):
    Exp (location),
    var_ (var),
    exp_ (exp)
  { }

  AssignExp::~AssignExp ()
  {
    delete var_;
    delete exp_;
  }

  void
  AssignExp::accept (ConstVisitor& v) const
  {
    v (*this);
  }
  void
  AssignExp::accept (Visitor& v)
  {
    v (*this);
  }
    

} // namespace ast

